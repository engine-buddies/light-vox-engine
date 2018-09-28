struct PSInput
{
	float4 position : SV_POSITION;
	float4 worldpos : POSITION;
	float2 uv : TEXCOORD0;
};

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(input.worldpos.xyz, 1.0f);
}
