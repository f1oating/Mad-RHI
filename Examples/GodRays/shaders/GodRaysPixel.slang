static const float  PI = 3.14159265;
static const float  SCATTERING = 0.15;
static const float  EXTINCTION = 0.12;
static const float  ANISOTROPY = 0.5;
static const float  MAX_DIST = 100.0;
static const uint   NUM_STEPS = 100;
static const float3 LIGHT_COLOR = float3(1.0, 0.5, 0.1);

struct Scene
{
    column_major float4x4 InvViewProj;
    float3 CameraPos;
    column_major float4x4 LightViewProj;
    float3 LightDir;
}

Texture2D SceneDepth;
Texture2D ShadowMap;
SamplerState PointClamp;
SamplerComparisonState ShadowSamp;

ParameterBlock<Scene> uScene;

float PhaseHG(float cosT, float g) 
{
    float g2 = g * g;
    float d  = 1.0 + g2 - 2.0 * g * cosT;
    return (1.0 - g2) / (4.0 * PI * pow(max(d, 1e-4), 1.5));
}

float3 WorldFromDepth(float2 uv, float depth) 
{
    float4 clip = float4(uv * 2.0 - 1.0, depth, 1.0);
    clip.y = -clip.y;
    float4 wp = mul(uScene.InvViewProj, clip);
    return wp.xyz / wp.w;
}

float IGN(float2 pos)
{
    return frac(52.9829189 * frac(0.06711056 * pos.x + 0.00583715 * pos.y));
}

[shader("fragment")]
float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target
{
    float depth = SceneDepth.SampleLevel(PointClamp, uv, 0).r;
    float3 endP  = WorldFromDepth(uv, depth);

    float3 ray = endP - uScene.CameraPos;
    float rLen = min(length(ray), MAX_DIST);
    float3 rDir = normalize(ray);

    float stepSize = rLen / float(NUM_STEPS);
    float3 stepVec = rDir * stepSize;

    float jitter = IGN(pos.xy);
    float3 marchPos = uScene.CameraPos + stepVec * jitter;

    float cosT = dot(rDir, -uScene.LightDir);
    float phase = PhaseHG(cosT, ANISOTROPY);

    float3 scattered = 0;
    float transmittance = 1;

    [loop]
    for (uint i = 0; i < NUM_STEPS; ++i) 
    {
        float4 sc = mul(uScene.LightViewProj, float4(marchPos, 1.0));
        sc.xyz /= sc.w;
        float2 shUV  = sc.xy * float2(0.5, -0.5) + 0.5;
        float shRef = sc.z;

        float visible = ShadowMap.SampleCmpLevelZero(ShadowSamp, shUV, shRef - 0.002);

        float3 sIn = LIGHT_COLOR * phase * SCATTERING * visible;
        float stepT = exp(-EXTINCTION * stepSize);
        float3 sInt = (sIn - sIn * stepT) / max(EXTINCTION, 1e-4);

        scattered += transmittance * sInt;
        transmittance *= stepT;

        marchPos += stepVec;
        if (transmittance < 0.01) break;
    }

    return float4(scattered, 1.0 - transmittance);
};