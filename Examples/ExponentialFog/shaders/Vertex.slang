struct Input
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
}

struct Output
{
    float4 sv_position : SV_Position;
    float2 uv : TEXCOORD;
    float depth : TEXCOORD1;
}

struct Transform
{
    column_major float4x4 model;
    column_major float4x4 view;
    column_major float4x4 proj;
}

ParameterBlock<Transform> uTransform;

[shader("vertex")]
Output main(Input input)
{
    float4 worldPos = mul(uTransform.model, float4(input.pos, 1.0f));
    float4 viewPos = mul(uTransform.view, worldPos);
    Output output;
    output.sv_position = mul(uTransform.proj, viewPos);;
    output.uv = input.uv;
    output.depth = length(viewPos.xyz);
    return output;
}
