void scatterLambertian(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    vec4 data = randomCosineDirectionWithPDF();

    mat3 onb = onbBuildFromW(hit.normal);
    ray.direction = onb * data.xyz;
    float scattering_pdf = data.w == -1.0 ? scatteringPdf(ray.direction, hit.normal) : data.w;

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = scattering_pdf;
}

void scatterVolume(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    ray.direction = randomOnSphere();
    float scattering_pdf = 1.0 / (4.0 * PI);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = scattering_pdf;
}

bool scatterSphereLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf, in Sphere light_sphere) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    float scattering_pdf = 0.0;
    float light_pdf = 0.0;

    vec3 on_light;
    on_light = sampleSphere(light_sphere);
    
    vec3 to_light = on_light - ray.origin;
    to_light = normalize(to_light);

    if(dot(to_light, hit.normal) < EPSILON) return false;

    Hit light_hit;
    light_hit.t = MAXIMUM_DISTANCE;
    Ray light_ray = Ray(ray.origin, to_light);

    if(!intersectSphere(light_hit, light_ray, light_sphere)) return false;
    
    float distance_squared = light_hit.t * light_hit.t;
    float light_cosine = 1.0;//abs(dot(light_hit.normal, -to_light));
    float light_area = sphereArea(light_sphere);

    light_pdf = max(0.0, distance_squared / (light_cosine * light_area));
   
    ray.direction = to_light;
    scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}

bool scatterVoxelLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf, in vec3 voxel_position) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    float scattering_pdf = 0.0;
    float light_pdf = 0.0;

    vec3 on_light;
    vec3 offset = randomVec3();
    on_light = (voxel_position + vec3(0.5)) + offset;
    
    vec3 to_light = on_light - ray.origin;
    to_light = normalize(to_light);

    if(dot(to_light, hit.normal) < EPSILON) return false;

    Hit light_hit;
    light_hit.t = MAXIMUM_DISTANCE;
    Ray light_ray = Ray(ray.origin, to_light);

    if(!intersectAABB(light_hit, light_ray, AABB(voxel_position, voxel_position + 1.0))) return false;
    
    float distance_squared = light_hit.t * light_hit.t;
    float light_cosine = abs(dot(light_hit.normal, -to_light));
    float light_area = 1.0;

    light_pdf = max(0.0, distance_squared / (light_cosine * light_area));
   
    ray.direction = to_light;
    scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}

bool scatterSkyLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    Sphere sun = Sphere(sky.sun_direction, 0.0175);
    ray.origin += ray.direction * (hit.t - EPSILON);
    vec3 on_light = sampleSphere(sun);
    vec3 to_light = normalize(on_light);

    if(dot(to_light, hit.normal) < EPSILON) return false;

    Hit light_hit;
    light_hit.t = MAXIMUM_DISTANCE;
    Ray light_ray = Ray(vec3(0), to_light);

    if(!intersectSphere(light_hit, light_ray, sun)) return false;
    
    ray.direction = to_light;

    float distance_squared = light_hit.t * light_hit.t;
    float light_cosine = 1.0;//abs(dot(light_hit.normal, -to_light));
    float light_area = sphereArea(sun);
    // Для получения света от солнца в n раз светлее тени использовать формулу 0.0075 * 0.5^n
    float light_pdf = 0.000234375 * distance_squared / (light_cosine * light_area); // Свет от солнца в 5 раз раз светлее тени
    //float light_pdf = 0.0000530697 * distance_squared / (light_cosine * light_area); // Свет от солнца в ~7.14 раз раз светлее тени
    float scattering_pdf = scatteringPdf(ray.direction, hit.normal); 

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}

// Option without soft shadows
/*bool scatterSkyLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    vec3 to_light = sky.sun_direction;

    if(dot(to_light, hit.normal) < EPSILON) return false;

    ray.origin += ray.direction * (hit.t - EPSILON);
    ray.direction = to_light;

    float light_pdf = 11.0 * PI;
    float scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}*/

bool scatterHdriLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    vec4 data = getUnitvec3hdriSTBN();
    vec3 to_light = 2.0 * data.xyz - 1.0;

    if(dot(to_light, hit.normal) < EPSILON) return false;

    float light_pdf = data.w;
    float scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    ray.origin += ray.direction * (hit.t - EPSILON);
    ray.direction = to_light;

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}

vec3 pathtrace(in Ray ray) {
    Hit hit;
	
    bool use_voxels = triangles.length() == 0;
    uint depth = camera.max_depth;

    vec3 color = vec3(1.0);
    float pdf = 1.0;
    vec3 possible_color = vec3(0);
    uint possible_colors = 0u;

    for(uint i = 0u; i < depth; i++) {
        if(intersectScene(hit, ray, use_voxels)) {
            if(i == 0) {
                gbuffer_albedo.rgb = hit.color;
                gbuffer_normal.rgb = hit.normal;
                gbuffer_position.rgb = ray.origin + hit.t * ray.direction;
                gbuffer_position.a = hit.t;
                hit.color = vec3(1.0);
            }

            if(is_light) {
                if(i == 0) gbuffer_albedo.rgb = vec3(1);
                return getFinalColor(color, pdf, hit.color, possible_color, possible_colors);
            }

            Hit light_hit = hit;
            Ray light_ray = ray;
            vec3 light_color = color;
            float light_pdf = pdf;
            
            if(sky.type == SKY_TYPE_REALISTIC && sky.sun_direction.y >= -EPSILON && scatterSkyLight(light_hit, light_ray, light_color, light_pdf) && !intersectScene(light_hit, light_ray, use_voxels)) {
                vec3 c = (light_color * getSkyColor(light_ray)) / light_pdf;
                possible_color += c;
                possible_colors++;

                if(is_volume) return getFinalColor(possible_color, possible_colors);
            }

            if(i + 1 < depth) {
                if(!is_volume) scatterLambertian(hit, ray, color, pdf);
                else scatterVolume(hit, ray, color, pdf);
            }
        }
        else if(i == 0) {
            gbuffer_albedo.rgb = vec3(1);
            gbuffer_normal.rgb = vec3(0);
            gbuffer_position.rgb = vec3(MAXIMUM_DISTANCE);
            gbuffer_position.a = -1.0;
            
            return getFinalColor(getSkyColor(ray));
        }
        else {
            return getFinalColor(color, pdf, getSkyColor(ray), possible_color, possible_colors);
        }
    }

    return getFinalColor(possible_color, possible_colors);
}