import "Geometry";
import "DataStructures";

struct Output
{
    float4 sv_position : SV_Position;
    float4 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> uObjectTransform;

[shader("vertex")]
Output main(MeshVertexInput input)
{
    float4 worldPos = mul(uObjectTransform.world, float4(input.position, 1.0));
    float4 viewPos = mul(uObjectTransform.view, worldPos);

    Output output;
    output.sv_position = mul(uObjectTransform.proj, viewPos);
    output.worldPos = worldPos;
    output.normal = mul(float3x3(uObjectTransform.world), input.normal);
    output.uv = input.uv;
    return output;
}
