
cbuffer DebugMode : register(b0, space3)
{
	bool uShowNormals;
};

cbuffer LightingParameters : register(b1, space3) {
	float ambientCoefficient;
	float lambertCoefficient;
	float specularCoefficient;
	float shininess;
	float4 specularColor;
};

Texture2D uTexture : register(t0, space2);
SamplerState uSampler : register(s0, space2);

struct Input
{
	float4 Position : SV_Position;
	float3 Normal : TEXCOORD0;
	float2 UV : TEXCOORD1;
	float4 ViewDirection : TEXCOORD2;
};

float4 main(Input input) : SV_Target0
{
	float3 L = normalize(float3(0, 1, 0));
	float3 N = normalize(input.Normal);
	float3 V = normalize(input.ViewDirection.xyz);
	float3 H = normalize(L + V);

	// Debug Output
	if (uShowNormals) {
		return float4(N * 0.5f + 0.5f, 1.f);
	}

	// Blinn-Phong
	float4 diffuseColor = uTexture.Sample(uSampler, input.UV);

	float4 ambient = ambientCoefficient * diffuseColor;
	float4 lambertian = lambertCoefficient * diffuseColor * max(dot(N, L), 0.f);
	float4 specular = specularCoefficient * specularColor * pow(max(dot(N, H), 0.f), shininess);

	return ambient + lambertian + specular;
}

