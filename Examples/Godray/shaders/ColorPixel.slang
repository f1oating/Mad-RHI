struct Input
{
    float4 sv_position : SV_Position;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Light
{
    column_major float4x4 LightViewProj;
    float3 LightDir;
    float  _pad;
}

[vk::binding(0, 1)]
Texture2D ShadowMap;

[vk::binding(1, 1)]
SamplerComparisonState ShadowSamp;

[vk::binding(2, 1)]
ConstantBuffer<Light> uLight;

static const float3 SURFACE_COLOR = float3(0.75, 0.72, 0.68);
static const float3 LIGHT_COLOR = float3(1.0, 0.5, 0.1);
static const float  AMBIENT = 0.12;

[shader("fragment")]
float4 main(Input input) : SV_Target
{
    float4 sc = mul(uLight.LightViewProj, float4(input.worldPos, 1.0));
    sc.xyz /= sc.w;
    float2 shUV = sc.xy * float2(0.5, -0.5) + 0.5;
    float shadow = ShadowMap.SampleCmpLevelZero(ShadowSamp, shUV, sc.z - 0.002);

    float3 n = normalize(input.normal);
    float lambert = max(dot(n, -uLight.LightDir), 0.0);

    float3 color = SURFACE_COLOR * (AMBIENT + lambert * shadow * LIGHT_COLOR);
    return float4(color, 1.0);
}