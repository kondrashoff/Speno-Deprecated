#version 460

layout(location = 0) out vec4 FragColor;

uniform sampler2D diffuse_texture;
uniform sampler2D albedo_texture;
uniform sampler2D normal_texture;
uniform sampler2D position_texture;
uniform sampler2D velocity_texture;
uniform sampler2D previous_denoised_texture;

uniform float u_delta_time;

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

    return clamp(coef * color, 0.0, 1.0);
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
    kernel[0] = 1.0/256.0;
    kernel[1] = 1.0/64.0;
    kernel[2] = 3.0/128.0;
    kernel[3] = 1.0/64.0;
    kernel[4] = 1.0/256.0;
    
    kernel[5] = 1.0/64.0;
    kernel[6] = 1.0/16.0;
    kernel[7] = 3.0/32.0;
    kernel[8] = 1.0/16.0;
    kernel[9] = 1.0/64.0;
    
    kernel[10] = 3.0/128.0;
    kernel[11] = 3.0/32.0;
    kernel[12] = 9.0/64.0;
    kernel[13] = 3.0/32.0;
    kernel[14] = 3.0/128.0;
    
    kernel[15] = 1.0/64.0;
    kernel[16] = 1.0/16.0;
    kernel[17] = 3.0/32.0;
    kernel[18] = 1.0/16.0;
    kernel[19] = 1.0/64.0;
    
    kernel[20] = 1.0/256.0;
    kernel[21] = 1.0/64.0;
    kernel[22] = 3.0/128.0;
    kernel[23] = 1.0/64.0;
    kernel[24] = 1.0/256.0;
    
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

