
Texture2D uTexture : register(t0, space2);
SamplerState uSampler : register(s0, space2);

struct Input
{
	float4 Position : SV_Position;
	float3 Normal : TEXCOORD0;
	float2 UV : TEXCOORD1;
};

float4 main(Input input) : SV_Target0
{
	float3 N = input.Normal;
	float3 L = float3(0, 1, 0);
	float ambientCoefficient = 0.2;
	float lambertCoefficient = 0.4;
	float4 specularColor = float4(1.f, 1.f, 1.f, 1.f);
	float specularCoefficient = 0.8;
	float shininess = 10.f;

	float4 diffuseColor = uTexture.Sample(uSampler, input.UV);

	float4 ambient = ambientCoefficient * diffuseColor;

	float4 lambertian = lambertCoefficient * diffuseColor * max(dot(N, L), 0.f);

	//float4 specular = specularCoefficient * specularColor * pow(max(dot(N, H), 0.f), shininess);

	return ambient + lambertian;
}

