#version 460
#extension GL_ARB_bindless_texture : require

precision highp float;
precision highp int;

in vec2 texCoords;
uniform vec2 resolution;

layout(location = 0) out vec3 gbufferDiffuse;
layout(location = 1) out vec3 gbufferAlbedo;
layout(location = 2) out vec4 gbufferPosition;
layout(location = 3) out vec3 gbufferNormal;
layout(location = 4) out vec4 gbufferGodrays;
layout(location = 5) out vec4 gbufferReservoir1;
layout(location = 6) out vec4 gbufferReservoir2;

layout(bindless_sampler) uniform sampler2D previousReservoir1Texture;
layout(bindless_sampler) uniform sampler2D previousReservoir2Texture;
layout(bindless_sampler) uniform sampler2D atmosphereTexture;

#include "shared_engine_data.glsl"
#include "atmosphere_data.glsl"
#include "color_functions.glsl"

#include "defines.glsl"
#include "random.glsl"
#include "stbn.glsl"
#include "noise.glsl"
#include "camera.glsl"
#include "scene.glsl"
#include "hit.glsl"

#include "pathtracing_functions.glsl"
#include "intersection_functions.glsl"

#define RadianceData() RadianceData(vec3(0.0), vec3(1.0), vec3(0.0), vec3(0.0), 0.0, 0.0)
struct RadianceData {
    vec3 color;
    vec3 absorbed;

    vec3 direct;
    vec3 indirect;

    float indirect_pdf;
    float direct_pdf;
};

#define Reservoir() Reservoir(vec3(0.0), 0.0, 0.0, 0)
struct Reservoir {
    vec3 position;
    float weight_sum;
    float target_pdf;
    int count;
};

struct BrdfData {
    vec3 P, V, N, C, E, R;
    float A;
};

void updateRD(inout RadianceData rd) {
    rd.direct = vec3(0.0);
    rd.indirect = vec3(0.0);
    rd.indirect_pdf = 0.0;
    rd.direct_pdf = 0.0;
}

BrdfData getBrdfData(HitData hit, Ray ray) {
    return BrdfData(hit.position, -ray.direction, hit.normal, hit.color, hit.emission, ray.origin + ray.direction * (hit.t - 0.04), hit.roughness);
};

void compressReservoir(inout Reservoir res) {
    if(res.count == 0) return;

    res.weight_sum /= res.count;
    res.count = 1;
}

void combineReservoirs(inout Reservoir best, Reservoir candidate) {
    float weight_sum = best.weight_sum + candidate.weight_sum;
    int count = best.count + candidate.count;

    best = candidate.weight_sum > weight_sum * randomFloatHQ() ? candidate : best;
    best.weight_sum = weight_sum;
    best.count = count;
};

void setupGBuffers(HitData hit) {
    if(hit.was_intersection) {
        gbufferAlbedo = hit.color;
        gbufferPosition = vec4(hit.position, hit.t);
        gbufferNormal = hit.normal;
    }
    else {
        gbufferAlbedo = vec3(1.0);
        gbufferPosition = vec4(-MAXIMUM_FLOAT);
        gbufferNormal = vec3(0.0);
    }
}

void scatterRay(BrdfData bd, inout Ray ray, inout RadianceData rd, int depth, inout bool use_next_normals) {
    ray.origin = bd.R;//bd.P + 0.004 * bd.N;

    if(bd.A == 1.0) {
#ifdef UNIFORM_HEMISPHERE_SAMPLING
        vec3 direction = getRandomOnSphere();
        ray.direction = faceforward(direction, bd.N, -direction);

        rd.indirect_pdf = TAU;
        rd.absorbed *= bd.C * dot(ray.direction, bd.N) * 2.0;
        rd.indirect = bd.E;
#else
        mat3 onb = onbBuildFromW(bd.N);
        vec4 unitvec3cosine_data = getRandomCosineDirectionWithPDF();
        ray.direction = normalize(onb * unitvec3cosine_data.xyz);
        
        rd.indirect_pdf = unitvec3cosine_data.w;
        rd.absorbed *= bd.C;
        rd.indirect = bd.E;
#endif
    }
    else if(bd.A == 0.0) {
        ray.direction = reflect(ray.direction, bd.N);
        use_next_normals = true;

        rd.indirect_pdf = 1.0;
        rd.absorbed *= bd.C;
        rd.indirect = bd.E;
    }
    else {
        mat3 onb = onbBuildFromW(bd.N);
        vec3 halfway = getRandomGGX(bd.A);
        vec3 microfacet_normal = onb * halfway;

        float HdotN = dot(microfacet_normal, bd.N);
        float VdotH = dot(-ray.direction, microfacet_normal);

        ray.direction = reflect(ray.direction, microfacet_normal);

        // TODO: figure out how to use ggx pdf
        rd.indirect_pdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
        rd.absorbed *= bd.C;// * rd.indirect_pdf;
        rd.indirect = bd.E;
    }
}

