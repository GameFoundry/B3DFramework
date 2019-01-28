//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A 4x4 matrix. Can be used for homogenous transformations of three dimensional vectors and points.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Matrix4 // Note: Must match C++ class Matrix4
    {
        /// <summary>
        /// A matrix with all zero values.
        /// </summary>
        public static readonly Matrix4 Zero = new Matrix4();

        /// <summary>
        /// Identity matrix.
        /// </summary>
        public static readonly Matrix4 Identity = new Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

        public float m00;
        public float m01;
        public float m02;
        public float m03;
        public float m10;
        public float m11;
        public float m12;
        public float m13;
        public float m20;
        public float m21;
        public float m22;
        public float m23;
        public float m30;
        public float m31;
        public float m32;
        public float m33;

        /// <summary>
        /// Creates a new matrix with the specified elements.
        /// </summary>
        public Matrix4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, 
            float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
        {
            this.m00 = m00;
            this.m01 = m01;
            this.m02 = m02;
            this.m03 = m03;
            this.m10 = m10;
            this.m11 = m11;
            this.m12 = m12;
            this.m13 = m13;
            this.m20 = m20;
            this.m21 = m21;
            this.m22 = m22;
            this.m23 = m23;
            this.m30 = m30;
            this.m31 = m31;
            this.m32 = m32;
            this.m33 = m33;
        }

        /// <summary>
        /// Value of the specified element in the matrix.
        /// </summary>
        /// <param name="row">Row index of the element to retrieve.</param>
        /// <param name="column">Column index of the element to retrieve.</param>
        /// <returns>Value of the element.</returns>
        public float this[int row, int column]
        {
            get
            {
                return this[row * 4 + column];
            }
            set
            {
                this[row * 4 + column] = value;
            }
        }

        /// <summary>
        /// Value of the specified element in the matrix using a linear index.
        /// Linear index can be calculated using the following formula: idx = row * 4 + column. 
        /// </summary>
        /// <param name="index">Linear index to get the value of.</param>
        /// <returns>Value of the element.</returns>
        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0:
                        return m00;
                    case 1:
                        return m01;
                    case 2:
                        return m02;
                    case 3:
                        return m03;
                    case 4:
                        return m10;
                    case 5:
                        return m11;
                    case 6:
                        return m12;
                    case 7:
                        return m13;
                    case 8:
                        return m20;
                    case 9:
                        return m21;
                    case 10:
                        return m22;
                    case 11:
                        return m23;
                    case 12:
                        return m30;
                    case 13:
                        return m31;
                    case 14:
                        return m32;
                    case 15:
                        return m33;
                    default:
                        throw new IndexOutOfRangeException("Invalid matrix index.");
                }

            }
            set
            {
                switch (index)
                {
                    case 0:
                        m00 = value;
                        break;
                    case 1:
                        m01 = value;
                        break;
                    case 2:
                        m02 = value;
                        break;
                    case 3:
                        m03 = value;
                        break;
                    case 4:
                        m10 = value;
                        break;
                    case 5:
                        m11 = value;
                        break;
                    case 6:
                        m12 = value;
                        break;
                    case 7:
                        m13 = value;
                        break;
                    case 8:
                        m20 = value;
                        break;
                    case 9:
                        m21 = value;
                        break;
                    case 10:
                        m22 = value;
                        break;
                    case 11:
                        m23 = value;
                        break;
                    case 12:
                        m30 = value;
                        break;
                    case 13:
                        m31 = value;
                        break;
                    case 14:
                        m32 = value;
                        break;
                    case 15:
                        m33 = value;
                        break;
                    default:
                        throw new IndexOutOfRangeException("Invalid matrix index.");
                }
            }
        }

        /// <summary>
        /// Calculates the inverse of the matrix. 
        /// </summary>
        public Matrix4 Inverse
        {
            get
            {
                Matrix4 value = this;
                value.Invert();
                return value;
            }
        }

        /// <summary>
        /// Calculates the inverse of the matrix. Use only if matrix is affine.
        /// </summary>
        public Matrix4 InverseAffine
        {
            get
            {
                Matrix4 value = this;
                value.InvertAffine();
                return value;
            }
        }

        public static Matrix4 operator *(Matrix4 lhs, Matrix4 rhs)
        {
            return new Matrix4()
            {
                m00 = lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10 + lhs.m02 * rhs.m20 + lhs.m03 * rhs.m30,
                m01 = lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11 + lhs.m02 * rhs.m21 + lhs.m03 * rhs.m31,
                m02 = lhs.m00 * rhs.m02 + lhs.m01 * rhs.m12 + lhs.m02 * rhs.m22 + lhs.m03 * rhs.m32,
                m03 = lhs.m00 * rhs.m03 + lhs.m01 * rhs.m13 + lhs.m02 * rhs.m23 + lhs.m03 * rhs.m33,
                m10 = lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10 + lhs.m12 * rhs.m20 + lhs.m13 * rhs.m30,
                m11 = lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * rhs.m31,
                m12 = lhs.m10 * rhs.m02 + lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * rhs.m32,
                m13 = lhs.m10 * rhs.m03 + lhs.m11 * rhs.m13 + lhs.m12 * rhs.m23 + lhs.m13 * rhs.m33,
                m20 = lhs.m20 * rhs.m00 + lhs.m21 * rhs.m10 + lhs.m22 * rhs.m20 + lhs.m23 * rhs.m30,
                m21 = lhs.m20 * rhs.m01 + lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * rhs.m31,
                m22 = lhs.m20 * rhs.m02 + lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * rhs.m32,
                m23 = lhs.m20 * rhs.m03 + lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * rhs.m33,
                m30 = lhs.m30 * rhs.m00 + lhs.m31 * rhs.m10 + lhs.m32 * rhs.m20 + lhs.m33 * rhs.m30,
                m31 = lhs.m30 * rhs.m01 + lhs.m31 * rhs.m11 + lhs.m32 * rhs.m21 + lhs.m33 * rhs.m31,
                m32 = lhs.m30 * rhs.m02 + lhs.m31 * rhs.m12 + lhs.m32 * rhs.m22 + lhs.m33 * rhs.m32,
                m33 = lhs.m30 * rhs.m03 + lhs.m31 * rhs.m13 + lhs.m32 * rhs.m23 + lhs.m33 * rhs.m33
            };
        }

        public static bool operator ==(Matrix4 lhs, Matrix4 rhs)
        {
            if (lhs.m00 == rhs.m00 && lhs.m01 == rhs.m01 && lhs.m02 == rhs.m02 && lhs.m03 == rhs.m03 &&
                lhs.m10 == rhs.m10 && lhs.m11 == rhs.m11 && lhs.m12 == rhs.m12 && lhs.m13 == rhs.m13 &&
                lhs.m20 == rhs.m20 && lhs.m21 == rhs.m21 && lhs.m22 == rhs.m22 && lhs.m23 == rhs.m23 &&
                lhs.m30 == rhs.m30 && lhs.m31 == rhs.m31 && lhs.m32 == rhs.m32 && lhs.m33 == rhs.m33)
                return true;
            else
                return false;
        }

        public static bool operator !=(Matrix4 lhs, Matrix4 rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            float hash1 = m00.GetHashCode() ^ m10.GetHashCode() << 2 ^ m20.GetHashCode() >> 2 ^ m30.GetHashCode() >> 1;
            float hash2 = m01.GetHashCode() ^ m11.GetHashCode() << 2 ^ m21.GetHashCode() >> 2 ^ m31.GetHashCode() >> 1;
            float hash3 = m02.GetHashCode() ^ m12.GetHashCode() << 2 ^ m22.GetHashCode() >> 2 ^ m32.GetHashCode() >> 1;
            float hash4 = m03.GetHashCode() ^ m13.GetHashCode() << 2 ^ m23.GetHashCode() >> 2 ^ m33.GetHashCode() >> 1;

            return hash1.GetHashCode() ^ hash2.GetHashCode() << 2 ^ hash3.GetHashCode() >> 2 ^ hash4.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Matrix4))
                return false;

            Matrix4 mat = (Matrix4)other;
            if (m00 == mat.m00 && m01 == mat.m01 && m02 == mat.m02 && m03 == mat.m03 &&
                m10 == mat.m10 && m11 == mat.m11 && m12 == mat.m12 && m13 == mat.m13 &&
                m20 == mat.m20 && m21 == mat.m21 && m22 == mat.m22 && m23 == mat.m23 &&
                m30 == mat.m30 && m31 == mat.m31 && m32 == mat.m32 && m33 == mat.m33)
                return true;
            else
                return false;
        }

        /// <summary>
        /// Calculates the inverse of the matrix. If matrix is affine use <see cref="InvertAffine()"/> as it is faster.
        /// </summary>
        public void Invert()
        {
            float v0 = m20 * m31 - m21 * m30;
            float v1 = m20 * m32 - m22 * m30;
            float v2 = m20 * m33 - m23 * m30;
            float v3 = m21 * m32 - m22 * m31;
            float v4 = m21 * m33 - m23 * m31;
            float v5 = m22 * m33 - m23 * m32;

            float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
            float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
            float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
            float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

            float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

            float d00 = t00 * invDet;
            float d10 = t10 * invDet;
            float d20 = t20 * invDet;
            float d30 = t30 * invDet;

            float d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
            float d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
            float d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
            float d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

            v0 = m10 * m31 - m11 * m30;
            v1 = m10 * m32 - m12 * m30;
            v2 = m10 * m33 - m13 * m30;
            v3 = m11 * m32 - m12 * m31;
            v4 = m11 * m33 - m13 * m31;
            v5 = m12 * m33 - m13 * m32;

            float d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
            float d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
            float d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
            float d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

            v0 = m21 * m10 - m20 * m11;
            v1 = m22 * m10 - m20 * m12;
            v2 = m23 * m10 - m20 * m13;
            v3 = m22 * m11 - m21 * m12;
            v4 = m23 * m11 - m21 * m13;
            v5 = m23 * m12 - m22 * m13;

            float d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
            float d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
            float d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
            float d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

            this = new Matrix4(
                d00, d01, d02, d03,
                d10, d11, d12, d13,
                d20, d21, d22, d23,
                d30, d31, d32, d33);
        }

        /// <summary>
        /// Calculates the inverse of the matrix. Matrix must be affine.
        /// </summary>
        public void InvertAffine()
        {
            float t00 = m22 * m11 - m21 * m12;
            float t10 = m20 * m12 - m22 * m10;
            float t20 = m21 * m10 - m20 * m11;

            float invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

            t00 *= invDet; t10 *= invDet; t20 *= invDet;

            m00 *= invDet; m01 *= invDet; m02 *= invDet;

            float r00 = t00;
            float r01 = m02 * m21 - m01 * m22;
            float r02 = m01 * m12 - m02 * m11;

            float r10 = t10;
            float r11 = m00 * m22 - m02 * m20;
            float r12 = m02 * m10 - m00 * m12;

            float r20 = t20;
            float r21 = m01 * m20 - m00 * m21;
            float r22 = m00 * m11 - m01 * m10;

            float r03 = -(r00 * m03 + r01 * m13 + r02 * m23);
            float r13 = -(r10 * m03 + r11 * m13 + r12 * m23);
            float r23 = -(r20 * m03 + r21 * m13 + r22 * m23);

            this = new Matrix4(
                r00, r01, r02, r03,
                r10, r11, r12, r13,
                r20, r21, r22, r23,
                  0, 0, 0, 1);
        }

        /// <summary>
        /// Transposes the matrix (switched columns and rows).
        /// </summary>
        public void Transpose()
        {
            float tmp = m01;
            m01 = m10;
            m10 = tmp;

            tmp = m02;
            m02 = m20;
            m20 = tmp;

            tmp = m03;
            m03 = m30;
            m30 = tmp;

            tmp = m12;
            m12 = m21;
            m21 = tmp;

            tmp = m13;
            m13 = m31;
            m31 = tmp;

            tmp = m23;
            m23 = m32;
            m32 = tmp;
        }

        /// <summary>
        /// Calculates the determinant of the matrix.
        /// </summary>
        /// <returns>Determinant of the matrix.</returns>
        public float Determinant()
        {
            float m1 = m11 * (m22 * m33 - m32 * m23) -
                       m12 * (m21 * m33 - m31 * m23) +
                       m13 * (m21 * m32 - m31 * m22);

            float m2 = m10 * (m22 * m33 - m32 * m23) -
                       m12 * (m20 * m33 - m30 * m23) +
                       m13 * (m20 * m32 - m30 * m22);

            float m3 = m10 * (m21 * m33 - m31 * m23) -
                       m11 * (m20 * m33 - m30 * m23) +
                       m13 * (m20 * m31 - m30 * m21);

            float m4 = m10 * (m21 * m32 - m31 * m22) -
                       m11 * (m20 * m32 - m30 * m22) +
                       m12 * (m20 * m31 - m30 * m21);

            return m00 * m1 - m01 * m2 + m02 * m3 - m03 * m4;
        }

        /// <summary>
        /// Decompose a matrix to translation, rotation and scale components. Matrix must consist only of translation, 
        /// rotation and uniform scale transformations, otherwise accurate results are not guaranteed. Applying non-uniform 
        /// scale guarantees results will not be accurate.
        /// </summary>
        /// <param name="translation">Translation offset.</param>
        /// <param name="rotation">Rotation quaternion.</param>
        /// <param name="scale">Scale factors.</param>
        public void GetTRS(out Vector3 translation, out Quaternion rotation, out Vector3 scale)
        {
            Matrix3 m3x3 = ToMatrix3(this);

            Matrix3 matQ;
            Vector3 vecU;
            m3x3.QDUDecomposition(out matQ, out scale, out vecU);

            rotation = Quaternion.FromRotationMatrix(matQ);
            translation = new Vector3(m03, m13, m23);
        }

        /// <summary>
        /// Transform a 3D point by this matrix. Matrix must be affine. Affine multiplication offers better performance 
        /// than the general case.
        /// </summary>
        /// <param name="p">Point to transform.</param>
        /// <returns>Point transformed by this matrix.</returns>
        public Vector3 MultiplyAffine(Vector3 p)
        {
            return new Vector3(
                m00 * p.x + m01 * p.y + m02 * p.z + m03,
                m10 * p.x + m11 * p.y + m12 * p.z + m13,
                m20 * p.x + m21 * p.y + m22 * p.z + m23);
        }

        /// <summary>
        /// Transform a 3D direction vector by this matrix. w component is assumed to be 0. 
        /// </summary>
        /// <param name="d">Direction vector to transform.</param>
        /// <returns>Direction vector transformed by this matrix.</returns>
        public Vector3 MultiplyDirection(Vector3 d)
        {
            return new Vector3(
                    m00 * d.x + m01 * d.y + m02 * d.z + m03, 
                    m10 * d.x + m11 * d.y + m12 * d.z + m13,
                    m20 * d.x + m21 * d.y + m22 * d.z + m23);
        }

        /// <summary>
        /// Transform a 4D vector by this matrix. Matrix must be affine. Affine multiplication offers better performance 
        /// than the general case.
        /// </summary>
        /// <param name="v">Vector to transform.</param>
        /// <returns>Vector transformed by this matrix.</returns>
        public Vector4 MultiplyAffine(Vector4 v)
        {
            return new Vector4(
                m00 * v.x + m01 * v.y + m02 * v.z + m03 * v.w, 
                m10 * v.x + m11 * v.y + m12 * v.z + m13 * v.w,
                m20 * v.x + m21 * v.y + m22 * v.z + m23 * v.w,
                v.w);
        }

        /// <summary>
        /// Transform a 3D point by this matrix. w component of the vector is assumed to be 1. After transformation all 
        /// components are projected back so that w remains 1. If your matrix doesn't contain projection components use 
        /// <see cref="MultiplyAffine(Vector4)"/> as it is faster.
        /// </summary>
        /// <param name="p">Point to transform.</param>
        /// <returns>Point transformed by this matrix.</returns>
        public Vector3 Multiply(Vector3 p)
        {
            Vector3 r = new Vector3();

            float fInvW = 1.0f / (m30 * p.x + m31 * p.y + m32 * p.z + m33);

            r.x = (m00 * p.x + m01 * p.y + m02 * p.z + m03) * fInvW;
            r.y = (m10 * p.x + m11 * p.y + m12 * p.z + m13) * fInvW;
            r.z = (m20 * p.x + m21 * p.y + m22 * p.z + m23) * fInvW;

            return r;
        }

        /// <summary>
        /// Transform a 4D vector by this matrix. If your matrix doesn't contain projection components 
        /// use <see cref="MultiplyAffine(Vector4)"/> as it is faster.
        /// </summary>
        /// <param name="v">Vector to transform.</param>
        /// <returns>Vector transformed by this matrix.</returns>
        public Vector4 Multiply(Vector4 v)
        {
            return new Vector4(
                m00 * v.x + m01 * v.y + m02 * v.z + m03 * v.w, 
                m10 * v.x + m11 * v.y + m12 * v.z + m13 * v.w,
                m20 * v.x + m21 * v.y + m22 * v.z + m23 * v.w,
                m30 * v.x + m31 * v.y + m32 * v.z + m33 * v.w);
        }

        /// <summary>
        /// Sets values of a specific column in the matrix.
        /// </summary>
        /// <param name="columnIdx">Index of the column in range [0, 3].</param>
        /// <param name="column">Values to set in the column.</param>
        public void SetColumn(int columnIdx, Vector4 column)
        {
            this[0, columnIdx] = column.x;
            this[1, columnIdx] = column.y;
            this[2, columnIdx] = column.z;
            this[3, columnIdx] = column.w;
        }

        /// <summary>
        /// Creates a new matrix that performs translation, rotation and scale.
        /// </summary>
        /// <param name="translation">Offset to translate by.</param>
        /// <param name="rotation">Rotation quaternion.</param>
        /// <param name="scale">Non-uniform scale factors.</param>
        /// <returns>Matrix that performs scale, followed by rotation, followed by translation. </returns>
        public static Matrix4 TRS(Vector3 translation, Quaternion rotation, Vector3 scale)
        {
            Matrix3 rot3x3 = rotation.ToRotationMatrix();
            Matrix4 mat = new Matrix4();

            mat.m00 = scale.x * rot3x3.m00; mat.m01 = scale.y * rot3x3.m01; mat.m02 = scale.z * rot3x3.m02; mat.m03 = translation.x;
            mat.m10 = scale.x * rot3x3.m10; mat.m11 = scale.y * rot3x3.m11; mat.m12 = scale.z * rot3x3.m12; mat.m13 = translation.y;
            mat.m20 = scale.x * rot3x3.m20; mat.m21 = scale.y * rot3x3.m21; mat.m22 = scale.z * rot3x3.m22; mat.m23 = translation.z;

            // No projection term
            mat.m30 = 0; mat.m31 = 0; mat.m32 = 0; mat.m33 = 1;

            return mat;
        }

        /// <summary>
        /// Returns the rotation/scaling parts of a 4x4 matrix.
        /// </summary>
        /// <param name="mat">Matrix to extract the rotation/scaling from.</param>
        /// <returns>3x3 matrix representing an upper left portion of the provided matrix.</returns>
        public static Matrix3 ToMatrix3(Matrix4 mat)
        {
            return new Matrix3(
                mat.m00, mat.m01, mat.m02,
                mat.m10, mat.m11, mat.m12, 
                mat.m20, mat.m21, mat.m22);
        }

        /// <summary>
        /// Returns the inverse of the specified matrix.
        /// </summary>
        /// <param name="mat">Matrix to take inverse of.</param>
        /// <returns>Inverse of the provided matrix.</returns>
        public static Matrix4 Invert(Matrix4 mat)
        {
            Matrix4 copy = mat;
            copy.Invert();
            return copy;
        }

        /// <summary>
        /// Returns the inverse of the affine matrix. Faster than <see cref="Invert(Matrix4)"/>.
        /// </summary>
        /// <param name="mat">Affine matrix to take inverse of.</param>
        /// <returns>Inverse of the provided matrix.</returns>
        public static Matrix4 InvertAffine(Matrix4 mat)
        {
            Matrix4 copy = mat;
            copy.InvertAffine();
            return copy;
        }

        /// <summary>
        /// Returns a transpose of the matrix (switched columns and rows).
        /// </summary>
        public static Matrix4 Transpose(Matrix4 mat)
        {
            Matrix4 copy = mat;
            copy.Transpose();
            return copy;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return String.Format("({0}, {1}, {2}, {3},\n{4}, {5}, {6}, {7}\n{8}, {9}, {10}, {11}\n{12}, {13}, {14}, {15})",
                m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
        }
    }

    /** @} */
}
