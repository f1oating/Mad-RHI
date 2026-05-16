struct Output
{
    float4 sv_position : SV_Position;
}

[shader("vertex")]
Output main()
{
    return { 1.0f };
}
