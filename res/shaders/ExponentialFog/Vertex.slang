import "Geometry";
import "DataStructures";

struct Output
{
    float4 sv_position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float depth : TEXCOORD1;
}

ParameterBlock<Transform> uTransform;

[shader("vertex")]
Output main(MeshVertexInput input)
{
    float4 worldPos = mul(uTransform.world, float4(input.position, 1.0));
    float4 viewPos = mul(uTransform.view, worldPos);
    Output output;
    output.sv_position = mul(uTransform.proj, viewPos);
    output.normal = input.normal;
    output.uv = input.uv;
    output.depth = length(viewPos.xyz);
    return output;
}
