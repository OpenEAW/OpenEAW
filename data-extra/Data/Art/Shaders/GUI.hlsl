#include "common.hlsli"

Texture2D BaseTexture;

////////////////////////////////////////////////

SamplerState LinearClampSampler;

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
    float2 UV : TEXCOORD1; // Texture coordinates
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = float4(In.Pos, 1);
    Out.UV = In.UV;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
    return BaseTexture.Sample(LinearClampSampler, In.UV);
}