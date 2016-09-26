//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Contains a three-component color with an alpha component.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Color // Note: Must match C++ class Color
    {
        public float r;
        public float g;
        public float b;
        public float a;

        public static Color Red { get { return new Color(1.0f, 0.0f, 0.0f, 1.0f); } }
        public static Color Green { get { return new Color(0.0f, 1.0f, 0.0f, 1.0f); } }
        public static Color Blue { get { return new Color(0.0f, 0.0f, 1.0f, 1.0f); } }
        public static Color Yellow { get { return new Color(1.0f, 1.0f, 0.0f, 1.0f); } }
        public static Color Cyan { get { return new Color(0.0f, 1.0f, 1.0f, 1.0f); } }
        public static Color Magenta { get { return new Color(1.0f, 0.0f, 1.0f, 1.0f); } }
        public static Color White { get { return new Color(1.0f, 1.0f, 1.0f, 1.0f); } }
        public static Color Black { get { return new Color(0.0f, 0.0f, 0.0f, 1.0f); } }
        public static Color DarkCyan { get { return new Color(0.0f, (114.0f / 255.0f), (188.0f / 255.0f), 1.0f); } }
        public static Color VeryDarkGray { get { return new Color(23.0f / 255.0f, 23.0f / 255.0f, 23.0f / 255.0f, 1.0f); } }
        public static Color DarkGray { get { return new Color(63.0f / 255.0f, 63.0f / 255.0f, 63.0f / 255.0f, 1.0f); } }
        public static Color LightGray { get { return new Color(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f); } }
        public static Color BansheeOrange { get { return new Color(1.0f, (168.0f/255.0f), 0.0f, 1.0f); } }
        public static Color Transparent { get { return new Color(0.0f, 0.0f, 0.0f, 0.0f); } }

        /// <summary>
        /// Accesses color components by an index.
        /// </summary>
        /// <param name="index">Index ranging [0, 3]</param>
        /// <returns>Value of the component at the specified index.</returns>
        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0:
                        return r;
                    case 1:
                        return g;
                    case 2:
                        return b;
                    case 3:
                        return a;
                    default:
                        throw new IndexOutOfRangeException("Invalid Color index.");
                }
            }
            set
            {
                switch (index)
                {
                    case 0:
                        r = value;
                        break;
                    case 1:
                        g = value;
                        break;
                    case 2:
                        b = value;
                        break;
                    case 3:
                        a = value;
                        break;
                    default:
                        throw new IndexOutOfRangeException("Invalid Color index.");
                }
            }
        }

        /// <summary>
        /// Creates a new color value.
        /// </summary>
        /// <param name="r">Red component, in range [0, 1].</param>
        /// <param name="g">Green component, in range [0, 1].</param>
        /// <param name="b">Blue component, in range [0, 1].</param>
        /// <param name="a">Alpha component, in range [0, 1].</param>
        public Color(float r, float g, float b, float a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }

        /// <summary>
        /// Creates a new color value. Alpha is assumed to be 1 (non-transparent).
        /// </summary>
        /// <param name="r">Red component, in range [0, 1].</param>
        /// <param name="g">Green component, in range [0, 1].</param>
        /// <param name="b">Blue component, in range [0, 1].</param>
        public Color(float r, float g, float b)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = 1f;
        }

        public static Color operator+ (Color a, Color b)
        {
            return new Color(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a);
        }

        public static Color operator- (Color a, Color b)
        {
            return new Color(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a);
        }

        public static Color operator* (Color a, Color b)
        {
            return new Color(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a);
        }

        public static Color operator* (Color a, float b)
        {
            return new Color(a.r * b, a.g * b, a.b * b, a.a * b);
        }

        public static Color operator* (float b, Color a)
        {
            return new Color(a.r * b, a.g * b, a.b * b, a.a * b);
        }

        public static Color operator/ (Color a, float b)
        {
            return new Color(a.r / b, a.g / b, a.b / b, a.a / b);
        }

        public static bool operator ==(Color lhs, Color rhs)
        {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
        }

        public static bool operator !=(Color lhs, Color rhs)
        {
            return !(lhs == rhs);
        }

        /// <summary>
        /// Converts the provided RGB color into HSV color space.
        /// </summary>
        /// <param name="input">Color in RGB color space.</param>
        /// <returns>Color in HSV color space.</returns>
        public static Color RGB2HSV(Color input)
        {
            Color output = input;

            float min = input.r < input.g ? input.r : input.g;
            min = min < input.b ? min : input.b;

            float max = input.r > input.g ? input.r : input.g;
            max = max > input.b ? max : input.b;

            output.b = max;

            if (max == 0.0f)
            {
                output.r = 0.0f;
                output.g = 0.0f;

                return output;
            }

            float delta = max - min;
            if (delta != 0.0f)
            {
                output.g = (delta / max);
            }
            else
            {
                output.g = 0.0f;
                delta = 1.0f;
            }

            if (input.r >= max)
                output.r = (input.g - input.b) / delta;
            else
            {
                if (input.g >= max)
                    output.r = 2.0f + (input.b - input.r) / delta;
                else
                    output.r = 4.0f + (input.r - input.g) / delta;
            }
            
            output.r /= 6.0f;

            if (output.r < 0.0f)
                output.r += 1.0f;

            return output;
        }

        /// <summary>
        /// Converts the provided HSV color into RGB color space.
        /// </summary>
        /// <param name="input">Color in HSV color space.</param>
        /// <returns>Color in RGB color space.</returns>
        public static Color HSV2RGB(Color input)
        {
            Color output = input;

            if (input.g <= 0.0)
            {
                output.r = input.b;
                output.g = input.b;
                output.b = input.b;

                return output;
            }

            float hh = input.r;
            if (hh >= 1.0f)
                hh = 0.0f;

            hh *= 6.0f;

            int i = (int)hh;
            float ff = hh - i;
            float p = input.b * (1.0f - input.g);
            float q = input.b * (1.0f - (input.g * ff));
            float t = input.b * (1.0f - (input.g * (1.0f - ff)));

            switch (i)
            {
                case 0:
                    output.r = input.b;
                    output.g = t;
                    output.b = p;
                    break;
                case 1:
                    output.r = q;
                    output.g = input.b;
                    output.b = p;
                    break;
                case 2:
                    output.r = p;
                    output.g = input.b;
                    output.b = t;
                    break;
                case 3:
                    output.r = p;
                    output.g = q;
                    output.b = input.b;
                    break;
                case 4:
                    output.r = t;
                    output.g = p;
                    output.b = input.b;
                    break;
                default:
                    output.r = input.b;
                    output.g = p;
                    output.b = q;
                    break;
            }

            return output;
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return r.GetHashCode() ^ g.GetHashCode() << 2 ^ b.GetHashCode() >> 2 ^ a.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is Color))
                return false;

            Color color = (Color)other;
            if (r.Equals(color.r) && g.Equals(color.g) && b.Equals(color.b) && a.Equals(color.a))
                return true;

            return false;
        }
    }

    /** @} */
}
