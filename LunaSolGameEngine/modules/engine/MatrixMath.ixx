export module MathLib:MatrixMath;
import Data.LSMathTypes;

export namespace LS::Math
{
    template<class T>
    consteval auto MatrixMultiply(const Mat4<T>& a, const Mat4<T>& b)
    {
        const auto& ma = a.Mat;
        const auto& mb = b.Mat;

        Mat4<T> mo;
        auto& mm = mo.Mat;

        //Row x Column = A[row]0 x B[col]0
        mm[0] = ma[0] * mb[0 * 4] + ma[1] * mb[0 * 4 + 1] + ma[2] * mb[0 * 4 + 2] + ma[3] * mb[0 * 4 + 3];
        mm[1] = ma[0] * mb[1 * 4] + ma[1] * mb[1 * 4 + 1] + ma[2] * mb[1 * 4 + 2] + ma[3] * mb[1 * 4 + 3];
        mm[2] = ma[0] * mb[2 * 4] + ma[1] * mb[2 * 4 + 1] + ma[2] * mb[2 * 4 + 2] + ma[3] * mb[2 * 4 + 3];
        mm[3] = ma[0] * mb[3 * 4] + ma[1] * mb[3 * 4 + 1] + ma[2] * mb[3 * 4 + 2] + ma[3] * mb[3 * 4 + 3];

        //Row x Column = A[row]0 x B[col]0
        mm[4] = ma[4] * mb[0 * 4] + ma[5] * mb[0 * 4 + 1] + ma[6] * mb[0 * 4 + 2] + ma[7] * mb[0 * 4 + 3];
        mm[5] = ma[4] * mb[1 * 4] + ma[5] * mb[1 * 4 + 1] + ma[6] * mb[1 * 4 + 2] + ma[7] * mb[1 * 4 + 3];
        mm[6] = ma[4] * mb[2 * 4] + ma[5] * mb[2 * 4 + 1] + ma[6] * mb[2 * 4 + 2] + ma[7] * mb[2 * 4 + 3];
        mm[7] = ma[4] * mb[3 * 4] + ma[5] * mb[3 * 4 + 1] + ma[6] * mb[3 * 4 + 2] + ma[7] * mb[3 * 4 + 3];

        //Row x Column = A[row]0 x B[col]0
        mm[8] = ma[8] * mb[0 * 4] + ma[9] * mb[0 * 4 + 1] + ma[10] * mb[0 * 4 + 2] + ma[11] * mb[0 * 4 + 3];
        mm[9] = ma[8] * mb[1 * 4] + ma[9] * mb[1 * 4 + 1] + ma[10] * mb[1 * 4 + 2] + ma[11] * mb[1 * 4 + 3];
        mm[10] = ma[8] * mb[2 * 4] + ma[9] * mb[2 * 4 + 1] + ma[10] * mb[2 * 4 + 2] + ma[11] * mb[2 * 4 + 3];
        mm[11] = ma[8] * mb[3 * 4] + ma[9] * mb[3 * 4 + 1] + ma[10] * mb[3 * 4 + 2] + ma[11] * mb[3 * 4 + 3];

        //Row x Column = A[row]0 x B[col]0
        mm[12] = ma[12] * mb[0 * 4] + ma[13] * mb[0 * 4 + 1] + ma[14] * mb[0 * 4 + 2] + ma[15] * mb[0 * 4 + 3];
        mm[13] = ma[12] * mb[1 * 4] + ma[13] * mb[1 * 4 + 1] + ma[14] * mb[1 * 4 + 2] + ma[15] * mb[1 * 4 + 3];
        mm[14] = ma[12] * mb[2 * 4] + ma[13] * mb[2 * 4 + 1] + ma[14] * mb[2 * 4 + 2] + ma[15] * mb[2 * 4 + 3];
        mm[15] = ma[12] * mb[3 * 4] + ma[13] * mb[3 * 4 + 1] + ma[14] * mb[3 * 4 + 2] + ma[15] * mb[3 * 4 + 3];

        return mo;
    }
}