bool sampleSun(BrdfData bd, inout RadianceData rd, int depth) {
    if(bd.A == 0.0) return false;

    Ray ray;
    ray.origin = bd.R;//bd.P + 0.004 * bd.N;

	ray.direction = normalize(atmosphere_direction + 0.00436 * getRandomOnSphere());

    vec3 color = getBackgroundColor(ray.direction);
    if(luminance(color) < LOW_EPSILON || dot(ray.direction, bd.N) < 0.0) return false;

    float brdf;
    if(bd.A < 1.0) {
        vec3 H = normalize(bd.V + ray.direction);

        float HdotN = dot(H, bd.N);
        float VdotH = dot(bd.V, H);
        
        brdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
    }
    else brdf = scatteringPDF(ray.direction, bd.N);

    HitData hit = HitData(); 
    if(brdf < LOW_EPSILON || traverse(hit, ray, MAXIMUM_FLOAT, true)) return false;
    
    float source_pdf = 1.01606 * brdf;
    vec3 emission = (bd.E == vec3(0.0) ? vec3(1.0) : bd.E);
    rd.direct = rd.absorbed * bd.C * emission * source_pdf * color;
    rd.direct_pdf = 1.01606;
	return true;
}

bool sampleFlashlight(BrdfData bd, inout RadianceData rd, int depth) {
    if(bd.A == 0.0) return false;

    Ray ray;
    ray.origin = bd.R;//bd.P + 0.004 * bd.N;

    vec3 to_light = camera.lookfrom - ray.origin;
    float dist = length(to_light);
	ray.direction = to_light / dist;

    const vec3 color = 150.0 * vec3(1.0, 0.8509803921568627, 0.7137254901960784);

    float brdf;
    if(bd.A < 1.0) {
        vec3 H = normalize(bd.V + ray.direction);

        float HdotN = dot(H, bd.N);
        float VdotH = dot(bd.V, H);
        
        brdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
    }
    else brdf = scatteringPDF(ray.direction, bd.N);

    AABB root = getBVHRootBounds();
	float radius = 0.5 * length(root.maximum - root.minimum);
    float area = 0.001257 * radius * radius;

    const float angle = 0.94;
    float light_cosine = -dot(camera.front, ray.direction);
    float light_power = area * (light_cosine > angle ? (light_cosine - angle) / (1.0 - angle) : 0.0) / max(dist * dist, radius);

    HitData hit = HitData(); 
    if(brdf < LOW_EPSILON || light_power < LOW_EPSILON || traverse(hit, ray, dist, true)) return false;
    
    float source_pdf = light_power * brdf;
    vec3 emission = (bd.E == vec3(0.0) ? vec3(1.0) : bd.E);
    rd.direct = rd.absorbed * bd.C * emission * source_pdf * color;
    rd.direct_pdf = source_pdf;
	return true;
}

Reservoir getSkySampleReservoir(BrdfData bd) {
    Ray ray;
    ray.origin = bd.R;//bd.P + 0.004 * bd.N;

    vec3 rsphere = getRandomOnSphere();
    ray.direction = faceforward(rsphere, bd.N, -rsphere);

    float brdf_pdf;
    if(bd.A < 1.0) {
        vec3 H = normalize(bd.V + ray.direction);

        float HdotN = dot(H, bd.N);
        float VdotH = dot(bd.V, H);

        brdf_pdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
    }
    else brdf_pdf = scatteringPDF(ray.direction, bd.N);

    if(brdf_pdf < LOW_EPSILON) return Reservoir();

    vec3 sky_color = getBackgroundColor(ray.direction);
    if(luminance(sky_color) < LOW_EPSILON) return Reservoir();

    float source_pdf = brdf_pdf * 4.0 * PI;
    float target_pdf = luminance(sky_color);
    float weight = target_pdf * source_pdf;

    if(weight < LOW_EPSILON || isinf(weight)) return Reservoir();

    Reservoir sky_reservoir = Reservoir();
    sky_reservoir.position = ray.direction;
    sky_reservoir.target_pdf = -target_pdf;
    sky_reservoir.weight_sum = weight;
    sky_reservoir.count = 1;
    return sky_reservoir;
}

