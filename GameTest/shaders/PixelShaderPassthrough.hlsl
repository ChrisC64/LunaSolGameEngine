cbuffer Color : register(b0)
{
    float4 ModelColor;
}

float4 main(float4 Position : SV_Position) : SV_Target
{
    return float4(0.0f, 1.0f, 0.3f, 1.0f);
}