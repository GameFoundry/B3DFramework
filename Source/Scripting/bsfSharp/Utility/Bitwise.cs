//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;

namespace b3d
{
    /** @addtogroup General
     *  @{
     */

    /// <summary>
    /// Class for manipulating bit patterns.
    /// </summary>
    public class Bitwise
    {
        /// <summary>
        /// Returns the power-of-two number greater or equal to the provided value.
        /// </summary>
        /// <param name="n">Parameter to take power-of-two value of.</param>
        /// <returns>Next power-of-two of the provided value.</returns>
        public static uint NextPow2(uint n)
        {
            --n;
            n |= n >> 16;
            n |= n >> 8;
            n |= n >> 4;
            n |= n >> 2;
            n |= n >> 1;
            ++n;
            return n;
        }

        /// <summary>
        /// Returns the power-of-two number closest to the provided value.
        /// </summary>
        /// <param name="n">Parameter to take power-of-two value of.</param>
        /// <returns>Closest power-of-two of the provided value.</returns>
        public static uint ClosestPow(uint n)
        {
            uint next = NextPow2(n);

            uint prev = next >> 1;
            if (n - prev < next - n)
                return prev;

            return next;
        }

        /// <summary>
        /// Determines whether the number is power-of-two or not.
        /// </summary>
        /// <param name="n">Provided value.</param>
        /// <returns>True if the number is a power of two, false otherwise.</returns>
        public static bool IsPow2(uint n)
        {
            return (n & (n - 1)) == 0;
        }

        /// <summary>
        /// Converts a float in range [-1, 1] into an unsigned 8-bit integer.
        /// </summary>
        /// <param name="v">Float in [-1, 1] range.</param>
        /// <returns>Converted floating point to unsigned 8-bit integer.</returns>
        public static byte Quantize8BitSigned(float v)
        {
            return Quantize8BitUnsigned(v * 0.5f + 0.5f);
        }

        /// <summary>
        /// Converts a float in range [0, 1] into an unsigned 8-bit integer.
        /// </summary>
        /// <param name="v">Float in [0, 1] range.</param>
        /// <returns>Converted floating point to unsigned 8-bit integer.</returns>
        public static byte Quantize8BitUnsigned(float v)
        {
            return (byte)(v * 255.999f);
        }
    }

    /** @} */
}
