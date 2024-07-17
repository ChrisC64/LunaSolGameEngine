 struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Uv : TEXCOORD;
};

struct VS_INPUT
{
    float4 Pos : POSITION0;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
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

VS_OUTPUT vs(VS_INPUT input, uint instanceId : SV_VertexID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    matrix mvp = mul(Projection, mul(View, Model));
    output.Pos = mul(mvp, input.Pos);
    output.Uv = input.UV;
    output.Color = input.Pos;

	return output;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
    return input.Color;
}