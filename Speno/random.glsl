void setRandomSeed() {
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

float random8BitFloat() {
    if(stbn.use) {
        return getScalarSTBN();
    }

    return floatConstruct(randomUint());
}

float randomFloat() {
    if(stbn.use) {
        return getVec1STBN();
    }
       
    return floatConstruct(randomUint());
}

vec2 randomVec2() {
    if(stbn.use) {
        return getVec2STBN();
    }
        
    return vec2(randomFloat(), randomFloat());
}

vec3 randomVec3() {
    if(stbn.use) {
        return getVec3STBN();
    }

    return vec3(randomFloat(), randomFloat(), randomFloat());
}