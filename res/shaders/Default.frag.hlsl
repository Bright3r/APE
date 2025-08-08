
Texture2D uTexture : register(t0);
SamplerState uSampler : register(s0);

struct Input
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

float4 main(Input input) : SV_Target
{
	return uTexture.Sample(uSampler, input.UV);
}
