struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT vs(uint instanceId : SV_InstanceID)
{
    VS_OUTPUT output;
    output.Color = float4(1.0f, 0.0f, 1.0f, 1.0f);
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
    float2 texCoord = float2(instanceId & 1, instanceId >> 1);
    output.Pos = float4((texCoord.x - 0.5f) * 2, -(texCoord.y - 0.5f) * 2, 0.0f, 1.0f);
	return output;
}

float4 ps(VS_OUTPUT input) : SV_Target
{
    return input.Color;
}