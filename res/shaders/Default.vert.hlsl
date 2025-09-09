
cbuffer ModelViewProj : register(b0, space1)
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
	float4 ViewDirection : TEXCOORD2;
};

Output main(Input input)
{
	Output output;

	// Calculate position with MVP matrix
	float4 worldPosition = float4(input.Position, 1.0f);
	float4 viewPosition = mul(mul(worldPosition, uModel), uView);

	output.ViewDirection = normalize(-viewPosition);
	output.Position = mul(viewPosition, uProj);

	// Pass normal
	output.Normal= input.Normal;

	// Pass UV coords
	output.UV = input.UV;

	return output;
}

