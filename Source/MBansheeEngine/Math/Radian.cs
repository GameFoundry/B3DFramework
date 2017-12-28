//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    ///	Wrapper class which indicates a given angle value is in radians. Radian values are interchangeable with degree 
    /// values, and conversions will be done automatically between them.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Radian // Note: Must match C++ class Radian
    {
        [SerializeField]
        readonly float value;

        /// <summary>
        /// Creates a new radian value.
        /// </summary>
        /// <param name="value">Value in radians.</param>
        public Radian(float value = 0.0f)
        {
            this.value = value;
        }

        /// <summary>
        /// Creates a new radian value.
        /// </summary>
        /// <param name="d">Value in degrees.</param>
        public Radian(Degree d)
        {
            this.value = d.Radians;
        }

        /// <summary>
        /// Converts a undefined angle value to a radian value.
        /// </summary>
        /// <param name="value">Value in degrees.</param>
        /// <returns>Radian object wrapping the value.</returns>
        public static explicit operator Radian(float value)
        {
            return new Radian(value);
        }

        /// <summary>
        /// Converts a degree angle value to a radian value.
        /// </summary>
        /// <param name="d">Value in degrees.</param>
        /// <returns>Radian object wrapping the value.</returns>
        public static implicit operator Radian(Degree d)
        {
            return new Radian(d.Radians);
        }

        /// <summary>
        /// Converts a radian value to a regular floating point value.
        /// </summary>
        /// <param name="r">Radian value to convert.</param>
        /// <returns>Value in radians as floating point type.</returns>
        public static explicit operator float(Radian r)
        {
            return r.value;
        }

        /// <summary>
        /// Returns the value in degrees as a floating point type.
        /// </summary>
        public float Degrees
        {
            get { return value*MathEx.Rad2Deg; }
        }

        /// <summary>
        /// Returns the value in radians as a floating point type.
        /// </summary>
        public float Radians
        {
            get { return value; }
        }

        public static Radian operator +(Radian a)
        {
            return a;
        }

        public static Radian operator +(Radian a, Radian b)
        {
            return new Radian(a.value + b.value);
        }

        public static Radian operator +(Radian a, Degree d) 
        {
            return new Radian(a.value + d.Radians); 
        }

        public static Radian operator -(Radian a)
        {
            return new Radian(-a.value);
        }

        public static Radian operator -(Radian a, Radian b)
        {
            return new Radian(a.value - b.value);
        }

        public static Radian operator -(Radian a, Degree d) 
        {
            return new Radian(a.value - d.Radians); 
        }

        public static Radian operator *(Radian a, float s)
        {
            return new Radian(a.value * s);
        }

        public static Radian operator *(float s, Radian a)
        {
            return new Radian(a.value * s);
        }

        public static Radian operator *(Radian a, Radian b)
        {
            return new Radian(a.value * b.value);
        }

        public static Radian operator /(Radian a, float s)
        {
            return new Radian(a.value / s);
        }

        public static Radian operator /(float s, Radian a)
        {
            return new Radian(a.value / s);
        }

        public static Radian operator /(Radian a, Radian b)
        {
            return new Radian(a.value / b.value);
        }

        public static bool operator <(Radian a, Radian b)
        {
            return a.value < b.value;
        }

        public static bool operator >(Radian a, Radian b)
        {
            return a.value > b.value;
        }

        public static bool operator <=(Radian a, Radian b)
        {
            return a.value <= b.value;
        }

        public static bool operator >=(Radian a, Radian b)
        {
            return a.value >= b.value;
        }

        public static bool operator ==(Radian a, Radian b)
        {
            return a.value == b.value;
        }

        public static bool operator !=(Radian a, Radian b)
        {
            return a.value != b.value;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Radian))
                return false;

            Radian radian = (Radian)other;
            if (value.Equals(radian.value))
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
    }

    /** @} */
}
