#include "common.hlsli"

////////////////////////////////////////////////
// Material
cbuffer Material
{
    float3 Emissive;
    float3 Diffuse;
    float3 Specular;
    float4 Atmosphere;
    float3 CityColor;
    float AtmospherePower;
    float CloudScrollRate;
};

Texture2D BaseTexture;
SamplerState BaseTextureSampler;

Texture2D NormalTexture;
SamplerState NormalTextureSampler;

Texture2D CloudTexture;
SamplerState CloudTextureSampler;

Texture2D CloudNormalTexture;
SamplerState CloudNormalTextureSampler;


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
    float4  Pos : SV_Position;            // Position (camera space)
    float3  Diff : COLOR0;
    float2  Tex0 : TEXCOORD0;             // Base + normal texture coordinates (tangent space)
    float2  Tex1 : TEXCOORD2;             // Clouds texture coordinates (tangent space)
    float3  LightDir : TEXCOORD3;         // Light direction (tangent space)
    float3  HalfAngleVector: TEXCOORD4;   // Half Angle light vector (tangent space)
    float   AtmosphereFactor : TEXCOORD5; // X = atmosphere (edge/rim factor)
};

VS_OUTPUT vs_main(VS_INPUT_MESH In)
{
    float Time = 0;

    float4 world_pos = mul(float4(In.Pos, 1), World);
    float3 camera_pos = camera_position();

    VS_OUTPUT Out;
    Out.Pos = mul(world_pos, ViewProj);

    // Base and normal texture coords
    Out.Tex0 = In.UV;

    // Cloud texture coords: scroll along the u axis
    Out.Tex1 = In.UV;
    Out.Tex1.x += Time * CloudScrollRate;

    // Compute the tangent-space light vector and half-angle vector for per-pixel lighting
    // Note that we are doing everything in object space here.
    float3x3 local_to_tangent = to_tangent_matrix(In.Tangent, In.Binormal, In.Normal);
    float3x3 world_to_tangent = mul((float3x3)WorldInv, local_to_tangent);

    float3 light0_dir = normalize(mul(-DirectionalLights[0].direction, world_to_tangent));
    float3 light1_dir = normalize(mul(-DirectionalLights[1].direction, world_to_tangent));
    float3 light2_dir = normalize(mul(-DirectionalLights[2].direction, world_to_tangent));

    // Main light is calculated in pixel shader; pass light vector. Other lights are calculated per-vertex.
    // Note that the light directions are in tangent space, so dot(N,L) is just L.z, since the normal vector
    // is always 0,0,1 in tangent space.
    Out.LightDir = encode_vector(light0_dir);
    Out.Diff = Diffuse *
               (DirectionalLights[1].intensity * DirectionalLights[1].diffuse_color * saturate(light1_dir.z) +
                DirectionalLights[2].intensity * DirectionalLights[2].diffuse_color * saturate(light2_dir.z)) +
               Emissive;

    Out.HalfAngleVector = encode_vector(normalize(mul(light_half_angle(world_pos.xyz, camera_pos, -DirectionalLights[0].direction), world_to_tangent)));

    // Compute the atmospheric fogging factor
    float3 world_eye_vec = normalize(camera_pos - world_pos.xyz);
    float3 world_normal = normalize(mul(In.Normal, (float3x3)World));
    float vdotn = saturate(dot(world_normal, world_eye_vec));
    Out.AtmosphereFactor = pow(1 - vdotn, AtmospherePower) * 2 * Atmosphere.a;

    return Out;
}

float4 ps_main(VS_OUTPUT In) : SV_Target
{
    float4 base_texel = BaseTexture.Sample(BaseTextureSampler, In.Tex0);
    float4 normal_texel = NormalTexture.Sample(NormalTextureSampler, In.Tex0);
    float4 cloud_texel = CloudTexture.Sample(CloudTextureSampler, In.Tex1);
    float4 cloud_normal_texel = CloudNormalTexture.Sample(CloudNormalTextureSampler, In.Tex1);

    float3 normal = decode_vector(normal_texel.rgb);
    float3 cloud_normal = decode_vector(cloud_normal_texel.rgb);
    float3 light_dir = decode_vector(In.LightDir);
    float3 half_vec = decode_vector(In.HalfAngleVector);

    // planet color depends on the base texture and the atmosphere
    float3 planet_color = base_texel.rgb;
    float3 surface_color = planet_color + Atmosphere.rgb * In.AtmosphereFactor;

    // Compute Lighting
    float3 diffuse = surface_color * (In.Diff + Diffuse * DirectionalLights[0].intensity * DirectionalLights[0].diffuse_color * saturate(dot(normal, light_dir))) * 2;
    float3 cloud_diffuse = (In.Diff + cloud_texel.rgb * DirectionalLights[0].intensity * DirectionalLights[0].diffuse_color * saturate(dot(cloud_normal, light_dir))) * 2;

    // Specular lights. Normal.a indicates water.
    float3 spec = Specular * normal_texel.a * DirectionalLights[0].intensity * DirectionalLights[0].specular_color * pow(saturate(dot(normal, half_vec)), 16);

    // Add in city lights. base_texel.a indicates where cities are.
    // When the light points away from the surface normal, it's in darkness, so we want full city lights
    // Scale by 32 for a hard transition where the dark side of the planet starts.
    float darkness_factor = saturate(-32 * light_dir.z);
    diffuse += base_texel.a * darkness_factor * (1 - cloud_texel.a) * CityColor;

    return float4(lerp(diffuse + spec, cloud_diffuse, cloud_texel.a), 1);
}
