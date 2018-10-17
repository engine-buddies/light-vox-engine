//Deferred Render Targets
Texture2D albedo    : register(t0);
Texture2D normal    : register(t1);
Texture2D position  : register(t2);


struct DirectionalLight
{
    float4 diffuseColor;
    float3 direction;
};

float4 dirLightDiffuse(DirectionalLight dirLight, float3 normal) {
    return dirLight.diffuseColor * saturate(dot(-normalize(dirLight.direction), normal));
};

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
    //lighting calculations (deferred)

    DirectionalLight dirLight;
    dirLight.diffuseColor = float4(0.8, 0.8, 0.8, 1.0);
    dirLight.direction = float3(-0.5, -0.5, 0);

    float4 lights = float4(0.1, 0.1, 0.1, 1.0)
        + dirLightDiffuse(dirLight, input.normal);

    return lights;
}