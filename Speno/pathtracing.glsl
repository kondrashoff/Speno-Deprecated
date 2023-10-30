float scatteringPdf(in vec3 scattered, in vec3 normal) {
    return max(0.0, dot(normal, scattered)) / PI;
}

void scatterLambertian(in Hit hit, inout Ray ray, inout vec3 color, inout float pdf) {
    ray.origin += ray.direction * (hit.t - EPSILON);

    mat3 onb = onbBuildFromW(hit.normal);
    ray.direction = onb * randomCosineDirection();
    float scattering_pdf = scatteringPdf(ray.direction, hit.normal);

    color = (color * scattering_pdf * hit.color) / pdf;
    pdf = scattering_pdf;
}

vec3 pathtrace(in Ray ray) {
    Hit hit;
	
    vec3 color = vec3(1.0);
    float pdf = 1.0;
    
    for(uint i = 0u; i < camera.max_depth; i++) {
        if(intersectScene(hit, ray)) {
            scatterLambertian(hit, ray, color, pdf);
        }
        else {
            vec3 sky_color = vec3(0.7, 0.8, 1.0) * min(1.0, 1.0 + dot(ray.direction, vec3(0, 1, 0)));
            return color * sky_color / pdf;
        }
    }

    return vec3(0);
}