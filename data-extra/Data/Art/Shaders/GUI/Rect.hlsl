#include "common.hlsli"

////////////////////////////////////////////////

struct VS_INPUT_MESH
{
    float3 Pos : ATTRIB0;
    float4 Color : ATTRIB5;
};

struct VS_OUTPUT
{
    float4 Pos : SV_Position; // Position (camera space)
    float4 Color : COLOR0;
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;
    Out.Pos = float4(In.Pos, 1);
    Out.Color = In.Color;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
    return float4(0,0,1,0.3);//In.Color;
}