void scatterLambertian(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    mat3 onb = onbBuildFromW(hit.normal);
    ray.direction = onb * randomCosineDirection();
    float scattering_pdf = scatteringPdf(ray.direction, hit.normal);

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
    float light_area = sphereArea(sun) * 10000.0;

    light_pdf = max(0.0, distance_squared / (light_cosine * light_area));
   
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
        if(intersectScene3DDA(hit, ray)) {
            if(i == 0) {
                gbuffer_albedo.rgb = hit.color;
                gbuffer_normal.rgb = hit.normal;
                gbuffer_position.rgb = ray.origin + hit.t * ray.direction;
                gbuffer_position.a = hit.t;
            }

            Hit light_hit = hit;
            Ray light_ray = ray;
            vec3 light_color = color;
            float light_pdf = pdf;

            if(sky.type == SKY_TYPE_REALISTIC && scatterSkyLight(light_hit, light_ray, light_color, light_pdf) && !intersectScene3DDA(light_hit, light_ray)) {
                vec3 c = (light_color * getSkyColor(light_ray)) / light_pdf;
                if((c.r + c.g + c.b) * 0.33333333 > (color.r + color.g + color.b) * 0.16666667) {
                    possible_color += c;
                    possible_colors++;
                }
            }

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