//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// A 3x3 matrix. Can be used for non-homogenous transformations of three dimensional vectors and points.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Matrix3 // Note: Must match C++ class Matrix3
    {
        /// <summary>
        /// Contains constant data that is used when calculating euler angles in a certain order.
        /// </summary>
        private struct EulerAngleOrderData
        {
            public EulerAngleOrderData(int a, int b, int c, float sign)
            {
                this.a = a;
                this.b = b;
                this.c = c;
                this.sign = sign;
            }

            public int a, b, c;
            public float sign;
        };

        private static EulerAngleOrderData[] EA_LOOKUP = 
        { new EulerAngleOrderData(0, 1, 2, 1.0f), new EulerAngleOrderData(0, 2, 1, -1.0f), new EulerAngleOrderData(1, 0, 2, -1.0f),
          new EulerAngleOrderData(1, 2, 0, 1.0f), new EulerAngleOrderData(2, 0, 1,  1.0f), new EulerAngleOrderData(2, 1, 0, -1.0f) };

        /// <summary>
        /// A matrix with all zero values.
        /// </summary>
        public static readonly Matrix3 Zero = new Matrix3();

        /// <summary>
        /// Identity matrix.
        /// </summary>
        public static readonly Matrix3 Identity = new Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);

        public float m00;
        public float m01;
        public float m02;
        public float m10;
        public float m11;
        public float m12;
        public float m20;
        public float m21;
        public float m22;

        /// <summary>
        /// Creates a new matrix with the specified elements.
        /// </summary>
        public Matrix3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
        {
            this.m00 = m00;
            this.m01 = m01;
            this.m02 = m02;
            this.m10 = m10;
            this.m11 = m11;
            this.m12 = m12;
            this.m20 = m20;
            this.m21 = m21;
            this.m22 = m22;
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
                return this[row * 3 + column];
            }
            set
            {
                this[row * 3 + column] = value;
            }
        }

        /// <summary>
        /// Value of the specified element in the matrix using a linear index.
        /// Linear index can be calculated using the following formula: idx = row * 3 + column. 
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
                        return m10;
                    case 4:
                        return m11;
                    case 5:
                        return m12;
                    case 6:
                        return m20;
                    case 7:
                        return m21;
                    case 8:
                        return m22;
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
                        m10 = value;
                        break;
                    case 4:
                        m11 = value;
                        break;
                    case 5:
                        m12 = value;
                        break;
                    case 6:
                        m20 = value;
                        break;
                    case 7:
                        m21 = value;
                        break;
                    case 8:
                        m22 = value;
                        break;
                    default:
                        throw new IndexOutOfRangeException("Invalid matrix index.");
                }
            }
        }

        public static Matrix3 operator *(Matrix3 lhs, Matrix3 rhs)
        {
            return new Matrix3()
            {
                m00 = lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10 + lhs.m02 * rhs.m20,
                m01 = lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11 + lhs.m02 * rhs.m21,
                m02 = lhs.m00 * rhs.m02 + lhs.m01 * rhs.m12 + lhs.m02 * rhs.m22,
                m10 = lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10 + lhs.m12 * rhs.m20,
                m11 = lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21,
                m12 = lhs.m10 * rhs.m02 + lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22,
                m20 = lhs.m20 * rhs.m00 + lhs.m21 * rhs.m10 + lhs.m22 * rhs.m20,
                m21 = lhs.m20 * rhs.m01 + lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21,
                m22 = lhs.m20 * rhs.m02 + lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22,
            };
        }

        public static bool operator== (Matrix3 lhs, Matrix3 rhs)
        {
            if (lhs.m00 == rhs.m00 && lhs.m01 == rhs.m01 && lhs.m02 == rhs.m02 && 
                lhs.m10 == rhs.m10 && lhs.m11 == rhs.m11 && lhs.m12 == rhs.m12 &&
                lhs.m20 == rhs.m20 && lhs.m21 == rhs.m21 && lhs.m22 == rhs.m22)
                return true;
            else
                return false;
        }

        public static bool operator !=(Matrix3 lhs, Matrix3 rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            float hash1 = m00.GetHashCode() ^ m10.GetHashCode() << 2 ^ m20.GetHashCode() >> 2;
            float hash2 = m01.GetHashCode() ^ m11.GetHashCode() << 2 ^ m21.GetHashCode() >> 2;
            float hash3 = m02.GetHashCode() ^ m12.GetHashCode() << 2 ^ m22.GetHashCode() >> 2;

            return hash1.GetHashCode() ^ hash2.GetHashCode() << 2 ^ hash3.GetHashCode() >> 2;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Matrix3))
                return false;

            Matrix3 mat = (Matrix3)other;
            if (m00 == mat.m00 && m01 == mat.m01 && m02 == mat.m02 &&
                m10 == mat.m10 && m11 == mat.m11 && m12 == mat.m12 &&
                m20 == mat.m20 && m21 == mat.m21 && m22 == mat.m22)
                return true;
            else
                return false;
        }

        /// <summary>
        /// Calculates an inverse of the matrix if it exists.
        /// </summary>
        public void Invert()
        {
            float[,] invVals = new float[3,3];

            invVals[0, 0] = m11 * m22 - m12 * m21;
            invVals[1, 0] = m12 * m20 - m10 * m22;
            invVals[2, 0] = m10 * m21 - m11 * m20;

            float det = m00 * invVals[0, 0] + m01 * invVals[1, 0] + m02 * invVals[2, 0];

            if (MathEx.Abs(det) <= 1e-06f)
                throw new DivideByZeroException("Matrix determinant is zero. Cannot invert.");

            invVals[0, 1] = m02 * m21 - m01 * m22;
            invVals[0, 2] = m01 * m12 - m02 * m11;

            invVals[1, 1] = m00 * m22 - m02 * m20;
            invVals[1, 2] = m02 * m10 - m00 * m12;

            invVals[2, 1] = m01 * m20 - m00 * m21;
            invVals[2, 2] = m00 * m11 - m01 * m10;

            float invDet = 1.0f/det;
            for (int row = 0; row < 3; row++)
            {
                for (int col = 0; col < 3; col++)
                    invVals[row, col] *= invDet;
            }
        }

        /// <summary>
        /// Returns a transpose of the matrix (switched columns and rows).
        /// </summary>
        public void Transpose()
        {
            float tmp = m10;
            m10 = m01;
            m01 = tmp;

            tmp = m20;
            m20 = m02;
            m02 = tmp;

            tmp = m12;
            m12 = m21;
            m21 = tmp;
        }

        /// <summary>
        /// Calculates the matrix determinant.
        /// </summary>
        /// <returns>Determinant of the matrix.</returns>
        public float Determinant()
        {
            float cofactor00 = m11 * m22 - m12 * m21;
            float cofactor10 = m12 * m20 - m10 * m22;
            float cofactor20 = m10 * m21 - m11 * m20;

            float det = m00 * cofactor00 + m01 * cofactor10 + m02 * cofactor20;

            return det;
        }

        /// <summary>
        /// Transforms the given vector by this matrix and returns the newly transformed vector.
        /// </summary>
        /// <param name="vec">Three dimensional vector to transform.</param>
        /// <returns>Vector transformed by the matrix.</returns>
        public Vector3 Transform(Vector3 vec)
        {
            Vector3 outVec;
            outVec.x = m00 * vec.x + m01 * vec.y + m02 * vec.z;
            outVec.y = m10 * vec.x + m11 * vec.y + m12 * vec.z;
            outVec.z = m20 * vec.x + m21 * vec.y + m22 * vec.z;
            return outVec;
        }

        /// <summary>
        /// Decomposes the matrix into a set of values.
        /// </summary>
        /// <param name="matQ">Columns form orthonormal bases. If your matrix is affine and doesn't use non-uniform scaling 
        /// this matrix will be the rotation part of the matrix.
        /// </param>
        /// <param name="vecD">If the matrix is affine these will be scaling factors of the matrix.</param>
        /// <param name="vecU">If the matrix is affine these will be shear factors of the matrix.</param>
        public void QDUDecomposition(out Matrix3 matQ, out Vector3 vecD, out Vector3 vecU)
        {
            matQ = new Matrix3();
            vecD = new Vector3();
            vecU = new Vector3();

            // Build orthogonal matrix Q
            float invLength = MathEx.InvSqrt(m00*m00 + m10*m10 + m20*m20);
            matQ.m00 = m00*invLength;
            matQ.m10 = m10*invLength;
            matQ.m20 = m20*invLength;

            float dot = matQ.m00*m01 + matQ.m10*m11 + matQ.m20*m21;
            matQ.m01 = m01-dot*matQ.m00;
            matQ.m11 = m11-dot*matQ.m10;
            matQ.m21 = m21-dot*matQ.m20;

            invLength = MathEx.InvSqrt(matQ.m01*matQ.m01 + matQ.m11*matQ.m11 + matQ.m21*matQ.m21);
            matQ.m01 *= invLength;
            matQ.m11 *= invLength;
            matQ.m21 *= invLength;

            dot = matQ.m00*m02 + matQ.m10*m12 + matQ.m20*m22;
            matQ.m02 = m02-dot*matQ.m00;
            matQ.m12 = m12-dot*matQ.m10;
            matQ.m22 = m22-dot*matQ.m20;

            dot = matQ.m01*m02 + matQ.m11*m12 + matQ.m21*m22;
            matQ.m02 -= dot*matQ.m01;
            matQ.m12 -= dot*matQ.m11;
            matQ.m22 -= dot*matQ.m21;

            invLength = MathEx.InvSqrt(matQ.m02*matQ.m02 + matQ.m12*matQ.m12 + matQ.m22*matQ.m22);
            matQ.m02 *= invLength;
            matQ.m12 *= invLength;
            matQ.m22 *= invLength;

            // Guarantee that orthogonal matrix has determinant 1 (no reflections)
            float fDet = matQ.m00*matQ.m11*matQ.m22 + matQ.m01*matQ.m12*matQ.m20 +
                matQ.m02*matQ.m10*matQ.m21 - matQ.m02*matQ.m11*matQ.m20 -
                matQ.m01*matQ.m10*matQ.m22 - matQ.m00*matQ.m12*matQ.m21;

            if (fDet < 0.0f)
            {
                matQ.m00 = -matQ.m00;
                matQ.m01 = -matQ.m01;
                matQ.m02 = -matQ.m02;
                matQ.m10 = -matQ.m10;
                matQ.m11 = -matQ.m11;
                matQ.m12 = -matQ.m12;
                matQ.m20 = -matQ.m20;
                matQ.m21 = -matQ.m21;
                matQ.m21 = -matQ.m22;
            }

            // Build "right" matrix R
            Matrix3 matRight = new Matrix3();
            matRight.m00 = matQ.m00 * m00 + matQ.m10 * m10 + matQ.m20 * m20;
            matRight.m01 = matQ.m00 * m01 + matQ.m10 * m11 + matQ.m20 * m21;
            matRight.m11 = matQ.m01 * m01 + matQ.m11 * m11 + matQ.m21 * m21;
            matRight.m02 = matQ.m00 * m02 + matQ.m10 * m12 + matQ.m20 * m22;
            matRight.m12 = matQ.m01 * m02 + matQ.m11 * m12 + matQ.m21 * m22;
            matRight.m22 = matQ.m02 * m02 + matQ.m12 * m12 + matQ.m22 * m22;

            // The scaling component
            vecD[0] = matRight.m00;
            vecD[1] = matRight.m11;
            vecD[2] = matRight.m22;

            // The shear component
            float invD0 = 1.0f/vecD[0];
            vecU[0] = matRight.m01 * invD0;
            vecU[1] = matRight.m02 * invD0;
            vecU[2] = matRight.m12 / vecD[1];
        }

        /// <summary>
        /// Converts an orthonormal matrix to euler angle (pitch/yaw/roll) representation.
        /// </summary>
        /// <returns>Euler angles in degrees representing the rotation in this matrix.</returns>
        public Vector3 ToEulerAngles()
        {
            Radian xAngle = -MathEx.Asin(this[1, 2]);
            if (xAngle < MathEx.HalfPi)
            {
                if (xAngle > -MathEx.HalfPi)
                {
                    Radian yAngle = MathEx.Atan2(this[0, 2], this[2, 2]);
                    Radian zAngle = MathEx.Atan2(this[1, 0], this[1, 1]);

                    return new Vector3(xAngle.Degrees, yAngle.Degrees, zAngle.Degrees);
                }
                else
                {
                    // Note: Not an unique solution.
                    xAngle = -MathEx.HalfPi;
                    Radian yAngle = MathEx.Atan2(-this[0, 1], this[0, 0]);
                    Radian zAngle = (Radian)0.0f;

                    return new Vector3(xAngle.Degrees, yAngle.Degrees, zAngle.Degrees);
                }
            }
            else
            {
                // Note: Not an unique solution.
                xAngle = MathEx.HalfPi;
                Radian yAngle = MathEx.Atan2(this[0, 1], this[0, 0]);
                Radian zAngle = (Radian)0.0f;

                return new Vector3(xAngle.Degrees, yAngle.Degrees, zAngle.Degrees);
            }
        }

        /// <summary>
        /// Converts an orthonormal matrix to quaternion representation.
        /// </summary>
        /// <returns>Quaternion representing the rotation in this matrix.</returns>
        public Quaternion ToQuaternion()
        {
            return Quaternion.FromRotationMatrix(this);
        }

        /// <summary>
        /// Converts an orthonormal matrix to axis angle representation.
        /// </summary>
        /// <param name="axis">Axis around which the rotation is performed.</param>
        /// <param name="angle">Amount of rotation.</param>
        public void ToAxisAngle(out Vector3 axis, out Degree angle)
        {
            float trace = m00 + m11 + m22;
            float cos = 0.5f*(trace-1.0f);
            Radian radians = (Radian)MathEx.Acos(cos);  // In [0, PI]
            angle = radians;

            if (radians > (Radian)0.0f)
            {
                if (radians < MathEx.Pi)
                {
                    axis.x = m21 - m12;
                    axis.y = m02 - m20;
                    axis.z = m10 - m01;

                    axis.Normalize();
                }
                else
                {
                    // Angle is PI
                    float halfInverse;
                    if (m00 >= m11)
                    {
                        // r00 >= r11
                        if (m00 >= m22)
                        {
                            // r00 is maximum diagonal term
                            axis.x = 0.5f*MathEx.Sqrt(m00 - m11 - m22 + 1.0f);
                            halfInverse = 0.5f/axis.x;
                            axis.y = halfInverse*m01;
                            axis.z = halfInverse*m02;
                        }
                        else
                        {
                            // r22 is maximum diagonal term
                            axis.z = 0.5f*MathEx.Sqrt(m22 - m00 - m11 + 1.0f);
                            halfInverse = 0.5f/axis.z;
                            axis.x = halfInverse*m02;
                            axis.y = halfInverse*m12;
                        }
                    }
                    else
                    {
                        // r11 > r00
                        if (m11 >= m22)
                        {
                            // r11 is maximum diagonal term
                            axis.y = 0.5f*MathEx.Sqrt(m11 - m00 - m22 + 1.0f);
                            halfInverse  = 0.5f/axis.y;
                            axis.x = halfInverse*m01;
                            axis.z = halfInverse*m12;
                        }
                        else
                        {
                            // r22 is maximum diagonal term
                            axis.z = 0.5f*MathEx.Sqrt(m22 - m00 - m11 + 1.0f);
                            halfInverse = 0.5f/axis.z;
                            axis.x = halfInverse*m02;
                            axis.y = halfInverse*m12;
                        }
                    }
                }
            }
            else
            {
                // The angle is 0 and the matrix is the identity.  Any axis will
                // work, so just use the x-axis.
                axis.x = 1.0f;
                axis.y = 0.0f;
                axis.z = 0.0f;
            }
        }

        /// <summary>
        /// Calculates the inverse of the matrix if it exists.
        /// </summary>
        /// <param name="mat">Matrix to calculate the inverse of.</param>
        /// <returns>Inverse of the matrix.</returns>
        public static Matrix3 Inverse(Matrix3 mat)
        {
            Matrix3 copy = mat;
            copy.Invert();
            return copy;
        }


        /// <summary>
        /// Calculates the transpose of the matrix.
        /// </summary>
        /// <param name="mat">Matrix to calculate the transpose of.</param>
        /// <returns>Transpose of the matrix.</returns>
        public static Matrix3 Transpose(Matrix3 mat)
        {
            Matrix3 copy = mat;
            copy.Transpose();
            return copy;
        }

        /// <summary>
        /// Creates a rotation matrix from the provided euler angle (pitch/yaw/roll) rotation.
        /// </summary>
        /// <param name="euler">Euler angles in degrees.</param>
        /// <param name="order">The order in which rotations will be applied. Different rotations can be created depending 
        /// on the order.</param>
        /// <returns>Rotation matrix that can rotate an object to the specified angles.</returns>
        public static Matrix3 FromEuler(Vector3 euler, EulerAngleOrder order)
        {
            return FromEuler(new Degree(euler.x), new Degree(euler.y), new Degree(euler.y), order);
        }

        /// <summary>
        /// Creates a rotation matrix from the provided euler angle (pitch/yaw/roll) rotation.
        /// </summary>
        /// <param name="xAngle">Pitch angle of rotation.</param>
        /// <param name="yAngle">Yar angle of rotation.</param>
        /// <param name="zAngle">Roll angle of rotation.</param>
        /// <param name="order">The order in which rotations will be applied. Different rotations can be created depending
        /// on the order.</param>
        /// <returns>Rotation matrix that can rotate an object to the specified angles.</returns>
        public static Matrix3 FromEuler(Radian xAngle, Radian yAngle, Radian zAngle, EulerAngleOrder order)
        {
            EulerAngleOrderData l = EA_LOOKUP[(int)order];

            Matrix3[] mats = new Matrix3[3];

            float cx = MathEx.Cos(xAngle);
            float sx = MathEx.Sin(xAngle);
            mats[0] = new Matrix3(
                1.0f, 0.0f, 0.0f,
                0.0f, cx, -sx,
                0.0f, sx, cx);

            float cy = MathEx.Cos(yAngle);
            float sy = MathEx.Sin(yAngle);
            mats[1] = new Matrix3(
                cy, 0.0f, sy,
                0.0f, 1.0f, 0.0f,
                -sy, 0.0f, cy);

            float cz = MathEx.Cos(zAngle);
            float sz = MathEx.Sin(zAngle);
            mats[2] = new Matrix3(
                cz, -sz, 0.0f,
                sz, cz, 0.0f,
                0.0f, 0.0f, 1.0f);
    
            return mats[l.a]*(mats[l.b]*mats[l.c]);
        }

        /// <summary>
        /// Creates a rotation matrix from the provided euler angle (pitch/yaw/roll) rotation. Angles are applied in YXZ 
        /// order.
        /// </summary>
        /// <param name="euler">Euler angles in degrees.</param>
        /// <returns>Rotation matrix that can rotate an object to the specified angles.</returns>
        public static Matrix3 FromEuler(Vector3 euler)
        {
            return FromEuler(new Degree(euler.x), new Degree(euler.y), new Degree(euler.y));
        }

        /// <summary>
        /// Creates a rotation matrix from the provided euler angle (pitch/yaw/roll) rotation. Angles are applied in YXZ 
        /// order.
        /// </summary>
        /// <param name="xAngle">Pitch angle of rotation.</param>
        /// <param name="yAngle">Yar angle of rotation.</param>
        /// <param name="zAngle">Roll angle of rotation.</param>
        /// <returns>Rotation matrix that can rotate an object to the specified angles.</returns>
        public static Matrix3 FromEuler(Radian xAngle, Radian yAngle, Radian zAngle)
        {
            Matrix3 m = new Matrix3();

            float cx = MathEx.Cos(xAngle);
            float sx = MathEx.Sin(xAngle);

            float cy = MathEx.Cos(yAngle);
            float sy = MathEx.Sin(yAngle);

            float cz = MathEx.Cos(zAngle);
            float sz = MathEx.Sin(zAngle);

            m[0, 0] = cy * cz + sx * sy * sz;
            m[0, 1] = cz * sx * sy - cy * sz;
            m[0, 2] = cx * sy;

            m[1, 0] = cx * sz;
            m[1, 1] = cx * cz;
            m[1, 2] = -sx;

            m[2, 0] = -cz * sy + cy * sx * sz;
            m[2, 1] = cy * cz * sx + sy * sz;
            m[2, 2] = cx * cy;

            return m;
        }

        /// <summary>
        /// Creates a rotation matrix from axis/angle rotation.
        /// </summary>
        /// <param name="axis">Axis around which the rotation is performed.</param>
        /// <param name="angle">Amount of rotation.</param>
        /// <returns>Rotation matrix that can rotate an object around the specified axis for the specified amount.</returns>
        public static Matrix3 FromAxisAngle(Vector3 axis, Degree angle)
        {
            Matrix3 mat;

            float cos = MathEx.Cos(angle);
            float sin = MathEx.Sin(angle);
            float oneMinusCos = 1.0f - cos;
            float x2 = axis.x * axis.x;
            float y2 = axis.y * axis.y;
            float z2 = axis.z * axis.z;
            float xym = axis.x * axis.y * oneMinusCos;
            float xzm = axis.x * axis.z * oneMinusCos;
            float yzm = axis.y * axis.z * oneMinusCos;
            float xSin = axis.x * sin;
            float ySin = axis.y * sin;
            float zSin = axis.z * sin;

            mat.m00 = x2 * oneMinusCos + cos;
            mat.m01 = xym - zSin;
            mat.m02 = xzm + ySin;
            mat.m10 = xym + zSin;
            mat.m11 = y2 * oneMinusCos + cos;
            mat.m12 = yzm - xSin;
            mat.m20 = xzm - ySin;
            mat.m21 = yzm + xSin;
            mat.m22 = z2 * oneMinusCos + cos;

            return mat;
        }

        /// <summary>
        /// Creates a rotation matrix from a quaternion rotation.
        /// </summary>
        /// <param name="quat">Quaternion to create the matrix from.</param>
        /// <returns>Rotation matrix containing the equivalent rotation of the provided quaternion.</returns>
        public static Matrix3 FromQuaternion(Quaternion quat)
        {
            return quat.ToRotationMatrix();
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return String.Format("({0}, {1}, {2},\n{3}, {4}, {5}\n{6}, {7}, {8})",
                m00, m01, m02, m10, m11, m12, m20, m21, m22);
        }
    }

    /** @} */
}
