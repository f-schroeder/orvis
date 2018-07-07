#pragma once

struct RawMaterialData
{
	//uvec2 can be packed vec4 or bindless sampler2D
	uvec2 albedo; 
	uvec2 normal;
	uvec2 metallic;
    uvec2 roughness;
	uvec2 ao;
	float ior;
	uint isTextureBitset; // texture if bit=1, color otherwise
};

layout(std430, binding = MATERIALS_BINDING) readonly buffer MaterialBuffer
{
    RawMaterialData materials[];
};

layout(std430, binding = MATERIAL_INDICES_BINDING) readonly buffer MaterialIndexBuffer
{
    uint materialIndices[];
};

struct Material
{
	vec4 albedo; 
	vec4 normal;
	float metallic;
    float roughness;
	float ao;
	float ior;
};

Material getMaterial(in int materialIndex, in vec2 uv)
{
    RawMaterialData rawMat = materials[materialIndices[materialIndex]];
    Material mat;

	mat.albedo = bitfieldExtract(rawMat.isTextureBitset, 0, 1) ? texture(sampler2D(rawMat.albedo), uv) : vec4(unpackHalf2x16(rawMat.albedo.x), unpackHalf2x16(rawMat.albedo.y));
	mat.normal = bitfieldExtract(rawMat.isTextureBitset, 1, 1) ? texture(sampler2D(rawMat.normal), uv) : vec4(-1.0f);
	mat.metallic = bitfieldExtract(rawMat.isTextureBitset, 2, 1) ? texture(sampler2D(rawMat.metallic), uv).x : unpackHalf2x16(rawMat.metallic.x).x;
	mat.roughness = bitfieldExtract(rawMat.isTextureBitset, 3, 1) ? texture(sampler2D(rawMat.roughness), uv).x : unpackHalf2x16(rawMat.roughness.x).x;
	mat.ao = bitfieldExtract(rawMat.isTextureBitset, 4, 1) ? texture(sampler2D(rawMat.ao), uv).x : unpackHalf2x16(rawMat.ao.x).x;
	mat.ior = rawMat.ior;

	return mat;
}
