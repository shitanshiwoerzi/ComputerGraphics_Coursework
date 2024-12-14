Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedoSpec : register(t2);
SamplerState samplerLinear : register(s0);

cbuffer LightBuffer : register(b0)
{
    float3 lightPos[4];
    float3 lightColor[4];
    int numLights;
    float3 viewPos;
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

float4 PS(float2 texCoords : TEXCOORD) : SV_TARGET
{
    float3 fragPos = gPosition.Sample(samplerLinear, texCoords).xyz;
    float3 normal = normalize(gNormal.Sample(samplerLinear, texCoords).xyz);
    float4 albedoSpec = gAlbedoSpec.Sample(samplerLinear, texCoords);

    float3 albedo = albedoSpec.rgb;
    float specular = albedoSpec.a;
   
    float3 lighting = calculateLighting(fragPos, normal, albedo, specular);

    return float4(lighting, 1.0);
}