Reservoir getSunSampleReservoir(BrdfData bd) {
    Ray ray;
    ray.origin = bd.R;//bd.P + 0.004 * bd.N;

	ray.direction = normalize(atmosphere_direction + 0.00436 * getRandomOnSphere());

    float brdf_pdf;
    if(bd.A < 1.0) {
        vec3 H = normalize(bd.V + ray.direction);

        float HdotN = dot(H, bd.N);
        float VdotH = dot(bd.V, H);

        brdf_pdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
    }
    else brdf_pdf = scatteringPDF(ray.direction, bd.N);

    if(brdf_pdf < LOW_EPSILON) return Reservoir();

    vec3 sun_color = getBackgroundColor(ray.direction);
    if(luminance(sun_color) < LOW_EPSILON) return Reservoir();

    float source_pdf = 1.0774 * brdf_pdf;
    float target_pdf = luminance(sun_color);
    float weight = target_pdf * source_pdf;

    if(weight < LOW_EPSILON || isinf(weight)) return Reservoir();

    Reservoir sun_reservoir = Reservoir();
    sun_reservoir.position = ray.direction;
    sun_reservoir.target_pdf = -target_pdf;
    sun_reservoir.weight_sum = weight;
    sun_reservoir.count = 1;
    return sun_reservoir;
}

void ReSTIR(inout Reservoir best, int depth, BrdfData bd) {
    Ray ray = getRay(texCoords);
    if(dot(ray.direction, bd.P - ray.origin) < 0.0) return;

    //int kernel_dist = 5;
    vec2 reservoir_tex_resolution = textureSize(previousReservoir1Texture, 0);
    ivec2 prev_uv = ivec2(getPreviousUV(bd.P) * reservoir_tex_resolution);

    /*if(depth == 0) {
        float center_pdf = texelFetch(previousReservoir1Texture, prev_uv, 0).w;
        if(center_pdf == 0.0) kernel_dist = 10;
    }

    Reservoir spatial = Reservoir();

    ray = Ray(bd.P + 0.004 * bd.N, vec3(0.0), vec3(0.0));
    for(int i = -kernel_dist; i <= kernel_dist; i++) {
        for(int j = -kernel_dist; j <= kernel_dist; j++) {
            ivec2 coord = prev_uv + ivec2(i, j);

            if(coord.x < 0) { if(depth != 0) continue; else coord.x = 0; }
            if(coord.y < 0) { if(depth != 0) continue; else coord.y = 0; }
            if(coord.x >= reservoir_tex_resolution.x) { if(depth != 0) continue; else coord.x = int(reservoir_tex_resolution.x) - 1; }
            if(coord.y >= reservoir_tex_resolution.y) { if(depth != 0) continue; else coord.y = int(reservoir_tex_resolution.y) - 1; }

            vec4 data = texelFetch(previousReservoir1Texture, coord, 0);
            
            float target_pdf = data.w;
            if(target_pdf == 0.0) continue;

            vec3 position = data.xyz;
            if(target_pdf == spatial.target_pdf && spatial.position == position) continue;

            ray.direction = target_pdf < 0.0 ? position : normalize(position - ray.origin);
            if(dot(ray.direction, bd.N) < 0.0) continue;

            float brdf_pdf;
            if(bd.A < 1.0) {
                vec3 H = normalize(bd.V + ray.direction);

                float HdotN = dot(H, bd.N);
                float VdotH = dot(bd.V, H);

                brdf_pdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
            }
            else brdf_pdf = scatteringPDF(ray.direction, bd.N);

            if(brdf_pdf < LOW_EPSILON) continue;
            
            float dist = target_pdf < 0.0 ? 0.0 : distance(ray.origin, position);
            float source_pdf = brdf_pdf / (1.0 + dist);
            float weight = abs(target_pdf) * source_pdf;

            if(weight < LOW_EPSILON || isinf(weight)) continue;

            Reservoir prev = Reservoir();
            prev.position = position;
            prev.target_pdf = target_pdf;
            prev.weight_sum = weight;
            prev.count = 1;

            combineReservoirs(spatial, prev);
        }
    }

    combineReservoirs(best, spatial);*/

    /*if(prev_uv.x < 0) { if(depth != 0) return; else prev_uv.x = 0; }
    if(prev_uv.y < 0) { if(depth != 0) return; else prev_uv.y = 0; }
    if(prev_uv.x >= reservoir_tex_resolution.x) { if(depth != 0) return; else prev_uv.x = int(reservoir_tex_resolution.x) - 1; }
    if(prev_uv.y >= reservoir_tex_resolution.y) { if(depth != 0) return; else prev_uv.y = int(reservoir_tex_resolution.y) - 1; }*/

    if(prev_uv.x < 0 || prev_uv.y < 0) return;
    if(prev_uv.x >= reservoir_tex_resolution.x) return;
    if(prev_uv.y >= reservoir_tex_resolution.y) return;

    vec4 data = texelFetch(previousReservoir1Texture, prev_uv, 0);
            
    float target_pdf = data.w;
    if(target_pdf == 0.0) return;

    vec3 position = data.xyz;
    if(target_pdf == best.target_pdf && best.position == position) return;

    ray = Ray(bd.P + 0.004 * bd.N, vec3(0.0), vec3(0.0));
    ray.direction = target_pdf < 0.0 ? position : normalize(position - ray.origin);

    if(dot(ray.direction, bd.N) < 0.0) return;

    float source_pdf;
    if(bd.A < 1.0) {
        vec3 H = normalize(bd.V + ray.direction);

        float HdotN = dot(H, bd.N);
        float VdotH = dot(bd.V, H);

        source_pdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
    }
    else source_pdf = scatteringPDF(ray.direction, bd.N);

    if(source_pdf < LOW_EPSILON) return;
            
    float dist = target_pdf < 0.0 ? 1.0 : distance(ray.origin, position);
    float weight = abs(target_pdf) * source_pdf / (dist * dist);

    if(weight < LOW_EPSILON || isinf(weight)) return;

    Reservoir prev = Reservoir();
    prev.position = position;
    prev.target_pdf = target_pdf;
    prev.weight_sum = weight;
    prev.count = 1;

    combineReservoirs(best, prev);
}

