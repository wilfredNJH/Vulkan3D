#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 bitangent;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec3 color;

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

//point light structure 
struct PointLight {
  vec4 position;                          // ignore w
  vec4 color;                             // w is intensity
};

//global ubo 
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;                        // projection matrix 
  mat4 view;                              // view matrix 
  vec4 ambientLightColor;                 // w is intensity
  vec4 cameraEyePos;                      // position of the camera 
  PointLight pointLights[10];             // point light data 
  int numLights;                          // number of lights 
} ubo;

//push constant 
layout(push_constant) uniform Push {
  vec4 position;                          // position of the light 
  vec4 color;                             // color of the light 
  float radius;                           // radius of light 
} push;


void main() {
  fragOffset = OFFSETS[gl_VertexIndex];                                     
  vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};  // getting camera's right world vector 
  vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};     // getting camera's up world vector 

  //getting the world position
  vec3 positionWorld = push.position.xyz
    + push.radius * fragOffset.x * cameraRightWorld
    + push.radius * fragOffset.y * cameraUpWorld;

  gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);//setting the position of the point light 
}