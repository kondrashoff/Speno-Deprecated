void setRandomSeed() {
    if(u_samples < 65u) {
        float stbn_seed = texelFetch(stbn_scalar_texture, ivec3(gl_FragCoord.xy, u_frame % 64) % 128, 0).r;
        random_seed = uint(stbn_seed * 65535.0);

        uvec2 c = uvec2(gl_FragCoord.xy) / 128u;
        uint max_c = uint(u_resolution.y) / 128u;
        uint quad = c.x * max_c + c.y;

        stbn_scalar_shift += quad;
        stbn_vec1_shift += quad;
        stbn_vec2_shift += quad;
        stbn_vec3_shift += quad;
        stbn_unitvec1_shift += quad;
        stbn_unitvec2_shift += quad;
        stbn_unitvec3_shift += quad;
        stbn_unitvec3_cosine_shift += quad;

        return;
    }

    random_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(u_resolution.x);
    random_seed *= u_frame_seed + 1u;
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
highp uint randomUint() {
    random_seed += (random_seed << 10u);
    random_seed ^= (random_seed >>  6u);
    random_seed += (random_seed <<  3u);
    random_seed ^= (random_seed >> 11u);
    random_seed += (random_seed << 15u);

    return random_seed;
}

highp float floatConstruct(highp uint m) {
    const highp uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const highp uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    highp float  f = uintBitsToFloat(m);   // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float randomFloat() {
    if(u_samples < 65u) {
        return texelFetch(stbn_vec1_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec1_shift++) % 64) % 128, 0).r;
    }
       
    return floatConstruct(randomUint());
}

vec2 randomVec2() {
    if(u_samples < 65u) {
        return texelFetch(stbn_vec2_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec2_shift++) % 64) % 128, 0).rg;
    }
        
    return vec2(randomFloat(), randomFloat());
}

vec3 randomVec3() {
    if(u_samples < 65u) {
        return texelFetch(stbn_vec3_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec3_shift++) % 64) % 128, 0).rgb;
    }

    return vec3(randomFloat(), randomFloat(), randomFloat());
}