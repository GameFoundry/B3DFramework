//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// Values that represent in which order are euler angles applied when used in transformations.
    /// </summary>
    public enum EulerAngleOrder
    {
        XYZ,
        XZY,
        YXZ,
        YZX,
        ZXY,
        ZYX
    };

    /// <summary>
    /// Utility class providing common scalar math operations.
    /// </summary>
    public class MathEx
    {
        /// <summary>
        /// Pi constant.
        /// </summary>
        public static readonly Radian Pi = new Radian(3.14159265359f);

        /// <summary>
        /// Two times pi constant.
        /// </summary>
        public static readonly Radian TwoPi = new Radian(3.14159265359f * 2.0f);

        /// <summary>
        /// Half of pi constant.
        /// </summary>
        public static readonly Radian HalfPi = new Radian(3.14159265359f * 0.5f);

        /// <summary>
        /// Constant that converts degrees to radians.
        /// </summary>
        public static readonly float Deg2Rad = 3.14159265359f / 180.0f;

        /// <summary>
        /// Constant that converts radians to degrees.
        /// </summary>
        public static readonly float Rad2Deg = 180.0f / 3.14159265359f;

        /// <summary>
        /// Returns the minimum value of the two provided.
        /// </summary>
        /// <param name="a">First value to compare.</param>
        /// <param name="b">Second value to compare.</param>
        /// <returns>Minimum of the two values.</returns>
        public static float Min(float a, float b)
        {
            if (a < b)
                return a;
            
            return b;
        }

        /// <summary>
        /// Returns the minimum value of all the values provided.
        /// </summary>
        /// <param name="values">Values to compare.</param>
        /// <returns>Minimum of all the values.</returns>
        public static float Min(params float[] values)
        {
            int length = values.Length;
            if (length == 0)
                return 0.0f;

            float min = values[0];
            for (int i = 1; i < length; i++)
            {
                if (values[i] < min)
                    min = values[i];
            }

            return min;
        }

        /// <summary>
        /// Returns the minimum value of the two provided.
        /// </summary>
        /// <param name="a">First value to compare.</param>
        /// <param name="b">Second value to compare.</param>
        /// <returns>Minimum of the two values.</returns>
        public static int Min(int a, int b)
        {
            if (a < b)
                return a;

            return b;
        }

        /// <summary>
        /// Returns the minimum value of the two provided.
        /// </summary>
        /// <param name="a">First value to compare.</param>
        /// <param name="b">Second value to compare.</param>
        /// <returns>Minimum of the two values.</returns>
        public static Radian Min(Radian a, Radian b)
        {
            if (a < b)
                return a;

            return b;
        }

        /// <summary>
        /// Returns the minimum value of all the values provided.
        /// </summary>
        /// <param name="values">Values to compare.</param>
        /// <returns>Minimum of all the values.</returns>
        public static int Min(params int[] values)
        {
            int length = values.Length;
            if (length == 0)
                return 0;

            int min = values[0];
            for (int i = 1; i < length; i++)
            {
                if (values[i] < min)
                    min = values[i];
            }

            return min;
        }

        /// <summary>
        /// Returns the maximum value of the two provided.
        /// </summary>
        /// <param name="a">First value to compare.</param>
        /// <param name="b">Second value to compare.</param>
        /// <returns>Maximum of the two values.</returns>
        public static float Max(float a, float b)
        {
            if (a > b)
                return a;

            return b;
        }

        /// <summary>
        /// Returns the maximum value of all the values provided.
        /// </summary>
        /// <param name="values">Values to compare.</param>
        /// <returns>Maximum of all the values.</returns>
        public static float Max(params float[] values)
        {
            int length = values.Length;
            if (length == 0)
                return 0.0f;

            float max = values[0];
            for (int i = 1; i < length; i++)
            {
                if (values[i] > max)
                    max = values[i];
            }

            return max;
        }

        /// <summary>
        /// Returns the maximum value of the two provided.
        /// </summary>
        /// <param name="a">First value to compare.</param>
        /// <param name="b">Second value to compare.</param>
        /// <returns>Maximum of the two values.</returns>
        public static int Max(int a, int b)
        {
            if (a > b)
                return a;
            else
                return b;
        }

        /// <summary>
        /// Returns the maximum value of the two provided.
        /// </summary>
        /// <param name="a">First value to compare.</param>
        /// <param name="b">Second value to compare.</param>
        /// <returns>Maximum of the two values.</returns>
        public static Radian Max(Radian a, Radian b)
        {
            if (a > b)
                return a;
            else
                return b;
        }

        /// <summary>
        /// Returns the maximum value of all the values provided.
        /// </summary>
        /// <param name="values">Values to compare.</param>
        /// <returns>Maximum of all the values.</returns>
        public static int Max(params int[] values)
        {
            int length = values.Length;
            if (length == 0)
                return 0;

            int max = values[0];
            for (int i = 1; i < length; ++i)
            {
                if (values[i] > max)
                    max = values[i];
            }

            return max;
        }

        /// <summary>
        /// Returns the absolute value of the provided parameter.
        /// </summary>
        /// <param name="f">Parameter to take absolute value of.</param>
        /// <returns>Absolute value of <paramref name="f"/>.</returns>
        public static float Abs(float f)
        {
            return Math.Abs(f);
        }

        /// <summary>
        /// Returns the absolute value of the provided parameter.
        /// </summary>
        /// <param name="value">Parameter to take absolute value of.</param>
        /// <returns>Absolute value of <paramref name="value"/>.</returns>
        public static int Abs(int value)
        {
            return Math.Abs(value);
        }

        /// <summary>
        /// Raises <paramref name="f"/> to the power of <paramref name="p"/>.
        /// </summary>
        /// <param name="f">Value to raise to a power.</param>
        /// <param name="p">Power to raise the value to.</param>
        /// <returns><paramref name="f"/> raised to the power of <paramref name="p"/>.</returns>
        public static float Pow(float f, float p)
        {
            return (float)Math.Pow(f, p);
        }

        /// <summary>
        /// Raises e to the power of <paramref name="power"/>.
        /// </summary>
        /// <param name="power">Power to raise e to.</param>
        /// <returns>e raised to the power of <paramref name="power"/>.</returns>
        public static float Exp(float power)
        {
            return (float)Math.Exp(power);
        }

        /// <summary>
        /// Returns the logarithm of a number in a specified base.
        /// </summary>
        /// <param name="f">Value to get the logarithm of.</param>
        /// <param name="p">Base of the logarithm</param>
        /// <returns>Logarithm of a number in the specified base.</returns>
        public static float Log(float f, float p)
        {
            return (float)Math.Log(f, p);
        }

        /// <summary>
        /// Returns the natural logarithm (base e).
        /// </summary>
        /// <param name="f">Value to get the logarithm of.</param>
        /// <returns>Natural logarithm of a number.</returns>
        public static float Log(float f)
        {
            return (float)Math.Log(f);
        }

        /// <summary>
        /// Returns the logarithm of a number in base 10.
        /// </summary>
        /// <param name="f">Value to get the logarithm of.</param>
        /// <returns>Logarithm of a number in base 10.</returns>
        public static float Log10(float f)
        {
            return (float)Math.Log10(f);
        }

        /// <summary>
        /// Returns the smallest integral value that is greater than or equal to the provided value.
        /// </summary>
        /// <param name="f">Value to round.</param>
        /// <returns>Smallest integral value that is greater than or equal to the provided value.</returns>
        public static float Ceil(float f)
        {
            return (float)Math.Ceiling(f);
        }

        /// <summary>
        /// Returns the largest integral value that is lesser than or equal to the provided value.
        /// </summary>
        /// <param name="f">Value to round.</param>
        /// <returns>Largest integral value that is lessert than or equal to the provided value.</returns>
        public static float Floor(float f)
        {
            return (float)Math.Floor(f);
        }

        /// <summary>
        /// Rounds the provided value to the nearest integral.
        /// </summary>
        /// <param name="f">Value to round.</param>
        /// <returns>Value rounded to the nearest integral.</returns>
        public static float Round(float f)
        {
            return (float)Math.Round(f);
        }

        /// <summary>
        /// Returns the smallest integral value that is greater than or equal to the provided value.
        /// </summary>
        /// <param name="f">Value to round.</param>
        /// <returns>Smallest integral value that is greater than or equal to the provided value.</returns>
        public static int CeilToInt(float f)
        {
            return (int)Math.Ceiling(f);
        }

        /// <summary>
        /// Returns the largest integral value that is lesser than or equal to the provided value.
        /// </summary>
        /// <param name="f">Value to round.</param>
        /// <returns>Largest integral value that is lessert than or equal to the provided value.</returns>
        public static int FloorToInt(float f)
        {
            return (int)Math.Floor(f);
        }

        /// <summary>
        /// Rounds the provided value to the nearest integral.
        /// </summary>
        /// <param name="f">Value to round.</param>
        /// <returns>Value rounded to the nearest integral.</returns>
        public static int RoundToInt(float f)
        {
            return (int)Math.Round(f);
        }

        /// <summary>
        /// Returns the sign of the provided value (positive or negative).
        /// </summary>
        /// <param name="f">Value to get the sign of.</param>
        /// <returns>-1.0f if negative or 1.0f if positive.</returns>
        public static float Sign(float f)
        {
            return f >= 0.0f ? 1.0f : -1.0f;
        }

        /// <summary>
        /// Returns the sine of the provided value.
        /// </summary>
        /// <param name="f">Angle in radians.</param>
        /// <returns>Sine of the angle.</returns>
        public static float Sin(float f)
        {
            return (float)Math.Sin(f);
        }

        /// <summary>
        /// Returns the cosine of the provided value.
        /// </summary>
        /// <param name="f">Angle in radians.</param>
        /// <returns>Cosine of the angle.</returns>
        public static float Cos(float f)
        {
            return (float)Math.Cos(f);
        }

        /// <summary>
        /// Returns the tangent of the provided value.
        /// </summary>
        /// <param name="f">Angle in radians.</param>
        /// <returns>Tangent of the angle.</returns>
        public static float Tan(float f)
        {
            return (float)Math.Tan(f);
        }

        /// <summary>
        /// Returns the angle whose sine is the specified number.
        /// </summary>
        /// <param name="f">Sine of an angle.</param>
        /// <returns>Angle in radians.</returns>
        public static Radian Asin(float f)
        {
            return (Radian)Math.Asin(f);
        }

        /// <summary>
        /// Returns the angle whose cosine is the specified number.
        /// </summary>
        /// <param name="f">Cosine of an angle.</param>
        /// <returns>Angle in radians.</returns>
        public static Radian Acos(float f)
        {
            return (Radian)Math.Acos(f);
        }

        /// <summary>
        /// Returns the angle whose tangent is the specified number.
        /// </summary>
        /// <param name="f">Tangent of an angle.</param>
        /// <returns>Angle in radians.</returns>
        public static Radian Atan(float f)
        {
            return (Radian)Math.Atan(f);
        }

        /// <summary>
        /// Returns an angle of a point.
        /// </summary>
        /// <param name="y">Y coordinate of the point.</param>
        /// <param name="x">X coordinate of the point.</param>
        /// <returns>Angle in radians in range [Pi, -Pi].</returns>
        public static Radian Atan2(float y, float x)
        {
            return (Radian)Math.Atan2(y, x);
        }

        /// <summary>
        /// Returns the sine of the provided value.
        /// </summary>
        /// <param name="f">Angle in radians.</param>
        /// <returns>Sine of the angle.</returns>
        public static float Sin(Radian f)
        {
            return (float)Math.Sin(f.Radians);
        }

        /// <summary>
        /// Returns the cosine of the provided value.
        /// </summary>
        /// <param name="f">Angle in radians.</param>
        /// <returns>Cosine of the angle.</returns>
        public static float Cos(Radian f)
        {
            return (float)Math.Cos(f.Radians);
        }

        /// <summary>
        /// Returns the tangent of the provided value.
        /// </summary>
        /// <param name="f">Angle in radians.</param>
        /// <returns>Tangent of the angle.</returns>
        public static float Tan(Radian f)
        {
            return (float)Math.Tan(f.Radians);
        }

        /// <summary>
        /// Returns the angle whose sine is the specified number.
        /// </summary>
        /// <param name="f">Sine of an angle.</param>
        /// <returns>Angle in radians.</returns>
        public static Radian Asin(Radian f)
        {
            return (Radian)Math.Asin(f.Radians);
        }

        /// <summary>
        /// Returns the angle whose cosine is the specified number.
        /// </summary>
        /// <param name="f">Cosine of an angle.</param>
        /// <returns>Angle in radians.</returns>
        public static Radian Acos(Radian f)
        {
            return (Radian)Math.Acos(f.Radians);
        }

        /// <summary>
        /// Returns the angle whose tangent is the specified number.
        /// </summary>
        /// <param name="f">Tangent of an angle.</param>
        /// <returns>Angle in radians.</returns>
        public static Radian Atan(Radian f)
        {
            return (Radian)Math.Atan(f.Radians);
        }

        /// <summary>
        /// Returns an angle of a point.
        /// </summary>
        /// <param name="y">Y coordinate of the point.</param>
        /// <param name="x">X coordinate of the point.</param>
        /// <returns>Angle in radians in range [Pi, -Pi].</returns>
        public static Radian Atan2(Radian y, Radian x)
        {
            return (Radian)Math.Atan2(y.Radians, x.Radians);
        }

        /// <summary>
        /// Returns a square root of the provided value.
        /// </summary>
        /// <param name="f">Value to take the square root of. Must not be negative.</param>
        /// <returns>Square root of the provided value.</returns>
        public static float Sqrt(float f)
        {
            return (float)Math.Sqrt(f);
        }

        /// <summary>
        /// Returns an inverse square root (1/sqrt(x)) of the provided value.
        /// </summary>
        /// <param name="f">Value to take the inverse square root of. Must not be negative or zero.</param>
        /// <returns>Inverse square root of the provided value.</returns>
        public static float InvSqrt(float f)
        {
            return 1.0f/(float) Math.Sqrt(f);
        }

        /// <summary>
        /// Clamps a value between two other values.
        /// </summary>
        /// <param name="value">Value to clamp.</param>
        /// <param name="min">Minimum value of the range to clamp. Must be lower than <paramref name="max"/></param>
        /// <param name="max">Maximum value of the range to clamp. Must be higher than <paramref name="min"/></param>
        /// <returns>Returns unchanged value if it is in valid range, otherwise returns value clamped to the range
        /// extremes. </returns>
        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;

            return value;
        }

        /// <summary>
        /// Clamps a value between two other values.
        /// </summary>
        /// <param name="value">Value to clamp.</param>
        /// <param name="min">Minimum value of the range to clamp. Must be lower than <paramref name="max"/></param>
        /// <param name="max">Maximum value of the range to clamp. Must be higher than <paramref name="min"/></param>
        /// <returns>Returns unchanged value if it is in valid range, otherwise returns value clamped to the range
        /// extremes. </returns>
        public static int Clamp(int value, int min, int max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;

            return value;
        }

        /// <summary>
        /// Clamps a value between two other values.
        /// </summary>
        /// <param name="value">Value to clamp.</param>
        /// <param name="min">Minimum value of the range to clamp. Must be lower than <paramref name="max"/></param>
        /// <param name="max">Maximum value of the range to clamp. Must be higher than <paramref name="min"/></param>
        /// <returns>Returns unchanged value if it is in valid range, otherwise returns value clamped to the range
        /// extremes. </returns>
        public static Radian Clamp(Radian value, Radian min, Radian max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;

            return value;
        }

        /// <summary>
        /// Clamps a value between zero and one.
        /// </summary>
        /// <param name="value">Value to clamp.</param>
        /// <returns>Returns unchanged value if it is in [0, 1] range, otherwise returns value clamped to the range. 
        /// </returns>
        public static float Clamp01(float value)
        {
            if (value < 0.0)
                return 0.0f;

            if (value > 1.0)
                return 1f;
            
            return value;
        }

        /// <summary>
        /// Linearly interpolates between two values.
        /// </summary>
        /// <param name="a">Starting value to interpolate from.</param>
        /// <param name="b">Ending value to interpolate towards.</param>
        /// <param name="t">Interpolation factor in specified range.</param>
        /// <param name="tmin">Minimum value for the range of <see cref="t"/></param>
        /// <param name="tmax">Maximum value for the range of <see cref="t"/></param>
        /// <returns>Interpolated value.</returns>
        public static float Lerp(float a, float b, float t, float tmin = 0.0f, float tmax = 1.0f)
        {
            t = Clamp01((t - tmin) / (tmax - tmin));

            return a * (1.0f - t) + b * t;
        }

        /// <summary>
        /// Wraps an angle in [0, 360) range. Values lower than zero, or higher or equal to 360
        /// will get wrapped around back into [0, 360) range.
        /// </summary>
        /// <param name="angle">Angle to wrap.</param>
        /// <returns>Angle in [0, 360) range.</returns>
        public static Degree WrapAngle(Degree angle)
        {
            const float inv360 = 1.0f/360.0f;
            float angleVal = angle.Degrees;
            float wrapCount = (float)MathEx.Floor(MathEx.Abs(angleVal) * inv360);

            if (angleVal > 0.0f)
                angleVal -= 360.0f * wrapCount;
            else if(angleVal < 0.0f)
                angleVal += 360.0f * (wrapCount + 1);

            return new Degree(angleVal);
        }

        /// <summary>
        /// Compares two floating point numbers with an error margin.
        /// </summary>
        /// <param name="a">First number to compare.</param>
        /// <param name="b">Second number to compare.</param>
        /// <param name="epsilon">Error margin within which the numbers should be considered equal.</param>
        /// <returns>True if equal, false otherwise.</returns>
        public static bool ApproxEquals(float a, float b, float epsilon = 1.192092896e-07F)
        {
            return Abs(b - a) <= epsilon;
        }
    }

    /** @} */
}
