Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
SamplerState samplerLinear : register(s0);

float4 PS(float2 texCoords : TEXCOORD) : SV_Target
{
    float3 position = gPosition.Sample(samplerLinear, texCoords).xyz;
    float3 normal = normalize(gNormal.Sample(samplerLinear, texCoords).xyz);
    float3 albedo = gAlbedo.Sample(samplerLinear, texCoords).rgb;

    float3 lightDir = normalize(float3(0.5, 1.0, 0.5));
    float3 lightColor = float3(1.0, 1.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diff * lightColor * albedo;

    return float4(diffuse, 1.0);
}