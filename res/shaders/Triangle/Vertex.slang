struct Input
{
    float2 position : POSITION;
    float3 color : COLOR;
}

struct Output
{
    float4 sv_position : SV_Position;
    float3 color : COLOR;
}

[shader("vertex")]
Output main(Input input)
{
    Output output;
    output.sv_position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}
