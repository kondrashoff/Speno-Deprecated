void setRandomSeed() {
    if(u_samples < 65u) {
        highp float stbn_seed = texelFetch(stbn_vec1_texture, ivec3(gl_FragCoord.xy, u_frame % 64) % 128, 0).r;
        if(stbn_seed > EPSILON && stbn_seed < (1.0 - EPSILON)) {
            random_seed = floatBitsToUint(stbn_seed);
        }
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
    /*if(u_samples < 65u) {
        float r = texelFetch(stbn_vec1_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec1_shift++) % 64) % 128, 0).r;
        if(r > EPSILON && r < (1.0 - EPSILON)) return r;
    }*/
       
    return floatConstruct(randomUint());
}

vec2 randomVec2() {
    /*if(u_samples < 65u) {
        vec2 r = texelFetch(stbn_vec2_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec2_shift++) % 64) % 128, 0).rg; 
        
        bool cond1 = all(greaterThan(r, vec2(EPSILON)));
        bool cond2 = all(lessThan(r, vec2(1.0 - EPSILON)));
        if(cond1 && cond2) return r;
    }*/
        
    return vec2(randomFloat(), randomFloat());
}

vec3 randomVec3() {
    /*if(u_samples < 65u) {
        vec3 r = texelFetch(stbn_vec3_texture, ivec3(gl_FragCoord.xy, (u_frame + stbn_vec3_shift++) % 64) % 128, 0).rgb;
        
        bool cond1 = all(greaterThan(r, vec3(EPSILON)));
        bool cond2 = all(lessThan(r, vec3(1.0 - EPSILON)));
        if(cond1 && cond2) return r;
    }*/

    return vec3(randomFloat(), randomFloat(), randomFloat());
}