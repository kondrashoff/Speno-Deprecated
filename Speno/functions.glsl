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

vec3 sampleSphere(vec3 center, float radius) {
    vec3 v = randomOnSphere();
    return v * radius + center;
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

Ray getRay() {
    vec2 uv = gl_FragCoord.xy / u_resolution;

    vec2 uv_offset = pixelSampleSquare();

    uv = 2.0*uv - 1.0;
    uv += uv_offset;
    uv.x *= u_resolution.x / u_resolution.y;

    float tan_half_fov = tan(radians(camera.fov / 2.0));

    vec3 w = normalize(camera.lookdir);
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), w));
    vec3 v = cross(w, u);

    vec3 direction = u * uv.x * tan_half_fov + v * uv.y * tan_half_fov + w;
    direction = normalize(direction);

    return Ray(camera.lookfrom, direction);
}