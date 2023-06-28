struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Uv : TEXCOORD;
};

struct VS_INPUT
{
    float4 Pos : POSITION0;
};

cbuffer ScreenView : register(b0)
{
    float4x4 View;
}

cbuffer ScreenProjection : register(b1)
{
    float4x4 Projection;
}

cbuffer PerObject : register(b2)
{
    float4x4 Model;
}

cbuffer Color : register(b3)
{
    float4 ModelColor;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    matrix mvp = mul(Projection, mul(View, Model));
    output.Pos = mul(mvp, input.Pos);
    output.Color = ModelColor;
    output.Uv = float2(0.0f, 0.0f);
    return output;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
    return input.Color;
}