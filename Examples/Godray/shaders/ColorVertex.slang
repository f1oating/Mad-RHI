struct Input
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Output
{
    float4 sv_position : SV_Position;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Transform
{
    column_major float4x4 model;
    column_major float4x4 view;
    column_major float4x4 proj;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> uTransform;

[shader("vertex")]
Output main(Input input)
{
    float4 worldPos = mul(uTransform.model, float4(input.pos, 1.0f));
    float4 viewPos = mul(uTransform.view, worldPos);
    Output output;
    output.sv_position = mul(uTransform.proj, viewPos);
    output.worldPos = worldPos.xyz;
    output.normal = normalize(mul((float3x3)uTransform.model, input.normal));
    output.uv = input.uv;
    return output;
}
