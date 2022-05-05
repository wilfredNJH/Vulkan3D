#version 450

layout(location = 0) in vec3 position;

//point light structure 
struct PointLight {
  vec4 position;                          // ignore w
  vec4 color;                             // w is intensity
};

//global ubo 
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;                        // projection matrix 
  mat4 view;                              // view matrix 
  mat4 invView;                       // stores the inverse view matrix 
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

void main() 
{
    gl_Position = ubo.projection * ubo.view * vec4(position.xyz, 1.0);
}
