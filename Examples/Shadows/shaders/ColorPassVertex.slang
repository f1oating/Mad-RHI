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

struct Object
{
    column_major float4x4 world;
    column_major float4x4 cameraView;
    column_major float4x4 cameraProj;
}

[vk::binding(0, 0)]
ConstantBuffer<Object> uScene;

[shader("vertex")]
Output main(Input input)
{
    float4 worldPos = mul(uScene.world, float4(input.position, 1.0f));
    float4 viewPos = mul(uScene.cameraView, worldPos);

    Output output;
    output.sv_position = mul(uScene.cameraProj, viewPos);
    output.worldPos = worldPos;
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}
