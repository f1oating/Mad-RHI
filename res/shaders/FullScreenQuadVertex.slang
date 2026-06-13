struct Input
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
}

struct Output
{
    float4 sv_position : SV_Position;
    float2 uv : TEXCOORD;
}

[shader("vertex")]
Output main(Input input)
{
    Output output;
    output.sv_position = float4(input.pos, 1.0, 1.0);
    output.uv = input.uv;
    return output;
}
