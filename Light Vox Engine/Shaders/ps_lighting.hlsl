//Deferred Render Targets
Texture2D gAlbedoTexture    : register(t0);
Texture2D gNormalTexture    : register(t1);
Texture2D gPositionTexture  : register(t2);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct DirectionalLight
{
    float4 diffuseColor;
    float3 direction;
};

float4 dirLightDiffuse(DirectionalLight dirLight, float3 normal) {
    return dirLight.diffuseColor * saturate(dot(-normalize(dirLight.direction), normal));
};

float4 main(PSInput input) : SV_TARGET
{
    //lighting calculations (deferred)

    DirectionalLight dirLight;
    dirLight.diffuseColor = float4(0.8, 0.8, 0.8, 1.0);
    dirLight.direction = float3(-0.5, -0.5, 0);

    float4 lights = float4(0.1, 0.1, 0.1, 1.0)
        + dirLightDiffuse(dirLight, input.normal);

    return lights;
}