struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Uv : TEXCOORD;
};

cbuffer ScreenView : register(b0)
{
    float4x4 View;
}

cbuffer ScreenProjection : register(b1)
{
    float4x4 Projection;
}

cbuffer Color : register(b3)
{
    float4 ModelColor;
}

float4x4 Identity()
{
    float4x4 identity = (float4x4)0;
    identity[0][0] = 1.0f;
    identity[1][1] = 1.0f;
    identity[2][2] = 1.0f;
    identity[3][3] = 1.0f;

    return identity;
}

float4x4 ScaleMatrixFromVector(float4 vec)
{
    float4x4 mat = Identity();
    mat[0][0] = vec.x;
    mat[1][1] = vec.y;
    mat[2][2] = vec.z;
    return mat;
}

float4x4 TransMatrixFromVector(float4 vec)
{
    float4x4 mat = Identity();
    mat[3][0] = vec.x;
    mat[3][1] = vec.y;
    mat[3][2] = vec.z;
    return mat;
}

float4x4 RotateXMatrix(float angle)
{
    float4x4 mat = Identity();
    float rads = radians(angle);
    float fcos = cos(rads);
    float fsin = sin(rads);
    
    mat[1][1] = fcos;
    mat[1][2] = -fsin;
    mat[2][1] = fsin;
    mat[2][2] = -fcos;
    
    return mat;
}

float4x4 RotateYMatrix(float angle)
{
    float4x4 mat = Identity();
    float rads = radians(angle);
    float fcos = cos(rads);
    float fsin = sin(rads);
    
    mat[0][0] = fcos;
    mat[0][2] = fsin;
    mat[2][0] = -fsin;
    mat[2][2] = fcos;
    
    return mat;
}

float4x4 RotateZMatrix(float angle)
{
    float4x4 mat = Identity();
    float rads = radians(angle);
    float fcos = cos(rads);
    float fsin = sin(rads);
    
    mat[0][0] = fcos;
    mat[0][1] = -fsin;
    mat[1][0] = fsin;
    mat[1][1] = fcos;
    
    return mat;
}

float4x4 RotationMatrixZXY(float4 vec)
{
    float4x4 rz = RotateZMatrix(vec.z);
    float4x4 ry = RotateYMatrix(vec.y);
    float4x4 rx = RotateXMatrix(vec.x);

    float4x4 zy = mul(rz, ry);
    float4x4 final = mul(zy, rx);
    return final;
}

VS_OUTPUT main(float3 pos : POSITION0) 
{
    float4 scale = float4(1.0f, 1.0f, 1.0, 1.0f);
    float4 rotation = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 worldPos = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float4x4 scaleMat = ScaleMatrixFromVector(scale);
    float4x4 rotMat = RotationMatrixZXY(rotation);
    float4x4 wpMat = TransMatrixFromVector(worldPos);
    
    float4x4 transform = mul(wpMat, mul(scaleMat, rotMat));
    
    float4x4 mvp = mul(Projection, mul(View, transform));
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul(mvp, float4(pos.xyz, 1.0f));
    output.Color = float4(0.4f, 1.0f, 0.3f, 1.0f);
    output.Uv = float2(0.0f, 0.0f);
    
    return output;
}