const vec3 filter_kernel[37] = {
        {-1, -3, 0.000997008973},
        { 0, -3, 0.00199401795},
        { 1, -3, 0.000997008973},
        {-2, -2, 0.00299102692},
        {-1, -2, 0.0129611167},
        { 0, -2, 0.0219341974},
        { 1, -2, 0.0129611167},
        { 2, -2, 0.00299102692},
        {-3, -1, 0.000997008973},
        {-2, -1, 0.0129611167},
        {-1, -1, 0.0588235294},
        { 0, -1, 0.0967098704},
        { 1, -1, 0.0588235294},
        { 2, -1, 0.0129611167},
        { 3, -1, 0.000997008973},
        {-3,  0, 0.00199401795},
        {-2,  0, 0.0219341974},
        {-1,  0, 0.0967098704},
        { 0,  0, 0.158524427},
        { 1,  0, 0.0967098704},
        { 2,  0, 0.0219341974},
        { 3,  0, 0.00199401795},
        {-3,  1, 0.000997008973},
        {-2,  1, 0.0129611167},
        {-1,  1, 0.0588235294},
        { 0,  1, 0.0967098704},
        { 1,  1, 0.0588235294},
        { 2,  1, 0.0129611167},
        { 3,  1, 0.000997008973},
        {-2,  2, 0.00299102692},
        {-1,  2, 0.0129611167},
        { 0,  2, 0.0219341974},
        { 1,  2, 0.0129611167},
        { 2,  2, 0.00299102692},
        {-1,  3, 0.000997008973},
        { 0,  3, 0.00199401795},
        { 1,  3, 0.000997008973}
};

const vec3 estimation_kernel[25] = {
        {-2.0, -2.0, 0.00366300366},
        {-2.0, -1.0, 0.0146520147},
        {-2.0,  0.0, 0.0256410256},
        {-2.0,  1.0, 0.0146520147},
        {-2.0,  2.0, 0.00366300366},
        {-1.0, -2.0, 0.0146520147},
        {-1.0, -1.0, 0.0586080586},
        {-1.0,  0.0, 0.0952380952},
        {-1.0,  1.0, 0.0586080586},
        {-1.0,  2.0, 0.0146520147},
        { 0.0, -2.0, 0.0256410256},
        { 0.0, -1.0, 0.0952380952},
        { 0.0,  0.0, 0.15018315},
        { 0.0,  1.0, 0.0952380952},
        { 0.0,  2.0, 0.0256410256},
        { 1.0, -2.0, 0.0146520147},
        { 1.0, -1.0, 0.0586080586},
        { 1.0,  0.0, 0.0952380952},
        { 1.0,  1.0, 0.0586080586},
        { 1.0,  2.0, 0.0146520147},
        { 2.0, -2.0, 0.00366300366},
        { 2.0, -1.0, 0.0146520147},
        { 2.0,  0.0, 0.0256410256},
        { 2.0,  1.0, 0.0146520147},
        { 2.0,  2.0, 0.00366300366}
};

const vec3 variance_kernel[9] = {
        { 0,  0, 0.25},
        { 1,  0, 0.125},
        {-1,  0, 0.125},
        { 0,  1, 0.125},
        { 0, -1, 0.125},
        {-1, -1, 0.0625},
        { 1, -1, 0.0625},
        {-1,  1, 0.0625},
        { 1,  1, 0.0625}
};

vec4 safeLoad(sampler2D tex, vec2 uv) {
    vec4 data = texture(tex, uv);

	if(isnan(data.r)) data.r = 0.0;
	if(isnan(data.g)) data.g = 0.0;
    if(isnan(data.b)) data.b = 0.0;
    if(isnan(data.a)) data.a = 0.0;

    return data;
}

vec4 safeLoad(sampler2D tex, ivec2 uv) {
    vec4 data = texelFetch(tex, uv, 0);

	if(isnan(data.r)) data.r = 0.0;
	if(isnan(data.g)) data.g = 0.0;
    if(isnan(data.b)) data.b = 0.0;
    if(isnan(data.a)) data.a = 0.0;

    return data;
}

float normalWeight(vec3 normal, vec3 temp_normal) {
    const float normal_weight_exponent = 8.0;
    return pow(max(0.0, dot(normal, temp_normal)), normal_weight_exponent);
}

float depthWeight(float depth, float temp_depth, vec2 depth_gradient, vec2 offset) {
    const float depth_weight_epsilon = 0.1;
    const float depth_strictness = 1.0;
    return exp(-abs(depth - temp_depth) / (depth_strictness * dot(abs(depth_gradient), abs(offset)) + depth_weight_epsilon));
}

float luminanceWeight(float lum, float temp_lum, float variance) {
    const float luminance_weight_epsilon = 0.01;
    const float color_strictness = 4.0;
    return exp(-abs(lum - temp_lum) / (color_strictness * variance + luminance_weight_epsilon));
}

float luminanceWeight(float lum, float temp_lum) {
    const float color_strictness = 4.0;
    return exp(-abs(lum - temp_lum) / color_strictness);
}