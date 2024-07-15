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
	float3 Colorization;
	float3 Diffuse;
};

Texture2D BaseTexture;
Texture2D NormalTexture;

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
	float3 LightDir : TEXCOORD2; // Light direction (tangent space)
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
	float3 Light0_Dir = normalize(float3(-1, 0, 0));

    VS_OUTPUT Out = (VS_OUTPUT)0;
   	Out.Pos = mul(float4(In.Pos, 1), mul(World, ViewProj));

	// Store the light direction in tangent space
	float3x3 to_tangent = to_tangent_matrix(In.Tangent,In.Binormal,In.Normal);
	Out.LightDir = encode_vector(normalize(mul(Light0_Dir, to_tangent)));

	Out.UV = In.UV;
    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
	float3 Light0_Color = {1, 0, 0};
	float3 Light_Ambient = {0.1, 0.1, 0.1};

	// Normal vector and light direction (both in tangent space)
	float3 normal = decode_vector(NormalTexture.Sample(LinearSampler, In.UV)).rgb;
	float3 light_dir = decode_vector(In.LightDir).rgb;

	float3 diffuse = Diffuse * Light0_Color * saturate(dot(normal, -light_dir));

	// Texel alpha contains colorization channel
	float4 texel = BaseTexture.Sample(LinearSampler, In.UV);
	float3 tex_color = lerp(texel.rgb, Colorization * texel.rgb, texel.a);

	return float4(tex_color * (Light_Ambient + diffuse), 1);
}