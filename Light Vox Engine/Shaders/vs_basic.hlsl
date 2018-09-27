struct PSInput
{
	float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
};

PSInput VSMain(
	float3 position : POSITION,
	float2 uv : TEXCOORD0,
	float3 normal : NORMAL)
{
	PSInput result;
	float4 pos = float4(position, 1.0f);
	pos = mul(pos, model);
	result.worldpos = pos;
	
	pos = mul(pos, view);
	pos = mul(pos, projection);
	result.position = pos;
	result.uv = uv;
    result.normal = mul(normal, (float3x3)model);

	return result;
}
