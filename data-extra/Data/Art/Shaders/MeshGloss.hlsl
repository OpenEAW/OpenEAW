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
	float3 Emissive;
	float3 Diffuse;
	float3 Specular;
	float  Shininess;
};

Texture2D BaseTexture;

////////////////////////////////////////////////

SamplerState LinearSampler;

////////////////////////////////////////////////

// Encodes a (assumed normalized) vector for storage in a TEXCOORD attribute
float3 encode_vector(float3 vec)
{
	return 0.5 * vec + 0.5;
}

float4 encode_vector(float4 vec)
{
	return 0.5 * vec + 0.5;
}

// Decodes a TEXCOORD attribute into a (assumed normalized) vector
float3 decode_vector(float3 texcoord)
{
	return 2 * (texcoord - 0.5);
}

float4 decode_vector(float4 texcoord)
{
	return 2 * (texcoord - 0.5);
}

// Returns a matrix that transforms vectors from the tangent space defined by the arguments to object space
float3x3 from_tangent_matrix(float3 tangent, float3 binormal, float3 normal)
{
	float3x3 tm;
	tm[0] = tangent;
	tm[1] = binormal;
	tm[2] = normal;
	return tm;
}

// Returns a matrix that transforms vectors object space to the tangent space defined by the arguments
float3x3 to_tangent_matrix(float3 tangent, float3 binormal, float3 normal)
{
	// The from-tangent-space matrix can assumed to be orthogonal.
	// The inverse of a orthogonal matrix is the same as its transpose.
	return transpose(from_tangent_matrix(tangent,binormal,normal));
}

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
	VS_OUTPUT Out = (VS_OUTPUT)0;

	Out.Pos = mul(float4(In.Pos, 1), mul(World, ViewProj));
	Out.UV = In.UV;
	Out.Diffuse = Diffuse + Emissive;

	return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
	float4 texel = BaseTexture.Sample(LinearSampler, In.UV);
	float3 diffuse = texel.rgb * In.Diffuse.rgb * 2;
	return float4(diffuse, 1);
}