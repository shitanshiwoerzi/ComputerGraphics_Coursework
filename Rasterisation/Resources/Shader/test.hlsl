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

struct PS_OUTPUT
{
    float4 Position : SV_Target0;
    float4 Normal : SV_Target1;
    float4 AlbedoSpec : SV_Target2;
};

PS_OUTPUT PS(PS_INPUT input)
{
    PS_OUTPUT output;

    float3 normalMapValue = normalMap.Sample(samplerLinear, input.TexCoords).rgb * 2.0 - 1.0;

    float3 T = normalize(input.Tangent);
    float3 B = normalize(cross(input.Normal, T));
    float3x3 TBN = float3x3(T, B, normalize(input.Normal));
    
    float3 worldNormal = normalize(mul(normalMapValue, TBN));

    output.Position = float4(input.Pos.xyz, 1.0);

    output.Normal = float4(worldNormal, 1.0);

    float4 albedo = tex.Sample(samplerLinear, input.TexCoords);
    output.AlbedoSpec = float4(albedo.rgb, 0.5);

    return output;
}
