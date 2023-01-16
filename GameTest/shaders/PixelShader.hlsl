#pragma pack_matrix(row_major)
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

float4 ps(VS_OUTPUT input) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}