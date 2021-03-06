#pragma once

#include "light.glsl"

// point light shadows do not work at the moment
float getPointShadow(in Light l, in vec3 worldPos, in vec3 lightDir)
{
    return 1.0f;
}

float getShadowPCF(in Light l, in vec3 worldPos, in vec3 worldNormal, in vec3 lightDir)
{
	//no shadow map available
	if(l.shadowMap.x == 0 && l.shadowMap.y == 0)
		return 1.0f;

    //transform position to light space
    vec4 worldPosLightSpace = l.lightSpaceMatrix * vec4(worldPos, 1.0f);
    worldPosLightSpace = worldPosLightSpace * 0.5f + 0.5f * worldPosLightSpace.w; // transform to [0,w] range  

    //calculate bias
    float cos_phi = max(dot(normalize(worldNormal), normalize(lightDir)), 0.0f);
    float bias = -0.00001f * tan(acos(cos_phi));

    worldPosLightSpace.z -= bias * worldPosLightSpace.w;

    float shadow = 0.0f;

    //gaussian stuff
    float twoSigmaSq = max(1.0f, l.pcfKernelSize) * 2.0f;
    float preFactor = 1.0f / (3.14159265f * twoSigmaSq);
    float kernelSum = 0.0f;

    sampler2DShadow sm = sampler2DShadow(l.shadowMap);
    vec2 texelSize = 1.0f / textureSize(sm, 0);
    int go = l.pcfKernelSize;
    for (int x = -go; x <= go; ++x)
    {
        for (int y = -go; y <= go; ++y)
        {
            vec4 tcOffset = vec4(vec2(x, y) * texelSize * worldPosLightSpace.w, 0.f, 0.f);
            float weight = preFactor * exp(-((x * x + y * y) / twoSigmaSq));
            shadow += weight * textureProj(sm, worldPosLightSpace + tcOffset);
            kernelSum += weight;
        }
    }
    shadow /= kernelSum;

    return shadow;
}

float getShadowBiased(in Light l, in vec3 worldPos, in vec3 worldNormal, in vec3 lightDir)
{
	//no shadow map available
	if(l.shadowMap.x == 0 && l.shadowMap.y == 0)
		return 1.0f;

    //transform position to light space
    vec4 worldPosLightSpace = l.lightSpaceMatrix * vec4(worldPos, 1.0f);
    worldPosLightSpace = worldPosLightSpace * 0.5f + 0.5f * worldPosLightSpace.w; // transform to [0,w] range   

    //calculate bias
    float cos_phi = max(dot(normalize(worldNormal), normalize(lightDir)), 0.0f);
    float bias = -0.00001f * tan(acos(cos_phi));

    worldPosLightSpace.z -= bias * worldPosLightSpace.w;

    float shadow = textureProj(sampler2DShadow(l.shadowMap), worldPosLightSpace);

    return shadow;
}

float calculateShadow(in Light l, in vec3 worldPos)
{
	//no shadow map available
	if(l.shadowMap.x == 0 && l.shadowMap.y == 0)
		return 1.0f;

    //transform position to light space
    vec4 worldPosLightSpace = l.lightSpaceMatrix * vec4(worldPos, 1.0f);
    worldPosLightSpace = worldPosLightSpace * 0.5f + 0.5f * worldPosLightSpace.w; // transform to [0,w] range   

    float shadow = textureProj(sampler2DShadow(l.shadowMap), worldPosLightSpace);

    return shadow;
}
