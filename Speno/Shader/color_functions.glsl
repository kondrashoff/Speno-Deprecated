// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
mat3 ACESinput = mat3(
    vec3(0.59719, 0.35458, 0.04823),
    vec3(0.07600, 0.90834, 0.01566),
    vec3(0.02840, 0.13383, 0.83777)
);

mat3 ACESoutput = mat3(
    vec3(1.60475, -0.53108, -0.07367),
    vec3(-0.10208, 1.10813, -0.00605),
    vec3(-0.00327, -0.07276, 1.07602)
);

vec3 RRTandODTfit(in vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACES(in vec3 color) {
    color *= ACESinput;
    color = RRTandODTfit(color);
    return color * ACESoutput;
}

// https://64.github.io/tonemapping/
vec3 uncharted2_tonemap_partial(vec3 x) {
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 uncharted2(vec3 color) {
    float exposure_bias = 2.0f;
    vec3 curr = uncharted2_tonemap_partial(color * exposure_bias);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

// CCIR 601
float luminance(vec3 color) {
    return dot(color, vec3(0.2989, 0.587, 0.114));
}

/*float luminance(vec3 color) {
	return dot(color, vec3(0.2126, 0.7152, 0.0722));
}*/

float realluminance(vec3 color) {
    return sqrt(dot(color, vec3(0.299, 0.587, 0.114)));
}

float normalizedluminance(vec3 color) {
    float val = luminance(color);
    return pow(log(val + 1.0), 1.0 / 2.2);
}

vec3 clampcolor(vec3 color) {
    float lum = luminance(color);
    return color / (1.0 + lum);
}

vec3 exponent(vec3 color) {
    return 1.0 - exp(-color);
}

vec3 reinhard(vec3 color) {
	float white = 2.0;
	float luma = realluminance(color);
	float toneMappedLuma = luma * (1.0 + luma / (white*white)) / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return color;
}

vec3 reinhard(vec3 color, float white) {
    float luma = realluminance(color);
    float toneMappedLuma = luma * (1.0 + luma / (white*white)) / (1.0 + luma);
    color *= toneMappedLuma / luma;
    return color;
}

vec3 reinhard_extended_luminance(vec3 v, float l_old, float max_white_l) {
    float numerator = l_old * (1.0 + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0 + l_old);
    return v * (l_old / l_new);
}

#define REC709_ALPHA 1.09929682680944
#define REC709_BETA 0.018053968510807

float rec_709_oetf(float x) {
    x = max(x, 0.0);
    if (x < REC709_BETA) x = x * 4.5;
    else x = REC709_ALPHA * pow(x, 0.45) - (REC709_ALPHA - 1.0);
    return x;
}

vec3 linearToREC709(vec3 color) {
    color.r = rec_709_oetf(color.r);
    color.g = rec_709_oetf(color.g);
    color.b = rec_709_oetf(color.b);

    return color;
}

vec3 REC709toREC2020(vec3 RGB709) {
    const mat3 ConvMat = mat3(
        0.627402, 0.329292, 0.043306,
        0.069095, 0.919544, 0.011360,
        0.016394, 0.088028, 0.895578
    );

    return ConvMat * RGB709;
}

vec3 ApplyREC2084Curve(vec3 L) {
    float m1 = 2610.0 / 4096.0 / 4;
    float m2 = 2523.0 / 4096.0 * 128;
    float c1 = 3424.0 / 4096.0;
    float c2 = 2413.0 / 4096.0 * 32;
    float c3 = 2392.0 / 4096.0 * 32;
    vec3 Lp = pow(L, vec3(m1));
    return pow((c1 + c2 * Lp) / (1.0 + c3 * Lp), vec3(m2));
}

vec3 sRGB(vec3 color) {
    bvec3 cutoff = lessThan(color, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(color, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = color * vec3(12.92);

    return mix(higher, lower, cutoff);
}

vec3 sRGBtoLinear(vec3 color) {
    bvec3 cutoff = lessThan(color, vec3(0.04045));
    vec3 higher = pow((color + vec3(0.055)) / vec3(1.055), vec3(2.4));
    vec3 lower = color / vec3(12.92);

    return mix(higher, lower, cutoff);
}