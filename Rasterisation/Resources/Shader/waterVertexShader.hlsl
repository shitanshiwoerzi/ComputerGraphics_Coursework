cbuffer staticMeshBuffer
{
	float4x4 W;
	float4x4 VP;
};

cbuffer waterParams
{
    float time;
    float waveFrequency;
    float waveAmplitude;
    float waveSpeed;
};

struct VS_INPUT
{
	float4 Pos : POS;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoords : TEXCOORD;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoords : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.Pos = mul(input.Pos, W);
	
	 // add flucuate
    float wave1 = sin(output.Pos.x * waveFrequency + time * waveSpeed) * waveAmplitude;
    float wave2 = cos(output.Pos.z * waveFrequency + time * waveSpeed) * waveAmplitude;
    output.Pos.y += wave1 + wave2;
	output.Pos = mul(output.Pos, VP);
	output.Normal = mul(input.Normal, (float3x3)W);
	output.Tangent = mul(input.Tangent, (float3x3)W);
	output.TexCoords = input.TexCoords;
	return output;
}