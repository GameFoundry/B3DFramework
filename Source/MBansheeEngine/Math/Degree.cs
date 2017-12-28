//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    ///	Wrapper class which indicates a given angle value is in degrees. Degree values are interchangeable with radian 
    /// values, and conversions will be done automatically between them.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Degree // Note: Must match C++ class Degree
    {
        [SerializeField]
        readonly float value;

        /// <summary>
        /// Creates a new degree value.
        /// </summary>
        /// <param name="value">Value in degrees.</param>
        public Degree(float value = 0.0f)
        {
            this.value = value;
        }

        /// <summary>
        /// Creates a new degree value.
        /// </summary>
        /// <param name="r">Value in radians.</param>
        public Degree(Radian r)
        {
            this.value = r.Degrees;
        }

        /// <summary>
        /// Converts a undefined angle value to a degree value.
        /// </summary>
        /// <param name="value">Value in degrees.</param>
        /// <returns>Degree object wrapping the value.</returns>
        public static explicit operator Degree(float value)
        {
            return new Degree(value);
        }

        /// <summary>
        /// Converts a radian angle value to a degree value.
        /// </summary>
        /// <param name="r">Value in radians.</param>
        /// <returns>Degree object wrapping the value.</returns>
        public static implicit operator Degree(Radian r)
        {
            return new Degree(r.Degrees);
        }

        /// <summary>
        /// Converts a degree value to a regular floating point value.
        /// </summary>
        /// <param name="d">Degree value to convert.</param>
        /// <returns>Value in degrees as floating point type.</returns>
        public static explicit operator float(Degree d)
        {
            return d.value;
        }

        /// <summary>
        /// Returns the value in degrees as a floating point type.
        /// </summary>
        public float Degrees
        {
            get { return value; }
        }

        /// <summary>
        /// Returns the value in radians as a floating point type.
        /// </summary>
        public float Radians
        {
            get { return value*MathEx.Deg2Rad; }
        }

        public static Degree operator+(Degree a)
        {
            return a;
        }

        public static Degree operator+(Degree a, Degree b)
        {
            return new Degree(a.value + b.value);
        }

        public static Degree operator+(Degree a, Radian r) 
        { 
            return new Degree (a.value + r.Degrees); 
        }

        public static Degree operator-(Degree a)
        {
            return new Degree(-a.value);
        }

        public static Degree operator-(Degree a, Degree b)
        {
            return new Degree(a.value - b.value);
        }

        public static Degree operator-(Degree a, Radian r) 
        { 
            return new Degree (a.value - r.Degrees); 
        }

        public static Degree operator*(Degree a, float s)
        {
            return new Degree(a.value * s);
        }

        public static Degree operator*(Degree a, Degree b)
        {
            return new Degree(a.value * b.value);
        }

        public static Degree operator/(Degree a, float s)
        {
            return new Degree(a.value / s);
        }

        public static Degree operator /(Degree a, Degree b)
        {
            return new Degree(a.value / b.value);
        }

        public static bool operator<(Degree a, Degree b)
        {
            return a.value < b.value;
        }

        public static bool operator>(Degree a, Degree b)
        {
            return a.value > b.value;
        }

        public static bool operator<=(Degree a, Degree b)
        {
            return a.value <= b.value;
        }

        public static bool operator>=(Degree a, Degree b)
        {
            return a.value >= b.value;
        }

        public static bool operator==(Degree a, Degree b)
        {
            return a.value == b.value;
        }

        public static bool operator!=(Degree a, Degree b)
        {
            return a.value != b.value;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Degree))
                return false;

            Degree degree = (Degree)other;
            if (value.Equals(degree.value))
                return true;

            return false;
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return value.GetHashCode();
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return value.ToString();
        }
    };

    /** @} */
}
