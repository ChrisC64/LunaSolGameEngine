#pragma pack_matrix(row_major)
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 uv : TEXCOORD0;
};

Texture2D image : register(t0);

SamplerState imageSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_Target
{
    return image.Sample(imageSampler, input.uv) + float4(0.0f, 1.0f, 0.0f, 1.0f);
}