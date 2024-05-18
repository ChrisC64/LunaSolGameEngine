module;
#include <DirectXMath.h>
export module DirectXCommon.DXMatrixUtils;
import LSEDataLib;

using namespace DirectX;

export namespace LS::DX
{
    struct Matrix
    {
        inline XMMATRIX Identity()
        {
            return XMMatrixIdentity();
        }

        inline XMFLOAT4X4 ToMatrix4x4(XMMATRIX matrix)
        {
            XMFLOAT4X4 out;
            XMStoreFloat4x4(&out, matrix);
            return out;
        }
        
        inline XMFLOAT4X3 ToMatrix4x3(XMMATRIX matrix)
        {
            XMFLOAT4X3 out;
            XMStoreFloat4x3(&out, matrix);
            return out;
        }
        
        inline XMFLOAT3X3 ToMatrix3x3(XMMATRIX matrix)
        {
            XMFLOAT3X3 out;
            XMStoreFloat3x3(&out, matrix);
            return out;
        }
        
        inline XMFLOAT3X4 ToMatrix3x4(XMMATRIX matrix)
        {
            XMFLOAT3X4 out;
            XMStoreFloat3x4(&out, matrix);
            return out;
        }

        inline XMFLOAT4 ToVector4(XMVECTOR vector)
        {
            XMFLOAT4 out;
            XMStoreFloat4(&out, vector);
            return out;
        }
        
        inline XMFLOAT3 ToVector3(XMVECTOR vector)
        {
            XMFLOAT3 out;
            XMStoreFloat3(&out, vector);
            return out;
        }
        
        inline XMFLOAT2 ToVector2(XMVECTOR vector)
        {
            XMFLOAT2 out;
            XMStoreFloat2(&out, vector);
            return out;
        }

        inline XMVECTOR SetVector(float vector)
        {
            return XMLoadFloat(&vector);
        }
        
        inline XMVECTOR SetVector(XMFLOAT2 vector)
        {
            return XMLoadFloat2(&vector);
        }
        
        inline XMVECTOR SetVector(XMFLOAT3 vector)
        {
            return XMLoadFloat3(&vector);
        }
        
        inline XMVECTOR SetVector(XMFLOAT4 vector)
        {
            return XMLoadFloat4(&vector);
        }
        
        inline XMVECTOR SetVector(float x, float y, float z, float w)
        {
            return XMVectorSet(x, y, z, w);
        }

        template<class T>
        constexpr XMFLOAT4X4 FromMat4(const LS::Mat4<T>& mat)
        {
            XMFLOAT4X4 out{};
            out[0][0] = mat.at(0, 0);
            out[0][1] = mat.at(0, 1);
            out[0][2] = mat.at(0, 2);
            out[0][3] = mat.at(0, 3);
            
            out[1][0] = mat.at(1, 0);
            out[1][1] = mat.at(1, 1);
            out[1][2] = mat.at(1, 2);
            out[1][3] = mat.at(1, 3);
            
            out[2][0] = mat.at(2, 0);
            out[2][1] = mat.at(2, 1);
            out[2][2] = mat.at(2, 2);
            out[2][3] = mat.at(2, 3);
            
            out[3][0] = mat.at(3, 0);
            out[3][1] = mat.at(3, 1);
            out[3][2] = mat.at(3, 2);
            out[3][3] = mat.at(3, 3);

            return out;
        }
        
        template<class T>
        constexpr XMFLOAT3X3 FromMat3(const LS::Mat3<T>& mat)
        {
            XMFLOAT3X3 out{};
            out[0][0] = mat.at(0, 0);
            out[0][1] = mat.at(0, 1);
            out[0][2] = mat.at(0, 2);
            
            out[1][0] = mat.at(1, 0);
            out[1][1] = mat.at(1, 1);
            out[1][2] = mat.at(1, 2);
            
            out[2][0] = mat.at(2, 0);
            out[2][1] = mat.at(2, 1);
            out[2][2] = mat.at(2, 2);
            
            out[3][0] = mat.at(3, 0);
            out[3][1] = mat.at(3, 1);
            out[3][2] = mat.at(3, 2);

            return out;
        }
    };
}