Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedoSpec : register(t2);
SamplerState samplerLinear : register(s0);

cbuffer LightBuffer
{
    float3 lightPos[4];
    float3 lightColor[4];
    int numLights;
    float3 viewPos;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Texcoords : TEXCOORD;
};

float3 calculateLighting(float3 fragPos, float3 normal, float3 albedo, float specular)
{
    float3 lighting = float3(0, 0, 0);
    float3 viewDir = normalize(viewPos - fragPos);

    for (int i = 0; i < numLights; ++i)
    {
        float3 lightDir = normalize(lightPos[i] - fragPos);

        float diff = max(dot(normal, lightDir), 0.0);

        float3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);

        float3 diffuse = diff * lightColor[i];
        float3 specularColor = spec * lightColor[i] * specular;
        lighting += diffuse + specularColor;
    }

    return lighting * albedo;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float3 fragPos = gPosition.Sample(samplerLinear, input.Texcoords).xyz;
    float3 normal = normalize(gNormal.Sample(samplerLinear, input.Texcoords).xyz);
    float4 albedoSpec = gAlbedoSpec.Sample(samplerLinear, input.Texcoords);

    float3 albedo = albedoSpec.rgb;
    float specular = albedoSpec.a;
   
    //float3 lighting = calculateLighting(fragPos, normal, albedo, specular);

    float3 lightDir = normalize(float3(0.0, 1.0, 0.5));
    float3 lightColor = float3(2.0, 2.0, 2.0);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diff * lightColor * albedo;
    
    float3 lightDir1 = normalize(float3(0, -1.0, 0.5));
    float3 lightColor1 = float3(2.0, 2.0, 2.0);
    float diff1 = max(dot(normal, lightDir1), 0.0);
    float3 diffuse1 = diff1 * lightColor1 * albedo;
    
    float3 ambientColor = float3(0.3, 0.3, 0.3);
    float ambientIntensity = 0.5;
    float3 ambient = ambientColor * ambientIntensity;
    
    float3 finalColour = diffuse1 + diffuse + ambient;
    return float4(finalColour, 1.0);
}