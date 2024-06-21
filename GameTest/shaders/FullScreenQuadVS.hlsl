struct PS_INPUT
{
	float4 Pos : SV_Position;
	float4 Color : COLOR0;
	float2 Uv : TEXCOORD;
};

PS_INPUT main(uint pos : SV_VertexID) 
{
	float4 oPos = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float2 uv = float2(0.0f, 0.0f);

	if (pos == 0)// Top Left
	{
		oPos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		uv = float2(0.0f, 0.0f);
	}
	else if (pos == 1)// Top Right
	{
		oPos = float4(1.0f, -1.0f, 0.0f, 1.0f);
		uv = float2(1.0f, 0.0f);
	}
	else if (pos == 2)// Bottom Left
	{
		oPos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
		uv = float2(0.0, 1.0f);
	}
	else if (pos == 3)// Top Right
	{
		oPos = float4(1.0f, -1.0f, 0.0f, 1.0f);
		uv = float2(1.0f, 0.0f);
	}
	else if (pos == 4)// Bottom Right
	{
		oPos = float4(1.0f, 1.0f, 0.0f, 0.0f);
		uv = float2(1.0f, 1.0f);
	}
	else if (pos == 5)// Bottom Left
	{
		oPos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
		uv = float2(0.0, 1.0f);
	}

	PS_INPUT psInput;
	psInput.Pos = oPos;
	psInput.Uv = uv;
	psInput.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);

	return psInput;
}