vec3 denoise2(float denoiseStrength) {
	vec2 offset[37];
    offset[0] = vec2(-1, -3);
    offset[1] = vec2(0,  -3);
    offset[2] = vec2(1,  -3);

    offset[3] = vec2(-2, -2);
    offset[4] = vec2(-1, -2);
    offset[5] = vec2(0,  -2);
    offset[6] = vec2(1,  -2);
    offset[7] = vec2(2,  -2);

    offset[8]  = vec2(-3, -1);
    offset[9]  = vec2(-2, -1);
    offset[10] = vec2(-1, -1);
    offset[11] = vec2(0,  -1);
    offset[12] = vec2(1,  -1);
    offset[13] = vec2(2,  -1);
    offset[14] = vec2(3,  -1);
    
    offset[15] = vec2(-3, 0);
    offset[16] = vec2(-2, 0);
    offset[17] = vec2(-1, 0);
    offset[18] = vec2(0,  0);
    offset[19] = vec2(1,  0);
    offset[20] = vec2(2,  0);
    offset[21] = vec2(3,  0);
    
    offset[22] = vec2(-3, 1);
    offset[23] = vec2(-2, 1);
    offset[24] = vec2(-1, 1);
    offset[25] = vec2(0,  1);
    offset[26] = vec2(1,  1);
    offset[27] = vec2(2,  1);
    offset[28] = vec2(3,  1);

    offset[29] = vec2(-2, 2);
    offset[30] = vec2(-1, 2);
    offset[31] = vec2(0,  2);
    offset[32] = vec2(1,  2);
    offset[33] = vec2(2,  2);

    offset[34] = vec2(-1, 3);
    offset[35] = vec2(0,  3);
    offset[36] = vec2(1,  3);
    
    float kernel[37];
    kernel[0] = 1.0 / 1003.0;
    kernel[1] = 2.0 / 1003.0;
    kernel[2] = 1.0 / 1003.0;

    kernel[3] = 3.0 / 1003.0;
    kernel[4] = 13.0 / 1003.0;
    kernel[5] = 22.0 / 1003.0;
    kernel[6] = 13.0 / 1003.0;
    kernel[7] = 3.0 / 1003.0;

    kernel[8]  = 1.0 / 1003.0;
    kernel[9]  = 13.0 / 1003.0;
    kernel[10] = 59.0 / 1003.0;
    kernel[11] = 97.0 / 1003.0;
    kernel[12] = 59.0 / 1003.0;
    kernel[13] = 13.0 / 1003.0;
    kernel[14] = 1.0 / 1003.0;
    
    kernel[15] = 2.0 / 1003.0;
    kernel[16] = 22.0 / 1003.0;
    kernel[17] = 97.0 / 1003.0;
    kernel[18] = 159.0 / 1003.0;
    kernel[19] = 97.0 / 1003.0;
    kernel[20] = 22.0 / 1003.0;
    kernel[21] = 2.0 / 1003.0;
    
    kernel[22] = 1.0 / 1003.0;
    kernel[23] = 13.0 / 1003.0;
    kernel[24] = 59.0 / 1003.0;
    kernel[25] = 97.0 / 1003.0;
    kernel[26] = 59.0 / 1003.0;
    kernel[27] = 13.0 / 1003.0;
    kernel[28] = 1.0 / 1003.0;

    kernel[29] = 3.0 / 1003.0;
    kernel[30] = 13.0 / 1003.0;
    kernel[31] = 22.0 / 1003.0;
    kernel[32] = 13.0 / 1003.0;
    kernel[33] = 3.0 / 1003.0;

    kernel[34] = 1.0 / 1003.0;
    kernel[35] = 2.0 / 1003.0;
    kernel[36] = 1.0 / 1003.0;
    
    vec3 sum = vec3(0.0);
    float c_phi = 1.0;
    float n_phi = 0.5;
    float p_phi = 0.3;
	vec3 cval = gaussianBlur(diffuse_texture, gl_FragCoord.xy);
	vec3 nval = texelFetch(normal_texture, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 pval = texelFetch(position_texture, ivec2(gl_FragCoord.xy), 0).rgb;
    
    float cum_w = 0.0;
    for(int i=0; i<37; i++) {
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

float calculateAverageLuminance(in float previous_average_luminance, in vec2 resolution) {
    int lod = int(log2(min(resolution.x, resolution.y))) - 1;
    vec2 resolution2 = textureSize(diffuse_texture, lod);

    vec3 sum_cols;

    for(int x = 0; x < resolution2.x; x++) {
       for(int y = 0; y < resolution2.y; y++) {
           sum_cols += texelFetch(diffuse_texture, ivec2(x, y), lod).rgb;
       }
    }

    sum_cols /= (resolution2.x * resolution2.y);

    float average_luminance = dot(sum_cols, vec3(0.2126, 0.7152, 0.0722));
    float coef = min(u_delta_time / 5.0, 1.0);

    return mix(previous_average_luminance, average_luminance, coef);
}

void main() {
    vec2 resolution = textureSize(diffuse_texture, 0);
    
    float a = 1.0;
    bool cannot_be_reprojected = false;
    if(ivec2(gl_FragCoord.xy) == ivec2(0)) {
        cannot_be_reprojected = true;
        float prev_avg_lum = texelFetch(previous_denoised_texture, ivec2(0), 0).a;
        float avg_luminance = calculateAverageLuminance(prev_avg_lum, resolution);
        a = avg_luminance;
    }

    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 color;
    if(texture(position_texture, uv).a > 0.0) {
        //color = gaussianBlur(diffuse_texture, gl_FragCoord.xy);
        color = texture(diffuse_texture, uv).rgb;
        //color = denoise2(2.0);
    }
    else {
       color = texture(diffuse_texture, uv).rgb;
       FragColor = vec4(color, a);
       return;
    }

    if(!cannot_be_reprojected) {
        vec4 reproj_data = texture(velocity_texture, uv);

        if(reproj_data.a == 1.0) {
            vec3 prev_color = texture(previous_denoised_texture, reproj_data.rg).rgb;
            float accum_frames = texture(previous_denoised_texture, reproj_data.rg).a + 1.0;
            color = mix(prev_color, color, 1.0 / accum_frames);
            FragColor = vec4(color, accum_frames);
            return;
        }
    }
	
	FragColor = vec4(color, a);
}