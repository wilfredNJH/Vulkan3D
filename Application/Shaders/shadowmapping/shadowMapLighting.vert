#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 bitangent;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec3 color;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out mat3 outT2W;
layout(location = 7) out struct 
{ 
    vec4  ShadowmapPos;
    vec4  VertColor;
    vec2  UV; 
} o;


struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;                       // stores the inverse view matrix 
  vec4 ambientLightColor; // w is intensity
  vec4 cameraEyePos;//position of the camera 
  PointLight pointLights[10];
  int numLights;
  mat4 ShadowmapL2C;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D SamplerNormalMap;		// [INPUT_TEXTURE_NORMAL]
layout(set = 0, binding = 2) uniform sampler2D SamplerDiffuseMap;		// [INPUT_TEXTURE_DIFFUSE]
layout(set = 0, binding = 3) uniform sampler2D SamplerAOMap;			  // [INPUT_TEXTURE_AO]
layout(set = 0, binding = 4) uniform sampler2D SamplerRoughnessMap;	// [INPUT_TEXTURE_ROUGHNESS]

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  const float Gamma = ubo.view[3][3];

  // Decompress the binormal
  vec3 BiTangent               = normalize(cross(tangent, normal));

  // shadow information 
  o.UV                = uv;
  o.VertColor.rgb     = vec3(ubo.pointLights[0].color) * max( 0, dot( normal.xyz, normalize(vec3(ubo.pointLights[0].position) - position).xyz ));
  o.VertColor.a       = 1;
  o.ShadowmapPos      = ubo.projection * ubo.view * vec4(position.xyz, 1.0);

  // Compute lighting information
  outT2W                  = mat3(push.modelMatrix) * mat3(tangent, BiTangent, normal);
  fragColor               = pow( vec4(color,1.0), Gamma.rrrr );    // SRGB to RGB
  fragTexCoord            = uv;
  vec4 positionWorld      = push.modelMatrix * vec4(position, 1.0);
  fragPosWorld            = positionWorld.xyz;
  gl_Position             = ubo.projection * ubo.view * positionWorld;
}