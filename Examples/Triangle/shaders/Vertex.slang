struct Output
{
    float4 sv_position : SV_Position;
}

[shader("vertex")]
Output main()
{
    Output output;
    output.sv_position = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return output;
}