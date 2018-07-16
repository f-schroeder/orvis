#pragma once

struct Light
{
    vec3 color;            // all
    float cutOff;          // spot
    vec3 position;         // spot, point    
    int pcfKernelSize;     // all (used for SM filtering)
    vec3 direction;        // dir, spot
	int type;

	//shadow mapping stuff
	mat4 lightSpaceMatrix;
	uvec2 shadowMap; // can be sampler2DShadow or samplerCubeShadow

	float pad1, pad2; //padding (not used)
};

layout(std430, binding = LIGHTS_BINDING) readonly buffer LightBuffer
{
    Light lights[];
};

#ifndef AMBIENT_LIGHT
#define AMBIENT_LIGHT vec3(0.01f);
#endif //AMBIENT_LIGHT

vec3 getAmbientLight()
{
	return AMBIENT_LIGHT;
}

vec3 getLightDirection(in Light l, in vec3 worldPos)
{
	return l.type == 0 ? normalize(-l.direction) : normalize(l.position - worldPos);
}

vec3 getLightRadiance(in Light l, in vec3 worldPos)
{
	if (l.type == 0) // D I R E C T I O N A L
    {		
        return l.color;
    }
    if (l.type == 1) // P O I N T
    {
		//distance attenuation
        float dist    = distance(l.position, worldPos);
        float attenuation = 1.0f / (dist * dist);
        return l.color * attenuation;
    }
    if (l.type == 2) // S P O T
    {
		//distance attenuation
        float dist    = distance(l.position, worldPos);
        float attenuation = 1.0f / (dist * dist);

		//cutoff attenuation
		vec3 lightDir	= normalize(l.position - worldPos);
		float theta     = dot(lightDir, normalize(-l.direction));
		float epsilon   = 0.1f; //light.cutOff - light.outerCutOff;
		float intensity = clamp((theta - (l.cutOff - epsilon)) / epsilon, 0.0f, 1.0f); 

		return l.color * attenuation * intensity;
    }
}
