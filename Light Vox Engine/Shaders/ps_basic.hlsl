struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
	float4 albedo  : SV_TARGET0;
	float4 normal   : SV_TARGET1;
    float4 position : SV_TARGET2;
};

PSOutput PSMain(PSInput input) : SV_TARGET
{
    PSOutput output;

    //re-normalize after rasterization
    input.normal = normalize(input.normal);

    //TO DO: Texturing:
    output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.normal = float4(input.normal, 1.0f);
    output.position = input.worldpos;

    return output;
}
