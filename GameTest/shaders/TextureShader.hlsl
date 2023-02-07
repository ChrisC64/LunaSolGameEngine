//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#pragma pack_matrix(row_major)
Texture2D tex2D : register(t0);
SamplerState samplerState : register(s0);

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

cbuffer cbChangesEveryFrame : register(b3)
{
	float4 vMeshColor;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Pos : POSITION;
	float2 Uv : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Uv : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(float4(input.Pos, 1.0f), Model);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Uv = input.Uv;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	return tex2D.Sample(samplerState, input.Uv) * float4(1.0f, 1.0f, 1.0f, 1.0f);
}