bool lightSampling(BrdfData bd, inout RadianceData rd, int depth) {
    if(bd.A == 0.0) return false;

    int light_count = light_ids.length();
    int number_of_triangles = triangles.length();

    HitData hit = HitData();
    //Ray ray = Ray(bd.P + 0.004 * bd.N, vec3(0.0), vec3(0.0));
    Ray ray = Ray(bd.R, vec3(0.0), vec3(0.0));

    Reservoir best = Reservoir();
    combineReservoirs(best, getSunSampleReservoir(bd));
    combineReservoirs(best, getSkySampleReservoir(bd));
    compressReservoir(best);

    int light_samples = min(8, light_count);

    for(int i = 0; i < light_samples; i++) {
        float stratified_random = (float(i - 1) + randomFloatHQ()) / float(light_samples);
        int light_index = int(stratified_random * float(light_count));
        int index = light_ids[light_index];

        Triangle light_triangle = triangles[index];
        vec3 position = getRandomPointOnTriangle(light_triangle);

        ray.direction = normalize(position - ray.origin);
        if(dot(ray.direction, bd.N) < 0.0) continue;

        float brdf_pdf;
        if(bd.A < 1.0) {
            vec3 H = normalize(bd.V + ray.direction);

            float HdotN = dot(H, bd.N);
            float VdotH = dot(bd.V, H);

            brdf_pdf = scatteringGGXPDF(bd.A, HdotN, VdotH);
        }
        else brdf_pdf = scatteringPDF(ray.direction, bd.N);

        if(brdf_pdf < LOW_EPSILON) continue;

        hit = HitData();
        if(!intersectTriangle(hit, ray, light_triangle)) continue;

        float target_pdf = luminance(hit.emission);
        if(target_pdf < LOW_EPSILON) continue;

        float distance_squared = hit.t * hit.t;
        float light_cosine = abs(dot(ray.direction, hit.normal));
        float light_area = getTriangleArea(light_triangle);
        float light_pdf = distance_squared / (light_area * light_cosine);

        if(light_pdf < LOW_EPSILON) continue;

        float source_pdf = float(light_count) * brdf_pdf / light_pdf;
        float weight = target_pdf * source_pdf;

        if(weight < LOW_EPSILON) continue;

        Reservoir candidate = Reservoir();
        candidate.position = position;
        candidate.target_pdf = target_pdf;
        candidate.weight_sum = weight;
        candidate.count = 1;

        combineReservoirs(best, candidate);
    }
    
    compressReservoir(best);

    ReSTIR(best, depth, bd); // Still very biased

    if(best.count == 0) return false;

    if(best.target_pdf < 0.0) {
        ray.direction = best.position;
        if(traverse(hit, ray, MAXIMUM_FLOAT, true)) return false;

        float weight = best.weight_sum / (float(best.count) * -best.target_pdf);
        vec3 emission = (bd.E == vec3(0.0) ? vec3(1.0) : bd.E);
        vec3 ls_emission = getBackgroundColor(ray.direction);
        
        rd.direct = rd.absorbed * weight * emission * bd.C * ls_emission;
        rd.direct_pdf = weight;

        float target_pdf = luminance(ls_emission);

        if(depth == 0 || gbufferReservoir1.w == 0.0) {
            gbufferReservoir1 = vec4(best.position, -target_pdf);
        }

        return true;
    }

    vec3 direction = best.position - ray.origin;
    float dist = length(direction);
    ray.direction = direction / dist;
    traverse(hit, ray, dist, false);

    if(hit.emission == vec3(0.0) || (hit.t > dist - EPSILON && hit.t < dist + EPSILON)) return false;

    float weight = best.weight_sum / (best.target_pdf * float(best.count));
    vec3 emission = (bd.E == vec3(0.0) ? vec3(1.0) : bd.E);

    rd.direct = rd.absorbed * weight * emission * bd.C * hit.emission;
    rd.direct_pdf = weight;

    float target_pdf = luminance(hit.emission);
    if(depth == 0 || gbufferReservoir1.w == 0.0) {
        gbufferReservoir1 = vec4(best.position, target_pdf);
    }

    return true;
}

