vec2 pixelSampleSquare() {
    return (2.0 * vec2(randomFloat(), randomFloat()) - 1.0) / min(u_resolution.x, u_resolution.y);
}

vec2 randomInUnitDisk() {
    float r = sqrt(randomFloat());
    float phi = TAU*randomFloat();

    return vec2(r * sin(phi), r * cos(phi));
}

vec3 randomOnSphere() {
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
    vec3 v;
    v.x = randomFloat(box.min.x, box.max.x);
    v.y = randomFloat(box.min.y, box.max.y);
    v.z = randomFloat(box.min.z, box.max.z);
    return v;
}

vec3 randomCosineDirection() {
    float r1 = randomFloat();
    float r2 = randomFloat();

    float phi = TAU*r1;
    float x = cos(phi)*sqrt(r2);
    float y = sin(phi)*sqrt(r2);
    float z = sqrt(1.0 - r2);

    return vec3(x, y, z);
}

vec3 randomCosineDirectionFromNormal(in vec3 normal) {
    // compute basis from normal
    vec3 tc = vec3(1.0 + normal.z - normal.xy * normal.xy, -normal.x * normal.y) / (1.0 + normal.z);
    vec3 uu = vec3(tc.x, tc.z, -normal.x);
    vec3 vv = vec3(tc.z, tc.y, -normal.y);
    
    float u = randomFloat();
    float v = randomFloat();
    float a = TAU * v;

    return normalize(sqrt(u) * (cos(a) * uu + sin(a) * vv) + sqrt(1.0 - u) * normal);
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
    uv.x *= u_resolution.x / u_resolution.y;

    float tan_half_fov = tan(radians(camera.fov / 2.0));

    vec3 w = normalize(camera.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    vec3 direction = u * uv.x * tan_half_fov + v * uv.y * tan_half_fov + w;
    direction = normalize(direction);

    return Ray(camera.lookfrom, direction);
}

vec2 getUV(in Camera cam, in vec3 intersection_point) {
    vec3 direction = normalize(intersection_point - cam.lookfrom);

    float tan_half_fov = tan(radians(cam.fov / 2.0));

    vec3 w = normalize(cam.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    float dp = dot(direction, w);

    vec2 uv;
    uv.y = dot(direction, v) / (tan_half_fov * dp);
    uv.x = dot(direction, u) / (tan_half_fov * dp);

    return uv;
}

Ray getRay(in Camera cam, in vec2 uv) {
    uv = 2.0*uv - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    float tan_half_fov = tan(radians(cam.fov / 2.0));

    vec3 w = normalize(cam.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    vec3 direction = u * uv.x * tan_half_fov + v * uv.y * tan_half_fov + w;
    direction = normalize(direction);

    return Ray(cam.lookfrom, direction);
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

vec3 getFinalColor(in vec3 color, in vec3 light_color_sum, in uint total_light_colors) {
    vec3 final_color = (1.0 / float(camera.max_depth)) * 0.5 * color;
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

vec4 getLoadingScreen() {
    vec2 vec = gl_FragCoord.xy - 0.5 * u_resolution.xy;
    float dist = length(vec);
    float angle = atan(vec.x,vec.y) / (PI*0.25);
    float frac = fract(angle);
    float light = sign(angle - frac+4.0 - floor(mod(u_time * 10.0, 8.0)));

    float a = smoothstep(100.0, 97.0, dist) * smoothstep(47.0, 50.0, dist) * 0.5*smoothstep(0.0, 0.05, frac) * smoothstep(1.0, 0.95, frac);
    a += a * (-abs(light) + 1.0);

    return vec4(a);
}