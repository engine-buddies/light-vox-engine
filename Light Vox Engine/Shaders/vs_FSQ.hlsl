struct PSInput
{
	float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

PSInput main(
	float3 position : POSITION,
	float2 uv : TEXCOORD0,
	float3 normal : NORMAL)
{
	PSInput output;
	output.position = float4(position, 1.f);
	output.worldpos = float4(0.f, 0.f, 0.f, 0.f);
	output.normal = float3(0.f, 0.f, 0.f);
	output.uv = uv;
	return output;
}
