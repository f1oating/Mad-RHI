struct Input
{
    float4 sv_position : SV_Position;
    float4 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Light
{
    column_major float4x4 lightView;
    column_major float4x4 lightProj;
}

[vk::binding(0, 1)]
Texture2D ShadowMap;

[vk::binding(0, 2)]
SamplerComparisonState ShadowSampler;

[vk::binding(0, 3)]
ConstantBuffer<Light> uLight;

float calcShadow(float4 pos)
{
    float4 viewPos = mul(uLight.lightView, pos);
    float4 projPos = mul(uLight.lightProj, viewPos);

    projPos.xyz = projPos.xyz / projPos.w;

    float2 uv = projPos.xy * float2(0.5, -0.5) + 0.5;

    float shadow = ShadowMap.SampleCmpLevelZero(ShadowSampler, uv, projPos.z - 0.005);

    return shadow;
}

[shader("fragment")]
float4 main(Input input) : SV_Target
{
    float4 color = float4(0.75, 0.72, 0.68, 1.0f);

    float shadowFactor = calcShadow(input.worldPos);

    color.x *= shadowFactor;
    color.y *= shadowFactor;
    color.z *= shadowFactor;

    return color;
}