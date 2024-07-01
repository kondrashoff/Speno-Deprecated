highp uint hq_random_seed;

void setupHQRandomSeed() {
    uint pixel_index = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(resolution.x);
    uint seed = pixel_index * (frame + 1u);

    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);

    hq_random_seed = (seed * 336343633u) | 1;
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
highp uint randomUint() {
    hq_random_seed += (hq_random_seed << 10u);
    hq_random_seed ^= (hq_random_seed >>  6u);
    hq_random_seed += (hq_random_seed <<  3u);
    hq_random_seed ^= (hq_random_seed >> 11u);
    hq_random_seed += (hq_random_seed << 15u);

    return hq_random_seed;
}

highp float floatConstruct(highp uint m) {
    const highp uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const highp uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    highp float  f = uintBitsToFloat(m);   // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float randomFloatHQ() {
    return floatConstruct(randomUint());
}

/*highp uint xorshift() {
    hq_random_seed ^= hq_random_seed << 13;
    hq_random_seed ^= hq_random_seed >> 17;
    hq_random_seed ^= hq_random_seed << 15;
    return hq_random_seed;
}

float randomFloatHQ() {
    return (xorshift() & 0xFFFFFF) / 16777216.0;   
}*/