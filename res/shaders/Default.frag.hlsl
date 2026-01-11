
cbuffer DebugMode : register(b0, space3)
{
	bool uShowNormals;
	float3 pad1;
};


Texture2D uTexture : register(t0, space2);

SamplerState uSampler : register(s0, space2);

struct Light 
{
	float4 position;
	float4 attenuation;
	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;

	int type;
	float3 dir;
};

StructuredBuffer<Light> uLights : register(t1, space2);

cbuffer LightData : register(b1, space3)
{
	int uLightCount;
	float3 pad2;
}


struct Material 
{
	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;

	float shininess;
	float3 pad;
};


float4 calcPointLight(Light light, float3 normal, float3 fragPos, float3 viewDir, Material mat)
{
	float3 lightDir = normalize(light.position.xyz - fragPos);

	float4 ambient = light.ambientColor * mat.ambientColor;

	float diff = max(dot(normal, lightDir), 0.f);
	float4 diffuse = light.diffuseColor * diff * mat.diffuseColor;

	float3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), mat.shininess);
	float4 specular = light.specularColor * spec * mat.specularColor;

	float dist = length(light.position.xyz - fragPos);
	float constantAtt = light.attenuation.x;
	float linearAtt = light.attenuation.y;
	float quadraticAtt = light.attenuation.z;
	float attenuation = 1.f / (constantAtt + linearAtt * dist + quadraticAtt * (dist * dist));

	return attenuation * (ambient + diffuse + specular);
}

float4 calcDirectionalLight(Light light, float3 normal, float3 viewDir, Material mat)
{
	float3 lightDir = normalize(-light.dir);

	float4 ambient = light.ambientColor * mat.ambientColor;

	float diff = max(dot(normal, lightDir), 0.f);
	float4 diffuse = light.diffuseColor * diff * mat.diffuseColor;

	float3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), mat.shininess);
	float4 specular = light.specularColor * spec * mat.specularColor;

	return (ambient + diffuse + specular);
}

struct Input
{
	float4 Position : SV_Position;
	float3 Normal : TEXCOORD0;
	float2 UV : TEXCOORD1;
	float3 ViewDirection : TEXCOORD2;
	float3 FragPos : TEXCOORD3;
};

float4 main(Input input) : SV_Target0
{
	float3 N = normalize(input.Normal);
	float3 V = normalize(input.ViewDirection);

	// Debug Output
	if (uShowNormals) {
		return float4(N * 0.5f + 0.5f, 1.f);
	}

	Material mat;
	mat.diffuseColor = uTexture.Sample(uSampler, input.UV);
	mat.ambientColor = mat.diffuseColor;
	mat.specularColor = float4(1.f, 1.f, 1.f, 1.f);
	mat.shininess = 2.5;

	float4 color = float4(0, 0, 0, 1);
	for (uint i = 0; i < uLightCount; ++i)
	{
		Light light = uLights[i];

		if (light.type == 0) {
			color += calcPointLight(light, N, input.FragPos, V, mat);
		}
		else {
			color += calcDirectionalLight(light, N, V, mat);
		}
	}

	return color;
}

