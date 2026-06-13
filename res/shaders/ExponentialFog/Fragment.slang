import "Constants";

static const float3 FOG_COLOR = float3(0.1, 0.1, 0.15);
static const float DENSITY = 0.5;

struct Input
{
    float4 sv_position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depth : TEXCOORD1;
}

[shader("fragment")]
float4 main(Input input) : SV_Target
{
    float fogFactor = exp(-DENSITY * input.depth);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    float3 finalColor = lerp(FOG_COLOR, SURFACE_COLOR, fogFactor);

    return float4(finalColor, 1.0);
}
