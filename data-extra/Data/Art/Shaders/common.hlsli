struct DirectionalLight
{
    float3 direction;	// Direction the light is pointing to, in world space.
    float intensity;
    float3 diffuse_color;
    float3 specular_color;
};

struct PointLight
{
    float3 position;  // in world space
    float intensity;
    float3 diffuse_color;
    float3 specular_color;
    float max_distance; // in world space
};

cbuffer InstanceConstants
{
    float4x4 World;
    float4x4 WorldInv;
};

cbuffer ViewConstants
{
    // mul(View, Projection)
    float4x4 ViewProj;

    // inverse(ViewProj)
    float4x4 ViewProjInv;
};

cbuffer DirectionalLightConstants
{
    DirectionalLight DirectionalLights[3];
};

cbuffer EnvironmentConstants
{
    // Color of the scene's ambient light
    float3 LightAmbient;

    int NumDirectionalLights;

    int NumPointLights;

	// In-world time, in seconds
    float Time;
};

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

// Compute the light half-angle vector (for specular calculation).
float3 light_half_angle(float3 vertex_pos, float3 eye_pos, float3 light_pos)
{
    float3 V = normalize(eye_pos - vertex_pos); // vector from vertex to eye
    float3 L = normalize(light_pos - vertex_pos); // vector from vertex to light source
    return normalize(V + L);
}
