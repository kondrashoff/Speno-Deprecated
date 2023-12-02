vec2 spherical_map(vec3 p) {
    vec2 uv = vec2(atan(p.z, p.x), asin(p.y));
    uv *= vec2(1.0 / TAU, 1.0 / PI); uv += 0.5;
    return uv;
}

vec3 spherical_map(vec2 uv) {
    uv -= 0.5; uv *= vec2(TAU, PI);
    vec2 s = sin(uv), c = cos(uv);
    return vec3(c.x*c.y, s.y, s.x*c.y);
}

vec2 pixelSampleSquare() {
    if(u_samples < 65u) {
        vec2 r = 2.0 * texelFetch(stbn_vec2_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec2_shift++) % 64) % 128, 0).rg - 1.0;
        return r / min(u_resolution.x, u_resolution.y);
    }

    return (2.0 * randomVec2() - 1.0) / min(u_resolution.x, u_resolution.y);
}

vec2 randomInUnitDisk() {
    if(u_samples < 65u) {
        return 2.0 * texelFetch(stbn_unitvec2_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_unitvec2_shift++) % 64) % 128, 0).rg - 1.0;
    }

    float r = sqrt(randomFloat());
    float phi = TAU*randomFloat();

    return vec2(r * sin(phi), r * cos(phi));
}

vec3 randomOnSphere() {
    if(u_samples < 65u) {
        return 2.0 * texelFetch(stbn_unitvec3_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_unitvec3_shift++) % 64) % 128, 0).rgb - 1.0;
    }

    float r1 = randomFloat();
    float r2 = randomFloat();

    float theta = TAU * r1;
    float phi = acos(2.0 * r2 - 1.0);

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return normalize(vec3(x, y, z));
}

vec3 sampleSphere(Sphere sphere) {
    vec3 v = randomOnSphere();
    return v * sphere.radius + sphere.center;
}

vec3 sampleBox(AABB box) {
    return box.min + randomVec3() * (box.max - box.min);
}

vec3 randomCosineDirection() {
    vec2 r = randomVec2();
    
    float phi = TAU * r.x;
    float x = cos(phi) * sqrt(r.y);
    float y = sin(phi) * sqrt(r.y);
    float z = sqrt(1.0 - r.y);

    return vec3(x, y, z);
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

float sphereArea(in Sphere sphere) {
    return 4.0 * PI * sphere.radius * sphere.radius;
}

float boxArea(in AABB box) {
    vec3 size;
    size.x = box.max.x - box.min.x;
    size.y = box.max.y - box.min.y;
    size.z = box.max.z - box.min.z;
    size += vec3(equal(size, vec3(0)));

    return size.x * size.y * size.z;
}

Ray getRay() {
    vec2 uv = gl_FragCoord.xy / u_resolution;

    uv = 2.0*uv - 1.0;
    uv += pixelSampleSquare();
    uv.x *= u_resolution.x / u_resolution.y;

    float tan_half_fov = tan(radians(camera.fov / 2.0));

    vec3 w = normalize(camera.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    //vec3 direction = u * uv.x * tan_half_fov + v * uv.y * tan_half_fov + w;
    vec3 direction = w * 2.0 + u * uv.x + v * uv.y;
    direction = normalize(direction);

    return Ray(camera.lookfrom, direction);
}

vec2 getUV(in Camera cam, in vec3 intersection_point) {
    vec3 to_point = intersection_point - cam.lookfrom;
    vec3 direction = normalize(to_point);

    //float tan_half_fov = tan(radians(cam.fov / 2.0));

    vec3 w = normalize(cam.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    vec3 fwd = w * 2.0;
    float d = dot(w, direction);

    if(d < EPSILON) return vec2(-MAXIMUM_DISTANCE, -MAXIMUM_DISTANCE);

    d = 2.0 / d;

    to_point = direction * d - fwd;

    float x = dot(to_point, u);
    float y = dot(to_point, v);

    return vec2(x, y);

    /*vec3 direction = normalize(intersection_point - cam.lookfrom);

    float tan_half_fov = tan(radians(cam.fov / 2.0));

    vec3 w = normalize(cam.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    float dp = dot(direction, w);

    vec2 uv;
    uv.y = dot(direction, v) / (tan_half_fov * dp);
    uv.x = dot(direction, u) / (tan_half_fov * dp);

    return uv;*/
}

Ray getRay(in Camera cam, in vec2 uv) {
    uv = 2.0*uv - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    //float tan_half_fov = tan(radians(cam.fov / 2.0));

    vec3 w = normalize(cam.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    vec3 direction = w * 2.0 + u * uv.x + v * uv.y; //u * uv.x * tan_half_fov + v * uv.y * tan_half_fov + w;
    direction = normalize(direction);

    return Ray(cam.lookfrom, direction);
}

vec3 getSkyColor(in Ray ray) {
    vec2 uv = spherical_map(ray.direction);
    return texture(hdri_texture, uv).rgb;
}

vec3 getFinalColor(in vec3 color, in float pdf, in vec3 light_color, in vec3 light_color_sum, in uint total_light_colors) {
    light_color = clamp(light_color, 0.0, 2.0);
    if(any(isnan(light_color))) light_color = vec3(0.0);

    vec3 final_color = (color * light_color) / pdf;
    final_color = (final_color + light_color_sum) / float(total_light_colors + 1u);
                
    final_color = clamp(final_color, 0.0, 65535.0);
    if(any(isnan(final_color))) final_color = vec3(0.0);

    return final_color;
}

vec3 getFinalColor(in vec3 light_color_sum, in uint total_light_colors) {
    vec3 final_color = light_color_sum / float(total_light_colors);
                
    final_color = clamp(final_color, 0.0, 65535.0);
    if(any(isnan(final_color))) final_color = vec3(0.0);

    return final_color;
}

vec3 getFinalColor(in vec3 color) {
    vec3 final_color = clamp(color, 0.0, 65535.0);
    if(any(isnan(final_color))) final_color = vec3(0.0);

    return final_color;
}