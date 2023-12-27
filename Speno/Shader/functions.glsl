vec4 randomCosineDirectionWithPDF() {
    vec4 data = getUnitvec3cosineSTBN();
    data.rgb = 2.0 * data.rgb - 1.0;
    data.a *= INV_PI;
    return data;

    /*vec2 r = randomVec2();

    float phi = TAU * r.x;
    float x = cos(phi) * sqrt(r.y);
    float y = sin(phi) * sqrt(r.y);
    float z = sqrt(1.0 - r.y);

    return vec3(x, y, z);*/
}

vec3 randomOnSphere() {
    return 2.0 * getUnitvec3STBN() - 1.0;

    /*float r1 = randomFloat();
    float r2 = randomFloat();

    float theta = TAU * r1;
    float phi = acos(2.0 * r2 - 1.0);

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return normalize(vec3(x, y, z));*/
}

vec2 randomInUnitDisk() {
    return 2.0 * getUnitvec2STBN() - 1.0;

    /*float r = sqrt(randomFloat());
    float phi = TAU*randomFloat();

    return vec2(r * sin(phi), r * cos(phi));*/
}

vec2 pixelSampleSquare() {
    vec2 r = 2.0 * getVec2STBN() - 1.0;
    return r / min(resolution.x, resolution.y);
    
    //return (2.0 * randomVec2() - 1.0) / min(resolution.x, resolution.y);
}

Ray getRay() {
    vec2 uv = texCoords;
    uv = 2.0*uv - 1.0;
    uv.x *= resolution.x / resolution.y;
    uv *= camera.tan_half_fov;

    vec3 direction = camera.view[0] * uv.x + camera.view[1] * uv.y + camera.view[2];
    direction = normalize(direction);

    return Ray(camera.lookfrom, direction);
}

vec2 getUV() {
    vec3 direction = normalize(gbuffer_intersection.rgb - camera.lookfrom_prev);

    float dp = camera.tan_half_fov_prev * dot(direction, camera.view_prev[2]);

    vec2 uv;
    uv.y = dot(direction, camera.view_prev[1]) / dp;
    uv.x = dot(direction, camera.view_prev[0]) / dp;

    return uv;
}

Ray getPreviousRay(in vec2 uv) {
    uv = 2.0*uv - 1.0;
    uv.x *= resolution.x / resolution.y;
    uv *= camera.tan_half_fov_prev;
     
    vec3 direction = camera.view_prev[0] * uv.x + camera.view_prev[1] * uv.y + camera.view_prev[2];
    direction = normalize(direction);

    return Ray(camera.lookfrom_prev, direction);
}

mat3 onbBuildFromW(in vec3 w) {
    vec3 a = (abs(w.x) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    vec3 v = normalize(cross(w, a));
    vec3 u = cross(w, v);

    mat3 onb;
    onb[0] = u;
    onb[1] = v;
    onb[2] = w;

    return onb;
}

float scatteringPdf(in vec3 scattered, in vec3 normal) {
    return max(0.0, dot(normal, scattered)) / PI;
}

vec2 spherical_map(in vec3 p) {
    vec2 uv = vec2(atan(p.z, p.x), asin(p.y));
    uv *= vec2(1.0 / TAU, 1.0 / PI); uv += 0.5;
    return uv;
}

vec3 spherical_map(in vec2 uv) {
    uv -= 0.5; uv *= vec2(TAU, PI);
    vec2 s = sin(uv), c = cos(uv);
    return vec3(c.x*c.y, s.y, s.x*c.y);
}

vec2 getTextureUV(in Hit hit, in Triangle triangle) {
    return mix(mix(triangle.uvs[0], triangle.uvs[1], hit.uv.x), triangle.uvs[2], hit.uv.y);
}

vec3 getBackgroundColor(in vec3 direction) {
    if(use_hdri) return texture(hdri_texture, spherical_map(direction)).rgb;
    else return vec3(0.7, 0.8, 1.0);
}

vec3 getFinalColor(in vec3 color, in float pdf, in vec3 light_color, in vec3 light_color_sum, in uint total_light_colors) {
    light_color = clamp(light_color, 0.0, 4.0);
    if(any(isnan(light_color))) light_color = vec3(0.0);

    vec3 final_color = (color * light_color) / pdf;
    final_color = (final_color + light_color_sum) / float(total_light_colors + 1u);
                
    final_color = clamp(final_color, 0.0, 128.0);
    if(any(isnan(final_color))) final_color = vec3(0.0);

    return final_color;
}

vec3 getFinalColor(in vec3 light_color_sum, in uint total_light_colors) {
    vec3 final_color = light_color_sum / float(total_light_colors);
                
    final_color = clamp(final_color, 0.0, 4.0);
    if(any(isnan(final_color))) final_color = vec3(0.0);

    return final_color;
}

vec3 getFinalColor(in vec3 color) {
    vec3 final_color = clamp(color, 0.0, 4.0);
    if(any(isnan(final_color))) final_color = vec3(0.0);

    return final_color;
}