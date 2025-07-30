#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
	float3 Colorization;
	float3 Diffuse;
};

Texture2D BaseTexture;
SamplerState BaseTextureSampler;

Texture2D NormalTexture;
SamplerState NormalTextureSampler;

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
	float2 UV : TEXCOORD0; // Texture coordinates (tangent space)
	float3 LightDir : TEXCOORD1; // Light direction (tangent space)
	float3 Diff : COLOR0; // Per-vertex diffuse color
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    VS_OUTPUT Out;
   	Out.Pos = mul(float4(In.Pos, 1), mul(World, ViewProj));

	// Store the light direction in tangent space
	float3x3 local_to_tangent = to_tangent_matrix(In.Tangent,In.Binormal,In.Normal);
	float3x3 world_to_tangent = mul((float3x3)WorldInv, local_to_tangent);

	float3 light0_dir = normalize(mul(-DirectionalLights[0].direction, world_to_tangent));
	float3 light1_dir = normalize(mul(-DirectionalLights[1].direction, world_to_tangent));
	float3 light2_dir = normalize(mul(-DirectionalLights[2].direction, world_to_tangent));

	// Main light is calculated in pixel shader; pass light vector. Other lights are calculated per-vertex.
	// Note that the light directions are in tangent space, so dot(N,L) is just L.z, since the normal vector
	// is always 0,0,1 in tangent space.
	Out.LightDir = encode_vector(light0_dir);
	Out.Diff = DirectionalLights[1].intensity * DirectionalLights[1].diffuse_color * saturate(light1_dir.z) +
	           DirectionalLights[2].intensity * DirectionalLights[2].diffuse_color * saturate(light2_dir.z);

	Out.UV = In.UV;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
	// Normal vector and light direction (both in tangent space)
	float3 normal = decode_vector(NormalTexture.Sample(NormalTextureSampler, In.UV).rgb);

	// Calculate main light in pixel shader to use normal map
	float3 light_dir = decode_vector(In.LightDir);
	float3 diffuse = In.Diff + DirectionalLights[0].intensity * DirectionalLights[0].diffuse_color * saturate(dot(normal, light_dir));

	// Texel alpha contains colorization channel
	float4 texel = BaseTexture.Sample(BaseTextureSampler, In.UV);
	float3 tex_color = lerp(texel.rgb, Colorization * texel.rgb, texel.a);

	return float4(tex_color * Diffuse * diffuse, 1);
}