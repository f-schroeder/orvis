#pragma once

struct RawMaterialData
{
	//uvec2 can be packed vec4 or bindless sampler2D
	uvec2 albedo; 
	uvec2 roughness;	
	uvec2 metallic;

	float ior;
	uint isTextureBitset; // texture if bit=1, color otherwise

	uvec2 normal;
	uvec2 ao;
};

layout(std430, binding = MATERIALS_BINDING) readonly buffer MaterialBuffer
{
    RawMaterialData materials[];
};

struct Material
{
	vec4 albedo; 
	float roughness;
	float metallic;
	float ior;

	vec4 normal;    
	float ao;	
};

Material getMaterial(in uint materialIndex, in vec2 uv)
{
    RawMaterialData rawMat = materials[materialIndex];
    Material mat;

	mat.albedo = bitfieldExtract(rawMat.isTextureBitset, 0, 1) == 1 ? texture(sampler2D(rawMat.albedo), uv) : vec4(unpackHalf2x16(rawMat.albedo.x), unpackHalf2x16(rawMat.albedo.y));
	mat.roughness = bitfieldExtract(rawMat.isTextureBitset, 1, 1) == 1 ? texture(sampler2D(rawMat.roughness), uv).x : uintBitsToFloat(rawMat.roughness.x);
	mat.metallic = bitfieldExtract(rawMat.isTextureBitset, 2, 1) == 1 ? texture(sampler2D(rawMat.metallic), uv).x : uintBitsToFloat(rawMat.metallic.x);

	mat.normal = bitfieldExtract(rawMat.isTextureBitset, 3, 1) == 1 ? texture(sampler2D(rawMat.normal), uv) : vec4(-1.0f);	
	mat.ao = bitfieldExtract(rawMat.isTextureBitset, 4, 1) == 1 ? texture(sampler2D(rawMat.ao), uv).x : 1.0f;
	
	mat.ior = rawMat.ior;

	return mat;
}
