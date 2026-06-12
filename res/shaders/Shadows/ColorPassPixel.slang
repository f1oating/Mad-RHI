import "Constants";
import "DataStructures";

struct Input
{
    float4 sv_position : SV_Position;
    float4 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Light
{
    float3 dir;
}

[vk::binding(0, 1)]
Texture2D ShadowMap;

[vk::binding(0, 2)]
SamplerComparisonState ShadowSampler;

[vk::binding(0, 3)]
ConstantBuffer<Transform> uLightTransform;

[vk::binding(0, 4)]
ConstantBuffer<Light> uLight;

float calcShadow(float4 pos)
{
    float4 viewPos = mul(uLightTransform.view, pos);
    float4 projPos = mul(uLightTransform.proj, viewPos);

    projPos.xyz = projPos.xyz / projPos.w;

    float2 uv = projPos.xy * float2(0.5, -0.5) + 0.5;

    float shadow = ShadowMap.SampleCmpLevelZero(ShadowSampler, uv, projPos.z - 0.005);

    return shadow;
}

[shader("fragment")]
float4 main(Input input) : SV_Target
{
    float shadowFactor = calcShadow(input.worldPos);

    float lighted = max(dot(normalize(input.normal), -uLight.dir), 0.0);

    float3 color = SURFACE_COLOR * (AMBIENT + lighted * shadowFactor * LIGHT_COLOR);

    return float4(color, 1.0);
}