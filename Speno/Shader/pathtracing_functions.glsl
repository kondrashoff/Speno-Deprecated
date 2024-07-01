float getSmartEpsilon(float t) {
    if(t < 0.2) return 2.384185791015625e-7;
    if(t < 0.4) return 4.76837158203125e-7;
    if(t < 0.6) return 9.5367431640625e-7;
    if(t < 5.7) return 1.9073486328125e-6;
    if(t < 11.0) return 3.814697265625e-6;
    if(t < 16.2) return 3.814697265625e-6;
    if(t < 29.4) return 7.62939453125e-6;
    if(t < 61.4) return 1.52587890625e-5;
    if(t < 152.7) return 3.0517578125e-5;
    if(t < 249.1) return 6.103515625e-5;
    if(t < 494.3) return 1.220703125e-4;
    if(t < 995.8) return 2.44140625e-4;
    if(t < 2000.0) return 4.8828125e-4;
    if(t < 4200.0) return 9.765625e-4;
    if(t < 6000.0) return 1.953125e-3;
    if(t < 9000.0) return 3.90625e-3;
    if(t < 30000.0) return 7.8125e-3;
    return 1.5625e-2;
    //return 3.125e-2;
    //return 6.25e-2;
    //return 1.25e-1;
    //return 2.5e-1;
    //return 5e-1;
}

vec2 sphericalMap(vec3 p) {
    vec2 uv = vec2(atan(p.z, p.x), asin(p.y));
    uv *= vec2(1.0 / TAU, 1.0 / PI); uv += 0.5;
    return uv;
}

void handleNoHit() {
    gbufferAlbedo = vec3(1.0);
    gbufferPosition = vec4(-MAXIMUM_FLOAT);
    gbufferNormal = vec3(0.0);
    gbufferGodrays = vec4(0.0);
    gbufferReservoir1 = vec4(0.0);
    gbufferReservoir2 = vec4(0.0);
}

vec3 getBackgroundColor(vec3 direction) {
    //return texture(atmosphereTexture, sphericalMap(direction)).rgb;
    //return 256.0 - 256.0 * exp(-0.00390625 * texture(atmosphereTexture, sphericalMap(direction)).rgb);
    return 64.0 - 64.0 * exp(-0.015625 * texture(atmosphereTexture, sphericalMap(direction)).rgb);
    //return 16.0 - 16.0 * exp(-0.0625 * texture(atmosphereTexture, sphericalMap(direction)).rgb);
    //return 4.0 - 4.0 * exp(-0.25 * texture(atmosphereTexture, sphericalMap(direction)).rgb);
    //return 1.0 - exp(-texture(atmosphereTexture, sphericalMap(direction)).rgb);
}

float balanceHeuristic(float a, float b) {
    return a / (a + b);
}

float powerHeuristic(float a, float b) {
    float a2 = a*a;
    float b2 = b*b;
    return a2 / (a2 + b2);
}

mat3 onbBuildFromW(vec3 w) {
    vec3 a = abs(w.x) > 0.996 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 v = normalize(cross(w, a));
    vec3 u = cross(w, v);

    return mat3(u, v, w);
}

float fresnelSchlick(float nIn, float nOut, vec3 direction, vec3 normal) {
    float R0 = ((nOut - nIn) * (nOut - nIn)) / ((nOut + nIn) * (nOut + nIn));
    float fresnel = R0 + (1.0 - R0) * pow((1.0 - abs(dot(direction, normal))), 5.0);
    return fresnel;
}

float getTriangleArea(Triangle triangle) {
    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];

    return 0.5 * length(cross(e1, e2));
}

vec3 getRandomPointOnTriangle(Triangle triangle) {
    /*vec2 uv = getRandomVec2();

    vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e2 = triangle.vertices[2] - triangle.vertices[0];

    return triangle.vertices[0] + u.x * mix(e1, e2, u.y);*/
    
    for(int attemp = 1; attemp <= 16; attemp++) {
        vec2 r = vec2(randomFloatHQ(), randomFloatHQ());

        float u = sqrt(r.x);
        float v = r.y * u;
        float w = 1.0 - (u + v);

        if(w >= 0.0 || attemp == 16) 
            return u * triangle.vertices[0] + v * triangle.vertices[1] + w * triangle.vertices[2];
    }
}

vec3 getRandomOnSphere() {
    vec2 r = vec2(randomFloatHQ(), randomFloatHQ());

    float theta = TAU * r.x;
    float phi = acos(2.0 * r.y - 1.0);

    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

    return vec3(x, y, z);
}

float scatteringPDF(vec3 scattered, vec3 normal) {
    return dot(scattered, normal) * INV_PI;
}

float scatteringGGXPDF(float roughness, float HdotN, float VdotH) {
    float HdotN2 = HdotN * HdotN;
    float roughness2 = roughness * roughness;

    float t = HdotN2 * roughness2 - HdotN2 + 1.0;
    float D = roughness2 / (t * t) * INV_PI;
    return max(D * HdotN / (4.0 * abs(VdotH)), 0.0);
}

vec4 getRandomCosineDirectionWithPDF() {
    vec2 r = vec2(randomFloatHQ(), randomFloatHQ());

    float phi = TAU * r.x;
    float sqrt_ry = sqrt(r.y);

    float x = cos(phi) * sqrt_ry;
    float y = sin(phi) * sqrt_ry;
    float z = sqrt(1.0 - r.y);

    vec3 direction = vec3(x, y, z);
    float pdf = PI / direction.z;

    return vec4(direction, pdf);
}

vec3 getRandomGGX(float a) {
    vec2 rand = vec2(randomFloatHQ(), randomFloatHQ());
   
    float phi = TAU * rand.y;
    float a2 = a * a;

    float cos_theta = sqrt((1.0 - rand.x) / (rand.x * (a2 - 1.0) + 1.0));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    float x = sin_theta * cos(phi);
    float y = sin_theta * sin(phi);
    float z = cos_theta;

    return vec3(x, y, z);
}