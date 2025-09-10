
cbuffer Camera : register(b0, space1)
{
	float3 uCameraPos;
};

cbuffer ModelViewProj : register(b1, space1)
{
	float4x4 uModel;
	float4x4 uView;
	float4x4 uProj;
};

struct Input
{
	float3 Position : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float2 UV : TEXCOORD2;
};

struct Output
{
	float4 Position : SV_Position;
	float3 Normal : TEXCOORD0;
	float2 UV : TEXCOORD1;
	float3 ViewDirection : TEXCOORD2;
	float3 FragPos : TEXCOORD3;
};

Output main(Input input)
{
	Output output;

	// Calculate position with MVP matrix
	float4 worldPos = mul(float4(input.Position, 1.0f), uModel);
	output.FragPos = worldPos.xyz;
	output.ViewDirection = normalize(uCameraPos.xyz - worldPos.xyz);

	float4 viewPos = mul(worldPos, uView);
	output.Position = mul(viewPos, uProj);

	// Pass normal
	output.Normal= input.Normal;

	// Pass UV coords
	output.UV = input.UV;

	return output;
}

