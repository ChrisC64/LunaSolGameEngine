struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

struct VS_INPUT
{
    float4 Pos : POSITION0;
};

cbuffer Color : register(b3)
{
    float4 ModelColor;
}


VS_OUTPUT main(VS_INPUT input) 
{
    VS_OUTPUT output;
    output.Pos = input.Pos;
    output.Color = ModelColor;

    return output;
}