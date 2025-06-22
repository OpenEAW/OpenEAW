cbuffer InstanceConstants
{
	float4x4 World;
	float4x4 WorldInv;
};

cbuffer ViewConstants
{
	float4x4 ViewProj;
};

////////////////////////////////////////////////
// Material
cbuffer Material
{
	float3 Color;
	float2 UVScrollRate;
};

Texture2D BaseTexture;
SamplerState BaseTextureSampler;

////////////////////////////////////////////////

struct VS_INPUT_MESH
{
	float3 Pos : ATTRIB0;
	float2 UV : ATTRIB4;
};

struct VS_OUTPUT
{
	float4 Pos : SV_Position; // Position (camera space)
	float2 UV : TEXCOORD1; // Texture coordinates (tangent space)
	float4 Diffuse: COLOR0;
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
	VS_OUTPUT Out;

	Out.Pos = mul(float4(In.Pos, 1), mul(World, ViewProj));
	Out.UV = In.UV;
	Out.Diffuse = float4(Color, 1);

	return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
	float4 texel = BaseTexture.Sample(BaseTextureSampler, In.UV);
	return texel * In.Diffuse;
}