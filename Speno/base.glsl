layout(location = 0) out vec4 gbuffer_diffuse;
layout(location = 1) out vec4 gbuffer_albedo;
layout(location = 2) out vec4 gbuffer_normal;
layout(location = 3) out vec4 gbuffer_position;
layout(location = 4) out vec4 gbuffer_velocity;
layout(location = 5) out vec4 gbuffer_restir;

uniform sampler2D previous_diffuse_texture;
uniform sampler2D previous_albedo_texture;
uniform sampler2D previous_normal_texture;
uniform sampler2D previous_position_texture;
uniform sampler2D previous_restir_texture;

uniform sampler2DArray stbn_scalar_texture;
uniform sampler2DArray stbn_vec1_texture;
uniform sampler2DArray stbn_vec2_texture;
uniform sampler2DArray stbn_vec3_texture;
uniform sampler2DArray stbn_unitvec1_texture;
uniform sampler2DArray stbn_unitvec2_texture;
uniform sampler2DArray stbn_unitvec3_texture;
uniform sampler2DArray stbn_unitvec3_cosine_texture;
uniform sampler2DArray stbn_unitvec3_hdri_texture;

uniform sampler2D hdri_texture;
uniform sampler2DArray voxel_textures;

uniform vec2 u_resolution;
uniform uint u_frame;
uniform uint u_samples;
uniform uint u_frame_seed;
uniform float u_time;
uniform float u_delta_time;

//#define USE_GRID

#define PI                 3.14159265
#define TAU                6.28318531
#define EPSILON            0.002

#define REPROJ_DELTA       0.1
#define MAXIMUM_DISTANCE   16777215.0
#define MAX_BVH_STACK_SIZE 23

#define STBN_SIZE   128
#define STBN_SIZE_Z 64

#define SKY_TYPE_BLACK     0
#define SKY_TYPE_DEFAULT   1
#define SKY_TYPE_REALISTIC 2
#define SKY_TYPE_HDRI      3

highp uint random_seed;

bool is_volume = false; // TODO: убрать эту хрень и наконец-то заменить на нормальные материалы
bool is_light = false;