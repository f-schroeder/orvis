#pragma once

#include "light.glsl"
#include "material.glsl"

#ifndef PI
#define PI 3.14159265359f
#endif //PI

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec4 getPBRColor(in int materialIndex, in vec3 worldPos, in vec3 normal, in vec3 viewDir)
{
	Material mat = getMaterial(materialIndex, uv);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, mat.albedo, mat.metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lights.length(); ++i) 
    {
		Light l = lights[i];

        // calculate per-light radiance
        vec3 L = getLightDirection(l, worldPos);
        vec3 H = normalize(viewDir + L);       
        
        // cook-torrance brdf
        float NDF = DistributionGGX(normal, H, mat.roughness);        
        float G   = GeometrySmith(normal, viewDir, L, mat.roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - mat.metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(normal, L), 0.0);                
        Lo += (kD * mat.albedo / PI + specular) * getLightRadiance(l, worldPos) * NdotL; 
    }   
  
    vec3 ambient = getAmbientLight() * mat.albedo * mat.ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

	return vec4(color, mat.albedo.a);
}
