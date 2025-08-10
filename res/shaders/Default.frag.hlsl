
Texture2D uTexture : register(t0, space2);
SamplerState uSampler : register(s0, space2);

struct Input
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

float4 main(Input input) : SV_Target0
{
	//return uTexture.Sample(uSampler, input.UV);
	return uTexture.Sample(uSampler, float2(0.5, 0.5));
	//return float4(input.UV, 0, 255);
}
