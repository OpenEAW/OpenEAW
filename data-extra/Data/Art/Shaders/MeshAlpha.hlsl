#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
    float3 Emissive;
    float4 Diffuse;
    float3 Specular;
};

Texture2D BaseTexture;
SamplerState BaseTextureSampler;

////////////////////////////////////////////////

struct VS_INPUT_MESH
{
    float3 Pos : ATTRIB0;
    float3 Normal : ATTRIB1;
    float2 UV : ATTRIB4;
};

struct VS_OUTPUT
{
    float4 Pos : SV_Position; // Position (camera space)
    float2 UV : TEXCOORD1; // Texture coordinates (tangent space)
    float4 Diffuse: COLOR0;
    float3 Spec: COLOR1;
};


VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    VS_OUTPUT Out;

    float4 world_pos = mul(float4(In.Pos, 1), World);
    float3 world_normal = normalize(mul(In.Normal, (float3x3)World));
    float3 camera_pos = camera_position();

    // No bump mapping, so calculate all lights per-vertex
    float3 diff_light =
        DirectionalLights[0].intensity * DirectionalLights[0].diffuse_color * saturate(dot(world_normal, -DirectionalLights[0].direction)) +
        DirectionalLights[1].intensity * DirectionalLights[1].diffuse_color * saturate(dot(world_normal, -DirectionalLights[1].direction)) +
        DirectionalLights[2].intensity * DirectionalLights[2].diffuse_color * saturate(dot(world_normal, -DirectionalLights[2].direction));

    float3 light_half_vec = light_half_angle((float3)world_pos, camera_pos, -DirectionalLights[0].direction);

    Out.Pos = mul(world_pos, ViewProj);
    Out.UV = In.UV;
    Out.Diffuse = float4(Diffuse.rgb * diff_light + Emissive, Diffuse.a);
    Out.Spec = Specular * DirectionalLights[0].intensity * DirectionalLights[0].specular_color * pow(max(0, dot(world_normal, light_half_vec)), 16);
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
    float4 base_texel = BaseTexture.Sample(BaseTextureSampler, In.UV);
    float4 diffuse = base_texel * In.Diffuse;
    return float4(diffuse.rgb * 2 + In.Spec.rgb, diffuse.a);
}
