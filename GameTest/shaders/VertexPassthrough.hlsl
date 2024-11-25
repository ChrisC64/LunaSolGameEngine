struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

VS_OUTPUT main(float3 position : POSITION)
{
    VS_OUTPUT output;
    output.Pos = float4(position, 1.0f);

    return output;
}