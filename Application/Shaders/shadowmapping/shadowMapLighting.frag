#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;
layout (location = 4) in mat3 outT2W;
layout(location = 7) in struct 
{ 
    vec4  ShadowmapPos;
    vec4  VertColor;
    vec2  UV; 
} i;

layout (location = 0) out vec4 outFragColor;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;                                                                                          // stores the inverse view matrix 
  vec4 ambientLightColor; // w is intensity
  vec4 cameraEyePos;                                                                                     //position of the camera 
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D SamplerNormalMap;		                                     // [INPUT_TEXTURE_NORMAL]
layout(set = 0, binding = 2) uniform sampler2D SamplerDiffuseMap;		                                     // [INPUT_TEXTURE_DIFFUSE]
layout(set = 0, binding = 3) uniform sampler2D SamplerAOMap;			                                       // [INPUT_TEXTURE_AO]
layout(set = 0, binding = 4) uniform sampler2D SamplerRoughnessMap;	                                     // [INPUT_TEXTURE_ROUGHNESS]
layout(set = 0, binding = 5) uniform sampler2D SamplerShadowMap;	                                     // [INPUT_TEXTURE_ROUGHNESS]

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

//----------------------------------------------------------------------------------------

int isqr( int a ) { return a*a; }

//----------------------------------------------------------------------------------------

float SampleShadowTexture( in const vec4 Coord, in const vec2 off )
{
	float dist = texture( SamplerShadowMap, Coord.xy + off ).r;
	return ( Coord.w > 0.0 && dist < Coord.z ) ? 0.0f : 1.0f;
}

//----------------------------------------------------------------------------------------

float ShadowNoPCF( in const vec4 UVProjection )
{
	float Shadow = 1.0;	
	if ( UVProjection.z > -1.0 && UVProjection.z < 1.0 ) 
	{
		Shadow = SampleShadowTexture( UVProjection, vec2(0) );
	}
	return Shadow;
}

//----------------------------------------------------------------------------------------

float ShadowPCF( in const vec4 UVProjection )
{
	float Shadow = 1.0;
	if ( UVProjection.z > -1.0 && UVProjection.z < 1.0 ) 
	{
		const float scale			= 1.5;
		const vec2  TexelSize 		= scale / textureSize(SamplerShadowMap, 0);
		const int	SampleRange		= 1;
		const int	SampleTotal		= isqr(1 + 2 * SampleRange);
		
		float		ShadowAcc = 0;
		for (int x = -SampleRange; x <= SampleRange; x++)
		{
			for (int y = -SampleRange; y <= SampleRange; y++)
			{
				ShadowAcc += SampleShadowTexture( UVProjection, vec2(TexelSize.x*x, TexelSize.y*y));
			}
		}

		Shadow = ShadowAcc / SampleTotal;
	}

	return Shadow;
}

//----------------------------------------------------------------------------------------

void main() {

    const float Shadow		 = (true) ? ShadowPCF(i.ShadowmapPos / i.ShadowmapPos.w) : ShadowNoPCF(i.ShadowmapPos / i.ShadowmapPos.w);
	const vec3  FinalColor   = (ubo.ambientLightColor.xyz + Shadow * i.VertColor.rgb);


	vec3 Normal;                                                                                          // get the normal from a compress texture BC5
	Normal.xy	= (texture(SamplerNormalMap, fragTexCoord).gr * 2.0) - 1.0;// For BC5 it used (rg)
	
	// Derive the final element (all in Tangent space)
  // x^2 + y^2 + z^2 = 1
  // z^2             = 1 - x^2 - y^2
  // z               = sqrt( 1 - (x^2 + y^2) )
	Normal.z =  sqrt(1.0 - dot(Normal.xy, Normal.xy));

	Normal = normalize(outT2W * Normal);                                                                  // Transform the normal to from tangent space to world space
    Normal.y = -Normal.y;


	// Different techniques to do Lighting
  vec4 worldEyeSpacePos = ubo.cameraEyePos;

  // Load Textures
  const vec3  Albedo          = texture(SamplerDiffuseMap, fragTexCoord).rgb;
  const float Shininess       = mix( 1, 100, 1 - texture( SamplerRoughnessMap, fragTexCoord).r );       //80 preset 
  const vec3  SamplerAOColor  = texture(SamplerAOMap, fragTexCoord).rgb;

	// Different techniques to do Lighting
  vec3 TotalLight     = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

  // loop through the lights
  for (int i = 0; i < ubo.numLights; i++) 
  {
    PointLight light = ubo.pointLights[i];
    const vec3  directionToLight  = light.position.xyz - fragPosWorld;                                  // the direction to the light 
    const vec3  directionToLightN = normalize(directionToLight);                                        // the direction to the light normal 
    const float attenuation       = max(1.0 / dot(directionToLight, directionToLight),0.6);             // distance squared
    const float cosAngIncidence   = max(dot(Normal, directionToLightN), 0);                             // the angle of incidence         
    const vec3  intensity         = light.color.xyz * light.color.w * attenuation;                      // calculating the intensity of the light 

    //adding intensity & albedo color to the total light 
    TotalLight += intensity * cosAngIncidence;
    TotalLight *= Albedo.rgb;

    // Note This is the true Eye to Texel direction 
    const vec3  EyeDirection      = normalize( fragPosWorld - worldEyeSpacePos.xyz );

    // Another way to compute specular "BLINN-PHONG" (https://learnopengl.com/Advanced-Lighting/Advanced-Lighting)
    const float  SpecularI  = pow( max( 0, dot(Normal, normalize( directionToLightN - EyeDirection ))), Shininess );

    // Add the contribution of this light
    TotalLight.rgb += SpecularI.rrr * light.color.xyz * SamplerAOColor;
  }

	// Convert to gamma
	const float Gamma = worldEyeSpacePos.w;
	outFragColor.rgb = pow( FinalColor * TotalLight.rgb, vec3(1.0f/Gamma) );

}