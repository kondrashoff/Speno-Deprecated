uint random_seed;

void setRootRandomSeed() {
    random_seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(u_resolution.x);
    random_seed *= u_frame_seed + 1u;
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint randomUint() {
    random_seed += (random_seed << 10u);
    random_seed ^= (random_seed >>  6u);
    random_seed += (random_seed <<  3u);
    random_seed ^= (random_seed >> 11u);
    random_seed += (random_seed << 15u);

    return random_seed;
}

float floatConstruct(uint m) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat(m);         // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float randomFloat() { return floatConstruct(randomUint()); }