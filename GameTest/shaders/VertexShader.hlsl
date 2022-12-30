struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
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


VS_OUTPUT vs(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    matrix mvp = mul(Projection, mul(View, Model));
    output.Pos = mul(mvp, input.Pos);
    /*output.Pos = mul(input.Pos, Model);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);*/

    //output.Color = float4(1.0f, 0.0f, 1.0f, 1.0f);
    output.Color = float4(input.Pos.xyz, 1.0f);
    /*output.Pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (instanceId == 0)
    {
        output.Pos = float4(0.0f, 0.5f, 0.0f, 1.0f);
    }
    else if (instanceId == 1)
    {
        output.Pos = float4(0.5f, 0.0f, 0.0f, 1.0f);
    }
    else if (instanceId == 2)
    {
        output.Pos = float4(-0.5f, 0.0f, 0.0f, 1.0f);
    }*/
    /*float2 texCoord = float2(instanceId & 1, instanceId >> 1);
    output.Pos = float4((texCoord.x - 0.5f) * 2, -(texCoord.y - 0.5f) * 2, 0.1f, 1.0f);*/
    //output.Pos = input.Pos;
	return output;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
    return input.Color;
}