void computeGodrays(in HitData hit, in Ray ray) {
    AABB root = getBVHRootBounds();
	float size = length(root.maximum - root.minimum);

    if(size == 0.0) return;

	float r = randomFloatHQ();
	float density = 1.0 / size;
	float neg_inv_density = -1.0 / density;
	float rand_dist = hit.was_intersection ? hit.t * r : neg_inv_density * log(r);
    gbufferGodrays.w = -(hit.was_intersection ? hit.t : 1.0);

	ray.origin += ray.direction * rand_dist;
	ray.direction = normalize(atmosphere_direction + 0.00436 * getRandomOnSphere());

    vec3 color = getBackgroundColor(ray.direction);

    if(luminance(color) < LOW_EPSILON) return;

	if (!traverse(hit, ray, MAXIMUM_FLOAT, true)) {
        float normalization_factor = 4.0 * PI * realluminance(color);
        gbufferGodrays.rgb = color / normalization_factor;
        gbufferGodrays.w = rand_dist;
	}
}

void main() {
    //setupSTBN();
    setupHQRandomSeed();

    HitData hit;
    Ray ray = getRay(texCoords);

    RadianceData rd = RadianceData();
    handleNoHit();

    float rr_prob = 0.5;
    bool use_next_normals = false;

    vec3 prev_direct = rd.direct;
    float prev_direct_pdf = rd.direct_pdf;

    for(int i = 0; i < max_depth; i++) {
        traverse(hit, ray, MAXIMUM_FLOAT, false);
        BrdfData brdf_data = getBrdfData(hit, ray);

        if(i == 0) {
            //computeGodrays(hit, ray);
            setupGBuffers(hit);
        }

        vec3 absorbed = rd.absorbed;
        if(!hit.was_intersection) {
            rd.color += absorbed * getBackgroundColor(ray.direction);
            break;
        }

        if(use_next_normals && i == 1) gbufferNormal = hit.normal;
        updateRD(rd);

        //if(atmosphere_direction.y < -0.0698 && light_ids.length() < 1) sampleFlashlight(brdf_data, rd, i);
        //else sampleSun(brdf_data, rd, i);
        sampleSun(brdf_data, rd, i);
        //lightSampling(brdf_data, rd, i);
        scatterRay(brdf_data, ray, rd, i, use_next_normals);

        float mis_weight = prev_direct_pdf == 0.0 ? 1.0 : powerHeuristic(1.0 / rd.indirect_pdf, 1.0 / prev_direct_pdf);
        rd.color += mix(prev_direct, absorbed * rd.indirect, mis_weight);
        rd.absorbed *= mis_weight;

        prev_direct = rd.direct;
        prev_direct_pdf = rd.direct_pdf;

        if(i > 0) {
            if(randomFloatHQ() > rr_prob) break;
            else rd.absorbed /= rr_prob;
        }
    }

    gbufferDiffuse = (rd.color + prev_direct) / gbufferAlbedo;
}