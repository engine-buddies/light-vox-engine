<<<<<<< HEAD
void main() { }


=======
struct PSInput
{
	float4 position : SV_POSITION;
	float4 worldpos : POSITION;
	float2 uv : TEXCOORD0;
};

cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 model;
	float4x4 viewProj;
};

PSInput VSMain(
	float3 position : POSITION, 
	float2 uv : TEXCOORD0)
{
	PSInput result;
	float4 pos = float4(position, 1.0f);
	pos = mul(pos, model);
	result.worldpos = pos;

	pos = mul(pos, viewProj);
	result.position = pos;
	result.uv = uv;

	return result;
}
>>>>>>> 59154ef2b77365057381e5e77919f30342187edb
