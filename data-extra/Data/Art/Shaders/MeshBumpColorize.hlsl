#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
	float3 Emissive;
	float3 Diffuse;
	float3 Specular;
	float3 Colorization;
	float2 UVOffset;
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
	float3 HalfAngleVector: TEXCOORD2; // Half Angle light vector (tangent space)
	float3 Diff : COLOR0; // Per-vertex diffuse color
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    float4 world_pos = mul(float4(In.Pos, 1), World);
    float3 camera_pos = camera_position();

    VS_OUTPUT Out;
    Out.Pos = mul(world_pos, ViewProj);

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
	Out.Diff = Diffuse.rgb *
	           (DirectionalLights[1].intensity * DirectionalLights[1].diffuse_color * saturate(light1_dir.z) +
	            DirectionalLights[2].intensity * DirectionalLights[2].diffuse_color * saturate(light2_dir.z)) +
			   Emissive;

	Out.HalfAngleVector = encode_vector(normalize(mul(light_half_angle((float3)world_pos, camera_pos, -DirectionalLights[0].direction), world_to_tangent)));

	Out.UV = In.UV + UVOffset;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
	float4 base_texel = BaseTexture.Sample(BaseTextureSampler, In.UV);
	float4 normal_texel = NormalTexture.Sample(NormalTextureSampler, In.UV);

	// base_texel alpha contains colorization channel
	float3 surface_color = lerp(base_texel.rgb, Colorization * base_texel.rgb, base_texel.a);

	// Normal vector and light direction (both in tangent space)
	float3 normal = decode_vector(normal_texel.rgb);
	float3 light_dir = decode_vector(In.LightDir);
	float3 half_vec = decode_vector(In.HalfAngleVector);

	// Calculate main light in pixel shader to use normal map
	float3 diffuse = surface_color * (In.Diff + DirectionalLights[0].intensity * DirectionalLights[0].diffuse_color * saturate(dot(normal, light_dir)));
	float3 spec = normal_texel.a * Specular * DirectionalLights[0].intensity * DirectionalLights[0].specular_color * pow(saturate(dot(normal, half_vec)), 16);
	return float4(diffuse + spec, 1);
}