vec3 reBlur(float denoiseStrength) {
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
    float n_phi = 0.5;
    float p_phi = 0.3;
	vec3 nval = texelFetch(gbufferNormal, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 pval = texelFetch(gbufferPosition, ivec2(gl_FragCoord.xy), 0).rgb;
    
    float cum_w = 0.0;
    for(int i=0; i<37; i++) {
        vec2 uv = gl_FragCoord.xy+offset[i]*denoiseStrength;
        vec3 ctmp = texelFetch(gbufferDiffuse, ivec2(uv), 0).rgb;
        
        vec3 ntmp = texelFetch(gbufferNormal, ivec2(uv), 0).rgb;
        vec3 t = nval - ntmp;
        float dist2 = max(dot(t,t), 0.0);
        float n_w = min(exp(-(dist2) / n_phi), 1.0);
        
        vec3 ptmp = texelFetch(gbufferPosition, ivec2(uv), 0).rgb;
        t = pval - ptmp;
        dist2 = dot(t,t);
        float p_w = min(exp(-(dist2) / p_phi), 1.0);
        
        float weight = n_w*p_w;
        sum += ctmp*weight*kernel[i];
        cum_w += weight*kernel[i];
    }

    return sum / cum_w;
}