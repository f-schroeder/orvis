#pragma once

#include "light.glsl"
#include "material.glsl"
#include "shadowMapping.glsl"

#ifndef PI
#define PI 3.14159265359f
#endif //PI

layout(binding = SKYBOX_BINDING) uniform samplerCube cubeTexture;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r*r) / 8.0f;

    float num   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec4 getPBRColor(in uint materialIndex, in vec3 worldPos, in vec3 normal, in vec3 viewDir, in vec2 uv)
{
	Material mat = getMaterial(materialIndex, uv);

    vec3 F0 = vec3(0.04f); 
    F0 = mix(F0, mat.albedo.xyz, mat.metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0f);
    for(int i = 0; i < lights.length(); ++i) 
    {
		Light l = lights[i];

        // calculate per-light radiance
        vec3 L = getLightDirection(l, worldPos);
        vec3 H = normalize(viewDir + L);       
        
        // cook-torrance brdf
        float NDF = DistributionGGX(normal, H, mat.roughness);        
        float G   = GeometrySmith(normal, viewDir, L, mat.roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0f), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - mat.metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, L), 0.0f);
        vec3 specular     = numerator / max(denominator, 0.001f);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(normal, L), 0.0f);                
        Lo += (kD * mat.albedo.xyz / PI + specular) * getLightRadiance(l, worldPos) * NdotL * getShadowPCF(l, worldPos, normal, L); 
    }   
  
    vec3 ambient = getAmbientLight() * mat.albedo.xyz * mat.ao;
    vec3 color = ambient + Lo;
	
	// tone mapping
	color = vec3(1.0f) - exp(-color * camera.exposure);

	// gamma
    color = pow(color, vec3(1.0f/camera.gamma));

	return vec4(color, mat.albedo.a);
}
