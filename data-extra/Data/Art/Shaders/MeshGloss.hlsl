#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
	float3 Emissive;
	float3 Diffuse;
	float3 Specular;
	float  Shininess;
};

Texture2D BaseTexture;
SamplerState BaseTextureSampler;

////////////////////////////////////////////////

struct VS_INPUT_MESH
{
	float3 Pos : ATTRIB0;
	float3 Normal : ATTRIB1;
	float3 Tangent : ATTRIB2;
	float3 Binormal : ATTRIB3;
	float2 UV : ATTRIB4;
};

struct VS_OUTPUT
{
	float4 Pos : SV_Position; // Position (camera space)
	float2 UV : TEXCOORD1; // Texture coordinates (tangent space)
	float3 Diffuse: COLOR0;
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
	VS_OUTPUT Out;

	float3 world_normal = normalize(mul(In.Normal, (float3x3)World));

	// No bump mapping, so calculate all lights per-vertex
	float3 diff_light =
		DirectionalLights[0].intensity * DirectionalLights[0].diffuse_color * saturate(dot(world_normal, -DirectionalLights[0].direction)) +
	    DirectionalLights[1].intensity * DirectionalLights[1].diffuse_color * saturate(dot(world_normal, -DirectionalLights[1].direction)) +
	    DirectionalLights[2].intensity * DirectionalLights[2].diffuse_color * saturate(dot(world_normal, -DirectionalLights[2].direction));

	Out.Pos = mul(float4(In.Pos, 1), mul(World, ViewProj));
	Out.UV = In.UV;
	Out.Diffuse = Diffuse * diff_light + Emissive;

	return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
	float4 texel = BaseTexture.Sample(BaseTextureSampler, In.UV);
	return float4(texel.rgb * In.Diffuse, 1);
}