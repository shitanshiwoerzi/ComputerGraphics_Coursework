Texture2D skyTexture : register(t0);
SamplerState samplerState : register(s0); 

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 texCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = skyTexture.Sample(samplerState, input.texCoord);
    return color;
}