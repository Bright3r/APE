
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
	float2 UV : TEXCOORD0;
};

Output main(Input input)
{
	Output output;

	// Calculate position with MVP matrix
	float4 worldPosition = float4(input.Position, 1.0f);
	output.Position = mul(mul(mul(worldPosition, uModel), uView), uProj);

	// Pass UV coords
	output.UV = input.UV;

	return output;
}

