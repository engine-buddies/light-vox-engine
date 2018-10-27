struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

//Deferred SRVs
Texture2D gAlbedoTexture    : register(t0);
Texture2D gNormalTexture    : register(t1);
Texture2D gPositionTexture  : register(t2);

//Sampler
SamplerState gSampler       : register(s0);

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
    float3 albedo = gAlbedoTexture.Sample( gSampler, input.uv ).xyz;
    float3 normal = gNormalTexture.Sample( gSampler, input.uv ).xyz; 

    DirectionalLight dirLight;
    dirLight.diffuseColor = float4(0.8, 0.8, 0.8, 1.0);
    dirLight.direction = float3(0, -1, 0);

    float4 lights = (float4(0.1, 0.1, 0.1, 1.0)
        + dirLightDiffuse(dirLight, normal)) * float4(albedo, 1.0);
        
    float4 diffuse = gAlbedoTexture.Sample(gSampler, input.uv);
    return lights;// +lights;
}