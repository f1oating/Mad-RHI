struct ColorData
{
    float4 tint;
}

ParameterBlock<ColorData> uColor;

struct Output 
{ 
    float4 color; 
}

[shader("fragment")]
Output main()
{
    Output output;
    output.color = uColor.tint;
    return output;
}
