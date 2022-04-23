#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;
layout (location = 4) in mat3 outT2W;

layout (location = 0) out vec4 outFragColor;

struct PointLight {
  vec4 position;                                                                                     // ignore w
  vec4 color;                                                                                        // w is intensity
};

//global ubo 
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;                                                                                   // projection matrix                                                               
  mat4 view;                                                                                         // view matrix   
  mat4 invView;                       // stores the inverse view matrix     
  vec4 ambientLightColor;                                                                            // w is intensity                              
  vec4 cameraEyePos;                                                                                 // position of the camera               
  PointLight pointLights[10];                                                                        // point light data                        
  int numLights;                                                                                     // number of lights           
} ubo;                                                                                               
  
layout(set = 0, binding = 5) uniform sampler2D SamplerNormalMap;		                                 // [INPUT_TEXTURE_NORMAL]
layout(set = 0, binding = 6) uniform sampler2D SamplerDiffuseMap;		                                 // [INPUT_TEXTURE_DIFFUSE]
layout(set = 0, binding = 7) uniform sampler2D SamplerAOMap;			                                   // [INPUT_TEXTURE_AO]
layout(set = 0, binding = 8) uniform sampler2D SamplerRoughnessMap;	                                 // [INPUT_TEXTURE_ROUGHNESS]
  
layout(push_constant) uniform Push {      
  mat4 modelMatrix;     
  mat4 normalMatrix;      
} push;     

void main() {     

	vec3 Normal;      
	Normal.xy	= (texture(SamplerNormalMap, fragTexCoord).gr * 2.0) - 1.0;                             // For BC5 it used (rg)

	// Derive the final element (all in Tangent space)      
  // x^2 + y^2 + z^2 = 1      
  // z^2             = 1 - x^2 - y^2      
  // z               = sqrt( 1 - (x^2 + y^2) )      
	Normal.z =  sqrt(1.0 - dot(Normal.xy, Normal.xy));      
	Normal = normalize(outT2W * Normal);                                                              // Transform the normal to from tangent space to world space
  Normal.y = -Normal.y;                                                                             //inverting the normal

	// Different techniques to do Lighting
  vec4 worldEyeSpacePos = ubo.cameraEyePos;

  // Load Textures
  const vec3  Albedo          = texture(SamplerDiffuseMap, fragTexCoord).rgb;
  const float Shininess       = mix( 1, 100, 1 - texture( SamplerRoughnessMap, fragTexCoord).r );   //80 preset 
  const vec3  SamplerAOColor  = texture(SamplerAOMap, fragTexCoord).rgb;

	// Different techniques to do Lighting
  vec3 TotalLight     = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

  //loop through all the lights
  for (int i = 0; i < ubo.numLights; i++) 
  {
    PointLight light = ubo.pointLights[i];                                                          //getting the point light 
    const vec3  directionToLight      = light.position.xyz - fragPosWorld;                          //calculating the direction of the light 
    const vec3  directionToLightN     = normalize(directionToLight);                                //direction to light normalized 
    const float attenuation           = max(1.0 / dot(directionToLight, directionToLight),0.6);     // distance squared
    const float cosAngIncidence       = max(dot(Normal, directionToLightN), 0);                     //getting the angle of incidence 
    const vec3  intensity             = light.color.xyz * light.color.w * attenuation;              //getting the intensity 

    //adding the intensity & albedo color to the total light 
    TotalLight += intensity * cosAngIncidence;
    TotalLight *= Albedo.rgb;

    const vec3  EyeDirection      = normalize( fragPosWorld - worldEyeSpacePos.xyz );               // Note This is the true Eye to Texel direction 

    // Another way to compute specular "BLINN-PHONG" (https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
    const float  SpecularI  = pow( max( 0, dot(Normal, normalize( directionToLightN - EyeDirection ))), Shininess );

    // Add the contribution of this light
    TotalLight.rgb += SpecularI.rrr * light.color.xyz * SamplerAOColor;
  }

	// Convert to gamma
	const float Gamma = worldEyeSpacePos.w;
	outFragColor.rgb = pow( TotalLight.rgb, vec3(1.0f/Gamma) );

}