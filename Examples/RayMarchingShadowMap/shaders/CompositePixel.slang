Texture2D SceneColor;
Texture2D Godrays;
SamplerState LinearClamp;

float3 Tonemap(float3 c) 
{
    return c / (1.0 + c);
}

[shader("fragment")]
float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target 
{
    float3 scene = SceneColor.SampleLevel(LinearClamp, uv, 0).rgb;
    float4 gr = Godrays.SampleLevel(LinearClamp, uv, 0);

    float3 result = scene * (1.0 - gr.a) + gr.rgb;

    result = Tonemap(result);

    return float4(result, 1.0);
}