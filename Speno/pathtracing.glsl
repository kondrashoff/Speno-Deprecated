void scatterLambertian(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    mat3 onb = onbBuildFromW(hit.normal);
    ray.direction = onb * randomCosineDirection();
    float scattering_pdf = scatteringPdf(ray.direction, hit.normal);

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
    float light_cosine = abs(dot(light_hit.normal, -to_light));
    float light_area = sphereArea(light_sphere);

    light_pdf = max(0.0, distance_squared / (light_cosine * light_area));
   
    ray.direction = to_light;
    scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}

bool scatterSkyLight(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    Sphere sun = Sphere(sky.sun_direction * 250000.0, 4000.0);
    ray.origin += ray.direction * (hit.t - EPSILON);

    float scattering_pdf = 0.0;
    float light_pdf = 0.0;

    vec3 on_light;
    on_light = sampleSphere(sun);
    
    vec3 to_light = on_light - ray.origin;
    to_light = normalize(to_light);

    if(dot(to_light, hit.normal) < EPSILON) return false;

    Hit light_hit;
    light_hit.t = MAXIMUM_DISTANCE;
    Ray light_ray = Ray(ray.origin, to_light);

    if(!intersectSphere(light_hit, light_ray, sun)) return false;
    
    float distance_squared = light_hit.t * light_hit.t;
    float light_cosine = abs(dot(light_hit.normal, -to_light));
    float light_area = sphereArea(sun) * 6500.0;

    light_pdf = distance_squared / (light_cosine * light_area);
    if(light_pdf < EPSILON) return false;
   
    ray.direction = to_light;
    scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = light_pdf;

    return true;
}

vec3 pathtrace(in Ray ray) {
    Hit hit;
	
    vec3 color = vec3(1.0);
    float pdf = 1.0;
    vec3 possible_color = vec3(0);
    uint possible_colors = 0u;
    
    for(uint i = 0u; i < camera.max_depth; i++) {
        if(intersectScene3DDAtest3(hit, ray)) {
            if(i == 0) {
                gbuffer_albedo.rgb = hit.color;
                gbuffer_normal.rgb = hit.normal;
                gbuffer_position.rgb = ray.origin + hit.t * ray.direction;
                gbuffer_position.a = hit.t;
            }

            if(is_light) {
                return getFinalColor(color, pdf, hit.color, possible_color, possible_colors);
            }

            Hit light_hit = hit;
            Ray light_ray = ray;
            vec3 light_color = color;
            float light_pdf = pdf;

            if(sky.type == SKY_TYPE_REALISTIC && scatterSkyLight(light_hit, light_ray, light_color, light_pdf) && !intersectScene3DDAtest3(light_hit, light_ray)) {
                vec3 c = (light_color * getSkyColor(light_ray)) / light_pdf;
                 // TODO: ����� ��������� ��� �������� ����� � scatteSkyLight(), ��������� light_contribution, 
                 // ��� ����� ������� ��������, 
                 // ��� ����� �� ������ �������� ����������, � ������� ����� ����� ��������� �������� getSkyColor()
                if((c.r + c.g + c.b) * 0.33333333 > (color.r + color.g + color.b) * 0.16666667) {
                    possible_color += c;
                    possible_colors++;

                    if(is_volume) return getFinalColor(possible_color, possible_colors);
                }
            }
            
            /*Hit camera_hit = hit;
            vec3 pos = ray.origin + ray.direction * (hit.t - EPSILON);
            vec3 to_camera = camera.lookfrom - pos;
            float distance_squared = dot(to_camera, to_camera);
            to_camera = normalize(to_camera);
            vec3 camera_flashlight_color = vec3(10000, 8463, 7098);

            if(dot(to_camera, hit.normal) > EPSILON) {
                intersectScene3DDAtest2(camera_hit, Ray(pos, to_camera));

                if(camera_hit.t > sqrt(distance_squared)) {
                    float scattering_pdf = 0.0;
                    float light_pdf = 0.0;
    
                    float light_cosine = dot(-camera.lookdir, to_camera);
                    light_pdf = distance_squared / pow(light_cosine, 64.0);

                    if(light_pdf > EPSILON) {
                        scattering_pdf = scatteringPdf(to_camera, hit.normal);
                        vec3 color2 = (color * scattering_pdf * hit.color) / pdf;
                        float pdf2 = light_pdf;
                    
                        possible_color += (color2 * camera_flashlight_color) / pdf2;
                        possible_colors++;

                        if(is_volume) return getFinalColor(possible_color, possible_colors);
                    }
                }
            }*/

            scatterLambertian(hit, ray, color, pdf);
        }
        else if(i == 0) {
            gbuffer_albedo.rgb = vec3(0);
            gbuffer_normal.rgb = vec3(0);
            gbuffer_position.rgb = vec3(MAXIMUM_DISTANCE);
            gbuffer_position.a = -1.0;
            
            return getFinalColor(getSkyColor(ray));
        }
        else {
            return getFinalColor(color, pdf, getSkyColor(ray), possible_color, possible_colors);
        }
    }

    return getFinalColor(color, possible_color, possible_colors);
}