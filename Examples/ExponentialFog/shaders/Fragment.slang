struct Input
{
    float4 sv_position : SV_Position;
    float2 uv : TEXCOORD;
    float depth : TEXCOORD1;
}

[shader("fragment")]
float4 main(Input input) : SV_Target
{
    float3 objectColor = float3(0.2f, 1.0f, 0.1f);
    float3 fogColor = float3(0.1f, 0.1f, 0.15f);
    float density = 0.5f;

    float fogFactor = exp(-density * input.depth);
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    float3 finalColor = lerp(fogColor, objectColor, fogFactor);

    return float4(finalColor, 1.0f);
}
