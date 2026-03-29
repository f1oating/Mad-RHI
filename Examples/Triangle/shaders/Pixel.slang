struct Output
{
    float4 color;
}

[shader("fragment")]
Output main()
{
    Output output;
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return output;
}