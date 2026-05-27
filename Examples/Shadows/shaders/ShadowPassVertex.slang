struct Input
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Output
{
    float4 sv_position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

struct Light
{
    column_major float4x4 lightView;
    column_major float4x4 lightProj;
}

struct Object
{
    column_major float4x4 world;
    column_major float4x4 cameraView;
    column_major float4x4 cameraProj;
}

[vk::binding(0, 0)]
ConstantBuffer<Light> uLight;

[vk::binding(0, 1)]
ConstantBuffer<Object> uScene;

[shader("vertex")]
Output main(Input input)
{
    float4 worldPos = mul(uScene.world, float4(input.position, 1.0f));
    float4 viewPos = mul(uLight.lightView, worldPos);

    Output output;
    output.sv_position = mul(uLight.lightProj, viewPos);
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}
