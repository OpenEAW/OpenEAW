#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
    float DistortionScale;
    float SFreq;
    float TFreq;
    float2 UVScrollRate;
};

Texture2D BaseTexture;
SamplerState BaseTextureSampler;

////////////////////////////////////////////////

struct VS_INPUT_MESH
{
    float3 Pos : ATTRIB0;
    float3 Normal : ATTRIB1;
    float2 UV : ATTRIB4;
    float3 Color : ATTRIB5;
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

    float3 world_pos = mul(float4(In.Pos,1), World).xyz;

    float SPATIAL_FREQ = SFreq;
    float TIME_FREQ = TFreq;
    float SCALE = DistortionScale;
    float Time = 0;

    float3 offset = float3(
        sin(2*PI * frac(SPATIAL_FREQ * world_pos.x + TIME_FREQ * Time) ),
        sin(2*PI * frac(SPATIAL_FREQ * world_pos.y + TIME_FREQ * Time) ),
        sin(2*PI * frac(SPATIAL_FREQ * world_pos.z + TIME_FREQ * Time) ));

    float3 obj_pos = In.Pos + SCALE * offset;

    // Edge fade
    float3 view_normal = normalize(mul(In.Normal, (float3x3)mul(World, View)));
    float scale = offset.x;
    float edge = 0.9 * view_normal.z + 0.1 * scale;
    float edge_fade = pow(view_normal.z, 8) * 2 + 0.1;

    Out.Pos     = mul(float4(obj_pos, 1), mul(World, ViewProj));
    Out.UV      = In.UV + Time * UVScrollRate * 3;
    Out.Diffuse = float4(In.Color.rgb, 1) * edge_fade;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
    float4 texel = BaseTexture.Sample(BaseTextureSampler, In.UV);
    return texel * In.Diffuse;
}
