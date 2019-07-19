//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// Quaternion used for representing rotations.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Quaternion // Note: Must match C++ class Quaternion
    {
        /// <summary>
        /// Contains constant data that is used when calculating euler angles in a certain order.
        /// </summary>
        private struct EulerAngleOrderData
        {
            public EulerAngleOrderData(int a, int b, int c)
            {
                this.a = a;
                this.b = b;
                this.c = c;
            }

            public int a, b, c;
        };

        /// <summary>
        /// Quaternion with all zero elements.
        /// </summary>
        public static readonly Quaternion Zero = new Quaternion(0.0f, 0.0f, 0.0f, 0.0f);

        /// <summary>
        /// Quaternion representing no rotation.
        /// </summary>
        public static readonly Quaternion Identity = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

        private static readonly float epsilon = 1e-03f;

        private static readonly EulerAngleOrderData[] EA_LOOKUP = new EulerAngleOrderData[6]
            { new EulerAngleOrderData(0, 1, 2), new EulerAngleOrderData(0, 2, 1), new EulerAngleOrderData(1, 0, 2),
              new EulerAngleOrderData(1, 2, 0), new EulerAngleOrderData(2, 0, 1), new EulerAngleOrderData(2, 1, 0) };

        public float x;
        public float y;
        public float z;
        public float w;

        /// <summary>
        /// Accesses a specific component of the quaternion.
        /// </summary>
        /// <param name="index">Index of the component (0 - x, 1 - y, 2 - z, 3 - w).</param>
        /// <returns>Value of the specific component.</returns>
        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    case 3:
                        return w;
                    default:
                        throw new IndexOutOfRangeException("Invalid Quaternion index.");
                }
            }
            set
            {
                switch (index)
                {
                    case 0:
                        x = value;
                        break;
                    case 1:
                        y = value;
                        break;
                    case 2:
                        z = value;
                        break;
                    case 3:
                        w = value;
                        break;
                    default:
                        throw new IndexOutOfRangeException("Invalid Quaternion index.");
                }
            }
        }

        /// <summary>
        /// Gets the positive x-axis of the coordinate system transformed by this quaternion.
        /// </summary>
        public Vector3 Right
        {
            get
            {
                float fTy = 2.0f*y;
                float fTz = 2.0f*z;
                float fTwy = fTy*w;
                float fTwz = fTz*w;
                float fTxy = fTy*x;
                float fTxz = fTz*x;
                float fTyy = fTy*y;
                float fTzz = fTz*z;

                return new Vector3(1.0f - (fTyy + fTzz), fTxy + fTwz, fTxz - fTwy);
            }
        }

        /// <summary>
        /// Gets the positive y-axis of the coordinate system transformed by this quaternion.
        /// </summary>
        public Vector3 Up
        {
            get
            {
                float fTx = 2.0f * x;
                float fTy = 2.0f * y;
                float fTz = 2.0f * z;
                float fTwx = fTx * w;
                float fTwz = fTz * w;
                float fTxx = fTx * x;
                float fTxy = fTy * x;
                float fTyz = fTz * y;
                float fTzz = fTz * z;

                return new Vector3(fTxy - fTwz, 1.0f - (fTxx + fTzz), fTyz + fTwx);
            }
        }

        /// <summary>
        /// Gets the positive z-axis of the coordinate system transformed by this quaternion.
        /// </summary>
        public Vector3 Forward
        {
            get
            {
                float fTx = 2.0f * x;
                float fTy = 2.0f * y;
                float fTz = 2.0f * z;
                float fTwx = fTx * w;
                float fTwy = fTy * w;
                float fTxx = fTx * x;
                float fTxz = fTz * x;
                float fTyy = fTy * y;
                float fTyz = fTz * y;

                return new Vector3(fTxz + fTwy, fTyz - fTwx, 1.0f - (fTxx + fTyy));
            }
        }

        /// <summary>
        /// Returns the inverse of the quaternion. Quaternion must be non-zero. Inverse quaternion has the opposite
        /// rotation of the original.
        /// </summary>
        public Quaternion Inverse
        {
            get
            {
                Quaternion copy = this;
                copy.Invert();
                return copy;
            }
        }

        /// <summary>
        /// Returns a normalized copy of the quaternion.
        /// </summary>
        public Quaternion Normalized
        {
            get
            {
                Quaternion copy = this;
                copy.Normalize();
                return copy;
            }
        }

        /// <summary>
        /// Constructs a new quaternion with the specified components.
        /// </summary>
        public Quaternion(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public static Quaternion operator* (Quaternion lhs, Quaternion rhs)
        {
            return new Quaternion((lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y),
                (lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z),
                (lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x),
                (lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z));
        }

        public static Quaternion operator* (float lhs, Quaternion rhs)
        {
            return new Quaternion(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
        }

        public static Quaternion operator+ (Quaternion lhs, Quaternion rhs)
        {
            return new Quaternion(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
        }

        public static Quaternion operator- (Quaternion lhs, Quaternion rhs)
        {
            return new Quaternion(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
        }

        public static Quaternion operator- (Quaternion quat)
        {
            return new Quaternion(-quat.x, -quat.y, -quat.z, -quat.w);
        }

        public static bool operator== (Quaternion lhs, Quaternion rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
        }

        public static bool operator!= (Quaternion lhs, Quaternion rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Calculates a dot product between two quaternions.
        /// </summary>
        /// <param name="a">First quaternion.</param>
        /// <param name="b">Second quaternion.</param>
        /// <returns>Dot product between the two quaternions.</returns>
        public static float Dot(Quaternion a, Quaternion b)
        {
            return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
        }

        /// <summary>
        /// Applies quaternion rotation to the specified point.
        /// </summary>
        /// <param name="point">Point to rotate.</param>
        /// <returns>Point rotated by the quaternion.</returns>
        public Vector3 Rotate(Vector3 point)
        {
            return ToRotationMatrix().Transform(point);
        }

        /// <summary>
        /// Initializes the quaternion with rotation that rotates from one direction to another.
        /// </summary>
        /// <param name="fromDirection">Rotation to start at.</param>
        /// <param name="toDirection">Rotation to end at.</param>
        public void SetFromToRotation(Vector3 fromDirection, Vector3 toDirection)
        {
            SetFromToRotation(fromDirection, toDirection, Vector3.Zero);
        }

        /// <summary>
        /// Initializes the quaternion with rotation that rotates from one direction to another.
        /// </summary>
        /// <param name="fromDirection">Rotation to start at.</param>
        /// <param name="toDirection">Rotation to end at.</param>
        /// <param name="fallbackAxis">Fallback axis to use if the from/to vectors are almost completely opposite.
        ///                            Fallback axis should be perpendicular to both vectors.</param>
        public void SetFromToRotation(Vector3 fromDirection, Vector3 toDirection, Vector3 fallbackAxis)
        {
            fromDirection.Normalize();
            toDirection.Normalize();

            float d = Vector3.Dot(fromDirection, toDirection);

            // If dot == 1, vectors are the same
            if (d >= 1.0f)
            {
                this = Identity;
                return;
            }

            if (d < (1e-6f - 1.0f))
            {
                if (fallbackAxis != Vector3.Zero)
                {
                    // Rotate 180 degrees about the fallback axis
                    this = FromAxisAngle(fallbackAxis, MathEx.Pi);
                }
                else
                {
                    // Generate an axis
                    Vector3 axis = Vector3.Cross(Vector3.XAxis, fromDirection);
                    if (axis.SqrdLength < ((1e-06f * 1e-06f))) // Pick another if collinear
                        axis = Vector3.Cross(Vector3.YAxis, fromDirection);
                    axis.Normalize();
                    this = FromAxisAngle(axis, MathEx.Pi);
                }
            }
            else
            {
                float s = MathEx.Sqrt((1+d)*2);
                float invs = 1 / s;

                Vector3 c = Vector3.Cross(fromDirection, toDirection);

                x = c.x * invs;
                y = c.y * invs;
                z = c.z * invs;
                w = s * 0.5f;
                Normalize();
            }
        }

        /// <summary>
        /// Normalizes the quaternion.
        /// </summary>
        /// <returns>Length of the quaternion prior to normalization.</returns>
        public float Normalize()
        {
            float len = MathEx.Sqrt(w * w + x * x + y * y + z * z);
            if (len > 1e-08f)
                this = (1.0f / len) * this;

            return len;
        }

        /// <summary>
        /// Calculates the inverse of the quaternion. Inverse quaternion has the opposite rotation of the original.
        /// </summary>
        public void Invert()
        {
            float fNorm = w * w + x * x + y * y + z * z;
            if (fNorm > 0.0f)
            {
                float fInvNorm = 1.0f / fNorm;
                x *= -fInvNorm;
                y *= -fInvNorm;
                z *= -fInvNorm;
                w *= fInvNorm;
            }
            else
            {
                this = Zero;
            }
        }

        /// <summary>
        /// Initializes the quaternion so that it orients an object so it faces in te provided direction.
        /// </summary>
        /// <param name="forward">Direction to orient the object towards.</param>
        public void SetLookRotation(Vector3 forward)
        {
            FromToRotation(-Vector3.ZAxis, forward);
        }

        /// <summary>
        /// Initializes the quaternion so that it orients an object so it faces in te provided direction.
        /// </summary>
        /// <param name="forward">Direction to orient the object towards.</param>
        /// <param name="up">Axis that determines the upward direction of the object.</param>
        public void SetLookRotation(Vector3 forward, Vector3 up)
        {
            Vector3 forwardNrm = Vector3.Normalize(forward);
            Vector3 upNrm = Vector3.Normalize(up);

            if (MathEx.ApproxEquals(Vector3.Dot(forwardNrm, upNrm), 1.0f))
            {
                SetLookRotation(forwardNrm);
                return;
            }

            Vector3 x = Vector3.Cross(forwardNrm, upNrm);
            Vector3 y = Vector3.Cross(x, forwardNrm);

            x.Normalize();
            y.Normalize();

            this = Quaternion.FromAxes(x, y, -forwardNrm);
        }

        /// <summary>
        /// Performs spherical interpolation between two quaternions. Spherical interpolation neatly interpolates between
        /// two rotations without modifying the size of the vector it is applied to (unlike linear interpolation).
        /// </summary>
        /// <param name="from">Start quaternion.</param>
        /// <param name="to">End quaternion.</param>
        /// <param name="t">Interpolation factor in range [0, 1] that determines how much to interpolate between
        /// <paramref name="from"/> and <paramref name="to"/>.</param>
        /// <param name="shortestPath">Should the interpolation be performed between the shortest or longest path between
        ///                            the two quaternions.</param>
        /// <returns>Interpolated quaternion representing a rotation between <paramref name="from"/> and
        /// <paramref name="to"/>.</returns>
        public static Quaternion Slerp(Quaternion from, Quaternion to, float t, bool shortestPath = true)
        {
            float dot = Dot(from, to);
            Quaternion quat;

            if (dot < 0.0f && shortestPath)
            {
                dot = -dot;
                quat = -to;
            }
            else
            {
                quat = to;
            }

            if (MathEx.Abs(dot) < (1 - epsilon))
            {
                float sin = MathEx.Sqrt(1 - (dot*dot));
                Radian angle = MathEx.Atan2(sin, dot);
                float invSin = 1.0f / sin;
                float a = MathEx.Sin((1.0f - t) * angle) * invSin;
                float b = MathEx.Sin(t * angle) * invSin;

                return a * from + b * quat;
            }
            else
            {
                Quaternion ret = (1.0f - t) * from + t * quat;

                ret.Normalize();
                return ret;
            }
        }

        /// <summary>
        /// Returns the inverse of the quaternion. Quaternion must be non-zero. Inverse quaternion has the opposite
        /// rotation of the original.
        /// </summary>
        /// <param name="rotation">Quaternion to calculate the inverse for.</param>
        /// <returns>Inverse of the provided quaternion.</returns>
        public static Quaternion Invert(Quaternion rotation)
        {
            Quaternion copy = rotation;
            copy.Invert();

            return copy;
        }

        /// <summary>
        /// Calculates an angle between two rotations.
        /// </summary>
        /// <param name="a">First rotation.</param>
        /// <param name="b">Second rotation.</param>
        /// <returns>Angle between the rotations, in degrees.</returns>
        public static Degree Angle(Quaternion a, Quaternion b)
        {
            return (MathEx.Acos(MathEx.Min(MathEx.Abs(Dot(a, b)), 1.0f)) * 2.0f);
        }

        /// <summary>
        /// Converts the quaternion rotation into axis/angle rotation.
        /// </summary>
        /// <param name="axis">Axis around which the rotation is performed.</param>
        /// <param name="angle">Amount of rotation.</param>
        public void ToAxisAngle(out Vector3 axis, out Degree angle)
        {
            float fSqrLength = x*x+y*y+z*z;
            if (fSqrLength > 0.0f)
            {
                angle = 2.0f * MathEx.Acos(w);
                float fInvLength = MathEx.InvSqrt(fSqrLength);
                axis.x = x*fInvLength;
                axis.y = y*fInvLength;
                axis.z = z*fInvLength;
            }
            else
            {
                // Angle is 0, so any axis will do
                angle = (Degree)0.0f;
                axis.x = 1.0f;
                axis.y = 0.0f;
                axis.z = 0.0f;
            }
        }

        /// <summary>
        /// Converts a quaternion into an orthonormal set of axes.
        /// </summary>
        /// <param name="xAxis">Output normalized x axis.</param>
        /// <param name="yAxis">Output normalized y axis.</param>
        /// <param name="zAxis">Output normalized z axis.</param>
        public void ToAxes(ref Vector3 xAxis, ref Vector3 yAxis, ref Vector3 zAxis)
        {
            Matrix3 matRot = ToRotationMatrix();

            xAxis.x = matRot[0, 0];
            xAxis.y = matRot[1, 0];
            xAxis.z = matRot[2, 0];

            yAxis.x = matRot[0, 1];
            yAxis.y = matRot[1, 1];
            yAxis.z = matRot[2, 1];

            zAxis.x = matRot[0, 2];
            zAxis.y = matRot[1, 2];
            zAxis.z = matRot[2, 2];
        }

    /// <summary>
    /// Converts the quaternion rotation into euler angle (pitch/yaw/roll) rotation.
    /// </summary>
    /// <returns>Rotation as euler angles, in degrees.</returns>
    public Vector3 ToEuler()
        {
            Matrix3 matRot = ToRotationMatrix();
            return matRot.ToEulerAngles();
        }

        /// <summary>
        /// Converts a quaternion rotation into a rotation matrix.
        /// </summary>
        /// <returns>Matrix representing the rotation.</returns>
        public Matrix3 ToRotationMatrix()
        {
            Matrix3 mat = new Matrix3();

            float tx = x + x;
            float ty = y + y;
            float tz = z + z;
            float twx = tx * w;
            float twy = ty * w;
            float twz = tz * w;
            float txx = tx * x;
            float txy = ty * x;
            float txz = tz * x;
            float tyy = ty * y;
            float tyz = tz * y;
            float tzz = tz * z;

            mat[0, 0] = 1.0f - (tyy + tzz);
            mat[0, 1] = txy - twz;
            mat[0, 2] = txz + twy;
            mat[1, 0] = txy + twz;
            mat[1, 1] = 1.0f - (txx + tzz);
            mat[1, 2] = tyz - twx;
            mat[2, 0] = txz - twy;
            mat[2, 1] = tyz + twx;
            mat[2, 2] = 1.0f - (txx + tyy);

            return mat;
        }

        /// <summary>
        /// Creates a quaternion with rotation that rotates from one direction to another.
        /// </summary>
        /// <param name="fromDirection">Rotation to start at.</param>
        /// <param name="toDirection">Rotation to end at.</param>
        /// <returns>Quaternion that rotates an object from <paramref name="fromDirection"/> to
        /// <paramref name="toDirection"/></returns>
        public static Quaternion FromToRotation(Vector3 fromDirection, Vector3 toDirection)
        {
            Quaternion q = new Quaternion();
            q.SetFromToRotation(fromDirection, toDirection);
            return q;
        }

        /// <summary>
        /// Creates a quaternion with rotation that rotates from one direction to another.
        /// </summary>
        /// <param name="fromDirection">Rotation to start at.</param>
        /// <param name="toDirection">Rotation to end at.</param>
        /// <param name="fallbackAxis">Fallback axis to use if the from/to vectors are almost completely opposite.
        ///                            Fallback axis should be perpendicular to both vectors.</param>
        /// <returns>Quaternion that rotates an object from <paramref name="fromDirection"/> to
        /// <paramref name="toDirection"/></returns>
        public static Quaternion FromToRotation(Vector3 fromDirection, Vector3 toDirection, Vector3 fallbackAxis)
        {
            Quaternion q = new Quaternion();
            q.SetFromToRotation(fromDirection, toDirection, fallbackAxis);
            return q;
        }

        /// <summary>
        /// Creates a quaternion that orients an object so it faces in te provided direction.
        /// </summary>
        /// <param name="forward">Direction to orient the object towards.</param>
        public static Quaternion LookRotation(Vector3 forward)
        {
            Quaternion quat = new Quaternion();
            quat.SetLookRotation(forward);

            return quat;
        }

        /// <summary>
        /// Creates a quaternion that orients an object so it faces in the provided direction.
        /// </summary>
        /// <param name="forward">Direction to orient the object towards.</param>
        /// <param name="up">Axis that determines the upward direction of the object.</param>
        public static Quaternion LookRotation(Vector3 forward, Vector3 up)
        {
            Quaternion quat = new Quaternion();
            quat.SetLookRotation(forward, up);

            return quat;
        }

        /// <summary>
        /// Converts the quaternion rotation into euler angle (pitch/yaw/roll) rotation.
        /// </summary>
        /// <param name="rotation">Quaternion to convert.</param>
        /// <returns>Rotation as euler angles, in degrees.</returns>
        public static Vector3 ToEuler(Quaternion rotation)
        {
            return rotation.ToEuler();
        }

        /// <summary>
        /// Converts the quaternion rotation into axis/angle rotation.
        /// </summary>
        /// <param name="rotation">Quaternion to convert.</param>
        /// <param name="axis">Axis around which the rotation is performed.</param>
        /// <param name="angle">Amount of rotation.</param>
        public static void ToAxisAngle(Quaternion rotation, out Vector3 axis, out Degree angle)
        {
            rotation.ToAxisAngle(out axis, out angle);
        }

        /// <summary>
        /// Creates a quaternion from a rotation matrix.
        /// </summary>
        /// <param name="rotMatrix">Rotation matrix to convert to quaternion.</param>
        /// <returns>Newly created quaternion that has equivalent rotation as the provided rotation matrix.</returns>
        public static Quaternion FromRotationMatrix(Matrix3 rotMatrix)
        {
            // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
            // article "Quaternion Calculus and Fast Animation".

            Quaternion quat = new Quaternion();
            float trace = rotMatrix.m00 + rotMatrix.m11 + rotMatrix.m22;
            float root;

            if (trace > 0.0f)
            {
                // |w| > 1/2, may as well choose w > 1/2
                root = MathEx.Sqrt(trace + 1.0f);  // 2w
                quat.w = 0.5f*root;
                root = 0.5f/root;  // 1/(4w)
                quat.x = (rotMatrix.m21 - rotMatrix.m12) * root;
                quat.y = (rotMatrix.m02 - rotMatrix.m20) * root;
                quat.z = (rotMatrix.m10 - rotMatrix.m01) * root;
            }
            else
            {
                // |w| <= 1/2
                int[] nextLookup = { 1, 2, 0 };
                int i = 0;

                if (rotMatrix.m11 > rotMatrix.m00)
                    i = 1;

                if (rotMatrix.m22 > rotMatrix[i, i])
                    i = 2;

                int j = nextLookup[i];
                int k = nextLookup[j];

                root = MathEx.Sqrt(rotMatrix[i,i] - rotMatrix[j, j] - rotMatrix[k, k] + 1.0f);

                quat[i] = 0.5f*root;
                root = 0.5f/root;

                quat.w = (rotMatrix[k, j] - rotMatrix[j, k]) * root;
                quat[j] = (rotMatrix[j, i] + rotMatrix[i, j]) * root;
                quat[k] = (rotMatrix[k, i] + rotMatrix[i, k]) * root;
            }

            quat.Normalize();

            return quat;
        }

        /// <summary>
        /// Creates a quaternion from axis/angle rotation.
        /// </summary>
        /// <param name="axis">Axis around which the rotation is performed.</param>
        /// <param name="angle">Amount of rotation.</param>
        /// <returns>Quaternion that rotates an object around the specified axis for the specified amount.</returns>
        public static Quaternion FromAxisAngle(Vector3 axis, Degree angle)
        {
            Quaternion quat;

            float halfAngle = (float)(0.5f*angle.Radians);
            float sin = (float)MathEx.Sin(halfAngle);
            quat.w = (float)MathEx.Cos(halfAngle);
            quat.x = sin * axis.x;
            quat.y = sin * axis.y;
            quat.z = sin * axis.z;

            return quat;
        }

        /// <summary>
        /// Initializes the quaternion from orthonormal set of axes.
        /// </summary>
        /// <param name="xAxis">Normalized x axis.</param>
        /// <param name="yAxis">Normalized y axis.</param>
        /// <param name="zAxis">Normalized z axis.</param>
        /// <returns>Quaternion that represents a rotation from base axes to the specified set of axes.</returns>
        public static Quaternion FromAxes(Vector3 xAxis, Vector3 yAxis, Vector3 zAxis)
        {
            Matrix3 mat;

            mat.m00 = xAxis.x;
            mat.m10 = xAxis.y;
            mat.m20 = xAxis.z;

            mat.m01 = yAxis.x;
            mat.m11 = yAxis.y;
            mat.m21 = yAxis.z;

            mat.m02 = zAxis.x;
            mat.m12 = zAxis.y;
            mat.m22 = zAxis.z;

            return FromRotationMatrix(mat);
        }

        /// <summary>
        /// Creates a quaternion from the provided euler angle (pitch/yaw/roll) rotation.
        /// </summary>
        /// <param name="xAngle">Pitch angle of rotation.</param>
        /// <param name="yAngle">Yar angle of rotation.</param>
        /// <param name="zAngle">Roll angle of rotation.</param>
        /// <param name="order">The order in which rotations will be applied. Different rotations can be created depending
        ///                     on the order.</param>
        /// <returns>Quaternion that can rotate an object to the specified angles.</returns>
        public static Quaternion FromEuler(Degree xAngle, Degree yAngle, Degree zAngle,
            EulerAngleOrder order = EulerAngleOrder.YXZ)
        {
            EulerAngleOrderData l = EA_LOOKUP[(int)order];

            Radian halfXAngle = xAngle * 0.5f;
            Radian halfYAngle = yAngle * 0.5f;
            Radian halfZAngle = zAngle * 0.5f;

            float cx = MathEx.Cos(halfXAngle);
            float sx = MathEx.Sin(halfXAngle);

            float cy = MathEx.Cos(halfYAngle);
            float sy = MathEx.Sin(halfYAngle);

            float cz = MathEx.Cos(halfZAngle);
            float sz = MathEx.Sin(halfZAngle);

            Quaternion[] quats = new Quaternion[3];
            quats[0] = new Quaternion(sx, 0.0f, 0.0f, cx);
            quats[1] = new Quaternion(0.0f, sy, 0.0f, cy);
            quats[2] = new Quaternion(0.0f, 0.0f, sz, cz);

            return (quats[l.a] * quats[l.b]) * quats[l.c];
        }

        /// <summary>
        /// Creates a quaternion from the provided euler angle (pitch/yaw/roll) rotation.
        /// </summary>
        /// <param name="euler">Euler angles in degrees.</param>
        /// <param name="order">The order in which rotations will be applied. Different rotations can be created depending
        ///                     on the order.</param>
        /// <returns>Quaternion that can rotate an object to the specified angles.</returns>
        public static Quaternion FromEuler(Vector3 euler, EulerAngleOrder order = EulerAngleOrder.YXZ)
        {
            return FromEuler((Degree)euler.x, (Degree)euler.y, (Degree)euler.z, order);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() << 2 ^ z.GetHashCode() >> 2 ^ w.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Quaternion))
                return false;

            Quaternion quat = (Quaternion)other;
            if (x.Equals(quat.x) && y.Equals(quat.y) && z.Equals(quat.z) && w.Equals(quat.w))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return String.Format("({0}, {1}, {2}, {3})", x, y, z, w);
        }
    }

    /** @} */
}
