import "Geometry";
import "DataStructures";

struct Output
{
    float4 sv_position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> uLightTransform;

[vk::binding(0, 1)]
ConstantBuffer<Transform> uObjectTransform;

[shader("vertex")]
Output main(MeshVertexInput input)
{
    float4 worldPos = mul(uObjectTransform.world, float4(input.position, 1.0));
    float4 viewPos = mul(uLightTransform.view, worldPos);

    Output output;
    output.sv_position = mul(uLightTransform.proj, viewPos);
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}
