float random8BitFloat() {
    return getScalarSTBN();
}

float randomFloat() {
    return getVec1STBN();
}

vec2 randomVec2() {
    return getVec2STBN();
}

vec3 randomVec3() {
    return getVec3STBN();
}

/*void setRandomSeed() {
    random_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(resolution.x);
    random_seed *= frame + 1u;
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
    return floatConstruct(randomUint());
}

vec2 randomVec2() {
    return vec2(randomFloat(), randomFloat());
}

vec3 randomVec3() {
    return vec3(randomFloat(), randomFloat(), randomFloat());
}*/