#version 460

out vec4 FragColor;

uniform sampler2D diffuse_texture;
uniform sampler2D albedo_texture;
uniform sampler2D normal_texture;
uniform sampler2D position_texture;
uniform sampler2D previous_frame;

vec3 clamp_color(in vec3 color) {
	float maximum = max(color.r, max(color.g, color.b));

	if(maximum <= 1.0) {
		return color;
	}

	return color / maximum;
}

float luminance(in vec3 color) {
	return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color) {
	float white = 2.0;
	float luma = luminance(color);
	float toneMappedLuma = luma * (1.0 + luma / (white*white)) / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return color;
}

// Source:
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat = mat3(
    vec3(0.59719, 0.35458, 0.04823),
    vec3(0.07600, 0.90834, 0.01566),
    vec3(0.02840, 0.13383, 0.83777)
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3(
    vec3(1.60475, -0.53108, -0.07367),
    vec3(-0.10208, 1.10813, -0.00605),
    vec3(-0.00327, -0.07276, 1.07602)
);

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color) {
    color = color * ACESInputMat;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = color * ACESOutputMat;

    return color;
}

vec3 sRGB(vec3 color) {
    bvec3 cutoff = lessThan(color, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(color, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = color * vec3(12.92);

    return mix(higher, lower, cutoff);
}

vec3 gaussianBlur(sampler2D sampler, vec2 pos) {
    mat3 kernel = mat3(1, 2, 1, 2, 4, 2, 1, 2, 1);
    ivec2 ipos = ivec2(pos);
    float coef = 1.0 / 16.0;
    vec3 color = vec3(0);

    for(uint x = 0u; x < 3u; x++) {
        for(uint y = 0u; y < 3u; y++) {
            color += kernel[x][y] * texelFetch(sampler, ipos + ivec2(x, y) - 1, 0).rgb;
        }
    }

    return coef * color;
}

vec3 denoise(float denoiseStrength) {
	vec2 offset[25];
    offset[0] = vec2(-2,-2);
    offset[1] = vec2(-1,-2);
    offset[2] = vec2(0,-2);
    offset[3] = vec2(1,-2);
    offset[4] = vec2(2,-2);
    
    offset[5] = vec2(-2,-1);
    offset[6] = vec2(-1,-1);
    offset[7] = vec2(0,-1);
    offset[8] = vec2(1,-1);
    offset[9] = vec2(2,-1);
    
    offset[10] = vec2(-2,0);
    offset[11] = vec2(-1,0);
    offset[12] = vec2(0,0);
    offset[13] = vec2(1,0);
    offset[14] = vec2(2,0);
    
    offset[15] = vec2(-2,1);
    offset[16] = vec2(-1,1);
    offset[17] = vec2(0,1);
    offset[18] = vec2(1,1);
    offset[19] = vec2(2,1);
    
    offset[20] = vec2(-2,2);
    offset[21] = vec2(-1,2);
    offset[22] = vec2(0,2);
    offset[23] = vec2(1,2);
    offset[24] = vec2(2,2);
    
    
    float kernel[25];
    kernel[0] = 1.0f/256.0f;
    kernel[1] = 1.0f/64.0f;
    kernel[2] = 3.0f/128.0f;
    kernel[3] = 1.0f/64.0f;
    kernel[4] = 1.0f/256.0f;
    
    kernel[5] = 1.0f/64.0f;
    kernel[6] = 1.0f/16.0f;
    kernel[7] = 3.0f/32.0f;
    kernel[8] = 1.0f/16.0f;
    kernel[9] = 1.0f/64.0f;
    
    kernel[10] = 3.0f/128.0f;
    kernel[11] = 3.0f/32.0f;
    kernel[12] = 9.0f/64.0f;
    kernel[13] = 3.0f/32.0f;
    kernel[14] = 3.0f/128.0f;
    
    kernel[15] = 1.0f/64.0f;
    kernel[16] = 1.0f/16.0f;
    kernel[17] = 3.0f/32.0f;
    kernel[18] = 1.0f/16.0f;
    kernel[19] = 1.0f/64.0f;
    
    kernel[20] = 1.0f/256.0f;
    kernel[21] = 1.0f/64.0f;
    kernel[22] = 3.0f/128.0f;
    kernel[23] = 1.0f/64.0f;
    kernel[24] = 1.0f/256.0f;
    
    vec3 sum = vec3(0.0);
    float c_phi = 1.0;
    float n_phi = 0.5;
    float p_phi = 0.3;
	vec3 cval = gaussianBlur(diffuse_texture, gl_FragCoord.xy);
	vec3 nval = texelFetch(normal_texture, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 pval = texelFetch(position_texture, ivec2(gl_FragCoord.xy), 0).rgb;
    
    float cum_w = 0.0;
    for(int i=0; i<25; i++)
    {
        vec2 uv = gl_FragCoord.xy+offset[i]*denoiseStrength;
        
        vec3 ctmp = gaussianBlur(diffuse_texture, uv);
        vec3 t = cval - ctmp;
        float dist2 = dot(t,t);
        float c_w = min(exp(-(dist2) / c_phi), 1.0);
        
        vec3 ntmp = texelFetch(normal_texture, ivec2(uv), 0).rgb;
        t = nval - ntmp;
        dist2 = max(dot(t,t), 0.0);
        float n_w = min(exp(-(dist2) / n_phi), 1.0);
        
        vec3 ptmp = texelFetch(position_texture, ivec2(uv), 0).rgb;
        t = pval - ptmp;
        dist2 = dot(t,t);
        float p_w = min(exp(-(dist2) / p_phi), 1.0);
        
        float weight = c_w*n_w*p_w;
        sum += ctmp*weight*kernel[i];
        cum_w += weight*kernel[i];
    }

    return sum / cum_w;
}

void main() {
    vec2 uv = gl_FragCoord.xy / textureSize(diffuse_texture, 0);
	//vec4 data = texture(diffuse_texture, uv);

    //vec3 color = data.rgb / data.w;
    vec3 color = denoise(9.0) / texture(diffuse_texture, uv).a;

    color = sRGB(color);
    color = ACESFitted(color);
    
    color *= vec3(1.0) * smoothstep(1.8, 0.5, length(uv * 2.0 - 1.0)) * 0.25 + 0.75;
	
	FragColor = vec4(color, 1.0);
}