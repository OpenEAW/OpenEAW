#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
    float4 Color;
};

struct VS_INPUT_MESH
{
    float3 Pos : ATTRIB0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_Position; // Position (camera space)
    float4 Diffuse: COLOR0;
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    VS_OUTPUT Out;
    Out.Pos  = mul(float4(In.Pos, 1), mul(World, ViewProj));
    Out.Diffuse = Color;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
    return In.Diffuse;
}
