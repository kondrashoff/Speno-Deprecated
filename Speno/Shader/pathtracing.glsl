void scatterLambertian(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    vec4 data = randomCosineDirectionWithPDF();
    float scattering_pdf = data.w;

    mat3 onb = onbBuildFromW(hit.normal);
    ray.direction = onb * data.xyz;

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = scattering_pdf;
}

void scatterMirror(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
	ray.origin += ray.direction * (hit.t - EPSILON);

	ray.direction = normalize(reflect(ray.direction, hit.normal));
	color = (color * hit.color) / pdf;
	pdf = 1.0;
}

bool scatterHdriLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    vec2 size = textureSize(hdri_data_texture, 0);
    float r;
    float coord1d;
    vec2 coord;
    vec4 data;
    vec3 hdri_direction;
    float luminance;

    int max_attempts = 4;
    do {
        r = randomFloat();
        coord1d = r * size.x * size.y;
        coord = vec2(mod(coord1d, size.x), floor(coord1d / size.x)) / size;

        data = texture(hdri_data_texture, coord);
        hdri_direction = 2.0 * data.xyz - 1.0;
        luminance = data.w;
    } while(dot(hdri_direction, hit.normal) < EPSILON && max_attempts-- > 0);

    ray.origin += ray.direction * (hit.t - EPSILON);
    ray.direction = hdri_direction;

    float scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = hdri_luminance_sum / data.w;

    return true;
}

vec3 getRayColor(in Ray ray) {
    Hit hit;
    vec3 color = vec3(1.0);
    vec3 possible_color = vec3(0);
    uint possible_colors = 0u;
    float pdf = 1.0;

    for(uint i = 0u; i < camera.max_depth; i++) {
        if(intersectScene(hit, ray)) {
            if(i == 0) {
                gbuffer_intersection.rgb = ray.origin + ray.direction * hit.t;
                gbuffer_intersection.a = hit.t;
                gbufferNormal.rgb = hit.normal;
                gbufferAlbedo.rgb = hit.color;
                hit.color = vec3(1.0);

                if(!use_pathtracing) {
                    vec3 light1 = vec3(0.3, 0.5, 0.8);
                    vec3 light2 = vec3(0.83, 0.62, 0.56);
                    vec3 light3 = vec3(0.83);
                    vec3 ptao = vec3(1.0);
                   
                    if(use_shadows) {
                        Hit shadow_hit;
                        Ray light_ray = Ray(ray.origin + ray.direction * (hit.t - EPSILON), sun_direction);
                        light3 = intersectScene(shadow_hit, light_ray) ? use_pathtracing_ambient_occlusion ? vec3(0.66) : vec3(0.33) : vec3(3.18, 1.42, 0.36);
                    }
                    if(use_pathtracing_ambient_occlusion) {
                        ptao = vec3(5.0);
                        Hit ptao_hit = hit;
                        float ptao_pdf = 1.0;
                        Ray ptao_ray = ray;
                        scatterLambertian(ptao_hit, ptao_ray, ptao, ptao_pdf);
                        if(intersectScene(ptao_hit, ptao_ray)) ptao = vec3(0.0);

                        light1 *= (0.6 + 0.4 * dot(sun_direction, hit.normal));
                        light2 *= max(0.2, pow(max(0.0, dot(reflect(ray.direction, hit.normal), sun_direction)), 2.0));
                    }
                    else {
                        light1 *= (0.501 + 0.499 * dot(sun_direction, hit.normal));
                        light2 *= max(0.01, pow(max(0.0, dot(reflect(ray.direction, hit.normal), sun_direction)), 2.0));
                    }

                    gbufferAlbedo.rgb *= light3 * (light1 + light2);
                    return ptao;
                }
            }

            if(use_hdri) {
                Hit light_hit = hit;
                Ray light_ray = ray;
                vec3 light_color = color;
                float light_pdf = pdf;

                if(scatterHdriLight(light_hit, light_ray, light_color, light_pdf) && !intersectScene(light_hit, light_ray)) {
                    vec3 c = (light_color * getBackgroundColor(light_ray.direction)) / light_pdf;
                    possible_color += c;
                    possible_colors++;
                }
            }

            if(hit.material.type == MATERIAL_LAMBERTIAN) scatterLambertian(hit, ray, color, pdf);
            else if(hit.material.type == MATERIAL_MIRROR) scatterMirror(hit, ray, color, pdf);
        }
        else if(i == 0) {
            gbuffer_intersection.a = -1.0;
            gbufferAlbedo = vec4(1);
            return getFinalColor(getBackgroundColor(ray.direction));
        }
        else {
            return getFinalColor(color, pdf, getBackgroundColor(ray.direction), possible_color, possible_colors);
        }
    }

    return getFinalColor(possible_color, possible_colors);
}