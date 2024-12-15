Texture2D tex : register(t0);
Texture2D normalMap : register(t1);
SamplerState samplerLinear : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    float3 WorldPos : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 gPosition : SV_Target0;
    float4 gNormal : SV_Target1;
    float4 gAlbedoSpec : SV_Target2;
};

PS_OUTPUT PS(PS_INPUT input)
{
    PS_OUTPUT output;
    
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

    //// Example lighting: Simple diffuse lighting
    //float3 ambientColor = float3(0.3, 0.3, 0.3);
    //float ambientIntensity = 0.5;
    //float3 ambient = ambientColor * ambientIntensity;
    //float lightIntensity = 2;
    //float3 lightDir = normalize(float3(0.0, 1.0, 0.5)); // Light direction
    //float diffuse = max(dot(worldNormal, lightDir), 0.0) * lightIntensity;
    ////float diffuse = max(dot(input.Normal, lightDir), 0.0);
    if (colour.a < 0.5)
    {
        discard;
    }

    // to GBuffer
    output.gPosition = float4(input.WorldPos, 1.0);
    output.gNormal = float4(worldNormal, 1.0);
    output.gAlbedoSpec = float4(colour.rgb, 0.5);
    
    // Output the final color
    return output;
    //return float4(colour.rgb * diffuse + ambient, colour.a);
}