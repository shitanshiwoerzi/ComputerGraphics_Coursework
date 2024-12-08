Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
SamplerState samplerLinear : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

float4 PS(PS_INPUT input) : SV_Target0
{
    float4 colour = tex.Sample(samplerLinear, input.TexCoords);
    // Sample the normal map and convert from [0,1] to [-1,1]
    float3 normalMapValue = normalMap.Sample(samplerLinear, input.TexCoords).rgb * 2.0 - 1.0;

    // Construct the TBN matrix (Tangent, Bitangent, Normal)
    float3 T = normalize(input.Tangent);
    float3 B = normalize(cross(input.Normal, T));
    float3 N = normalize(input.Normal);
    float3x3 TBN = float3x3(T, B, N);
    // Transform normal map value to world space
    float3 worldNormal = normalize(mul(normalMapValue, TBN));

    // Example lighting: Simple diffuse lighting
    float3 lightDir = normalize(float3(0.0, 1.0, 0.5)); // Light direction
    float diffuse = max(dot(worldNormal, lightDir), 0.0);

    // Output the final color
    return float4(colour.rgb * diffuse, colour.a);
}