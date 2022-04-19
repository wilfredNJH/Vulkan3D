#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;    // ignores W 
  vec4 color;       // W is the intensity 
};

//global ubo 
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;                    // stores the projection matrix 
  mat4 view;                          // stores the view matrix 
  vec4 ambientLightColor;             // w is intensity
  vec4 cameraEyePos;                  // position of the camera 
  PointLight pointLights[10];         // stores all the point light data 
  int numLights;                      // the number of lights 
} ubo;

//push constant 
layout(push_constant) uniform Push {
  vec4 position;                      // stores the position of light push constant 
  vec4 color;                         // stores the color of light push constant 
  float radius;                       // stores the radius of light push constant 
} push;

void main() {
  float dis = sqrt(dot(fragOffset, fragOffset));
  if (dis >= 1.0) {
    discard;
  }
  outColor = vec4(push.color.xyz, 1.0);
}