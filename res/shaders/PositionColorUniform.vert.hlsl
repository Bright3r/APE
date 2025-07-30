
cbuffer ModelViewProj : register(b0, space1)
{
	float4x4 uModel;
	float4x4 uView;
	float4x4 uProj;
};

struct Input
{
	float3 Position : TEXCOORD0;
	float4 Color : TEXCOORD1;
};

struct Output
{
	float4 Color : TEXCOORD0;
	float4 Position : SV_Position;
};

Output main(Input input)
{
	Output output;
	output.Color = input.Color;

	float4 worldPosition = float4(input.Position, 1.0f);
	//output.Position = mul(mul(mul(worldPosition, uProj), uView), uModel);
	output.Position = mul(mul(mul(worldPosition, uModel), uView), uProj);
	//output.Position = worldPosition;

	return output;
}
