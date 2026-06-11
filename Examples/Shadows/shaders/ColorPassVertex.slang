struct Input
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Output
{
    float4 sv_position : SV_Position;
    float4 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Transform
{
    column_major float4x4 world;
    column_major float4x4 view;
    column_major float4x4 proj;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> uObjectTransform;

[shader("vertex")]
Output main(Input input)
{
    float4 worldPos = mul(uObjectTransform.world, float4(input.position, 1.0f));
    float4 viewPos = mul(uObjectTransform.view, worldPos);

    Output output;
    output.sv_position = mul(uObjectTransform.proj, viewPos);
    output.worldPos = worldPos;
    output.normal = mul(float3x3(uObjectTransform.world), input.normal);
    output.uv = input.uv;
    return output;
}
