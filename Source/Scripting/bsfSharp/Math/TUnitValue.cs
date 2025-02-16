//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Numerics;

namespace bs
{
    /** @addtogroup Math
     *  @{
     */

    /// <summary>
    /// Contains a number value and an associated unit. Used primarily to prevent implicit conversion between numbers of different units.
    /// </summary>
    public partial struct TUnitValue<T, Unit>
            : INumber<TUnitValue<T, Unit>>
        where T : INumber<T>
    {
        public TUnitValue(T value)
        {
            Value = value;
        }

        public static TUnitValue<T, Unit> AdditiveIdentity => new (T.AdditiveIdentity);
        public static TUnitValue<T, Unit> MultiplicativeIdentity => new (T.MultiplicativeIdentity);
        public static TUnitValue<T, Unit> Zero => new (T.Zero);
        public static TUnitValue<T, Unit> One => new (T.One);
        public static int Radix => T.Radix;

        public static explicit operator T(TUnitValue<T, Unit> other) => other.Value;
        public static implicit operator TUnitValue<T, Unit>(T other) => new (other);

        public static TUnitValue<T, Unit> operator +(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => new (left.Value + right.Value);
        public static TUnitValue<T, Unit> operator -(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => new (left.Value - right.Value);
        public static TUnitValue<T, Unit> operator *(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => new (left.Value * right.Value);
        public static TUnitValue<T, Unit> operator /(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => new (left.Value / right.Value);
        public static TUnitValue<T, Unit> operator %(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => new (left.Value % right.Value);
        public static TUnitValue<T, Unit> operator -(TUnitValue<T, Unit> value) => new (-value.Value);
        public static TUnitValue<T, Unit> operator +(TUnitValue<T, Unit> value) => new (value.Value);

        public static bool operator ==(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => left.Value == right.Value;
        public static bool operator !=(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => left.Value != right.Value;
        public static bool operator >(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => left.Value > right.Value;
        public static bool operator >=(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => left.Value >= right.Value;
        public static bool operator <(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => left.Value < right.Value;
        public static bool operator <=(TUnitValue<T, Unit> left, TUnitValue<T, Unit> right) => left.Value <= right.Value;

        public static TUnitValue<T, Unit> operator --(TUnitValue<T, Unit> value)
        {
            T internalValue = value.Value;
            internalValue--;

            return new TUnitValue<T, Unit>(internalValue);
        }

        public static TUnitValue<T, Unit> operator ++(TUnitValue<T, Unit> value)
        {
            T internalValue = value.Value;
            internalValue++;

            return new TUnitValue<T, Unit>(internalValue);
        }

        public int CompareTo(TUnitValue<T, Unit> other) => Value.CompareTo(other.Value);
        public int CompareTo(T other) => Value.CompareTo(other);

        public bool Equals(TUnitValue<T, Unit> other) => Value.Equals(other.Value);
        public bool Equals(T other) => Value.Equals(other);

        public override bool Equals(object other) => other is TUnitValue<T, Unit> otherUnitValue && Equals(otherUnitValue);
        public override int GetHashCode() => EqualityComparer<T>.Default.GetHashCode(Value);

        public string ToString(string format, IFormatProvider formatProvider) => Value.ToString(format, formatProvider);
        public override string ToString() => this.ToString(null, System.Globalization.CultureInfo.CurrentCulture);

        public bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format, IFormatProvider provider) => Value.TryFormat(destination, out charsWritten, format, provider);

        public int CompareTo(object other)
        {
            if (other == null)
            {
                return 1;
            }

            if (other is T otherValue)
            {
                if (Value < otherValue) return -1;
                if (Value > otherValue) return 1;
                if (Value == otherValue) return 0;

                // At least one of the values is NaN.
                if (IsNaN(Value))
                    return IsNaN(otherValue) ? 0 : -1;
                else // f is NaN.
                    return 1;
            }

            throw new ArgumentException();
        }

        public static TUnitValue<T, Unit> Parse(string s, IFormatProvider provider) => new(T.Parse(s, provider));
        public static TUnitValue<T, Unit> Parse(ReadOnlySpan<char> s, NumberStyles style, IFormatProvider provider) => new(T.Parse(s, style, provider));
        public static TUnitValue<T, Unit> Parse(string s, NumberStyles style, IFormatProvider provider) => new(T.Parse(s, style, provider));
        public static TUnitValue<T, Unit> Parse(ReadOnlySpan<char> s, IFormatProvider provider) => new(T.Parse(s, provider));

        public static bool TryParse(string s, IFormatProvider provider, out TUnitValue<T, Unit> result)
        {
            bool returnValue = T.TryParse(s, provider, out var valueResult);

            result = new TUnitValue<T, Unit>(valueResult);
            return returnValue;
        }

        public static bool TryParse(ReadOnlySpan<char> s, IFormatProvider provider, out TUnitValue<T, Unit> result)
        {
            bool returnValue = T.TryParse(s, provider, out var valueResult);

            result = new TUnitValue<T, Unit>(valueResult);
            return returnValue;
        }

        public static bool TryParse(ReadOnlySpan<char> s, NumberStyles style, IFormatProvider provider, out TUnitValue<T, Unit> result)
        {
            bool returnValue = T.TryParse(s, style, provider, out var valueResult);

            result = new TUnitValue<T, Unit>(valueResult);
            return returnValue;
        }

        public static bool TryParse(string s, NumberStyles style, IFormatProvider provider, out TUnitValue<T, Unit> result)
        {
            bool returnValue = T.TryParse(s, style, provider, out var valueResult);

            result = new TUnitValue<T, Unit>(valueResult);
            return returnValue;
        }

        public static TUnitValue<T, Unit> Abs(TUnitValue<T, Unit> value) => new(T.Abs(value.Value));

        public static bool IsCanonical(TUnitValue<T, Unit> value) => T.IsCanonical(value.Value);
        public static bool IsComplexNumber(TUnitValue<T, Unit> value) => T.IsComplexNumber(value.Value);
        public static bool IsEvenInteger(TUnitValue<T, Unit> value) => T.IsEvenInteger(value.Value);
        public static bool IsFinite(TUnitValue<T, Unit> value) => T.IsFinite(value.Value);
        public static bool IsImaginaryNumber(TUnitValue<T, Unit> value) => T.IsImaginaryNumber(value.Value);
        public static bool IsInfinity(TUnitValue<T, Unit> value) => T.IsInfinity(value.Value);
        public static bool IsInteger(TUnitValue<T, Unit> value) => T.IsInteger(value.Value);
        public static bool IsNaN(TUnitValue<T, Unit> value) => T.IsNaN(value.Value);
        public static bool IsNegative(TUnitValue<T, Unit> value) => T.IsNegative(value.Value);
        public static bool IsNegativeInfinity(TUnitValue<T, Unit> value) => T.IsNegativeInfinity(value.Value);
        public static bool IsNormal(TUnitValue<T, Unit> value) => T.IsNormal(value.Value);
        public static bool IsOddInteger(TUnitValue<T, Unit> value) => T.IsOddInteger(value.Value);
        public static bool IsPositive(TUnitValue<T, Unit> value) => T.IsPositive(value.Value);
        public static bool IsPositiveInfinity(TUnitValue<T, Unit> value) => T.IsPositiveInfinity(value.Value);

        public static bool IsRealNumber(TUnitValue<T, Unit> value) => T.IsRealNumber(value.Value);
        public static bool IsSubnormal(TUnitValue<T, Unit> value) => T.IsSubnormal(value.Value);
        public static bool IsZero(TUnitValue<T, Unit> value) => T.IsZero(value.Value);

        public static TUnitValue<T, Unit> MaxMagnitude(TUnitValue<T, Unit> x, TUnitValue<T, Unit> y) => new(T.MaxMagnitude(x.Value, y.Value));
        public static TUnitValue<T, Unit> MaxMagnitudeNumber(TUnitValue<T, Unit> x, TUnitValue<T, Unit> y) => new(T.MaxMagnitudeNumber(x.Value, y.Value));

        public static TUnitValue<T, Unit> MinMagnitude(TUnitValue<T, Unit> x, TUnitValue<T, Unit> y) => new(T.MinMagnitude(x.Value, y.Value));
        public static TUnitValue<T, Unit> MinMagnitudeNumber(TUnitValue<T, Unit> x, TUnitValue<T, Unit> y) => new(T.MinMagnitudeNumber(x.Value, y.Value));

        public static bool TryConvertFromChecked<TOther>(TOther value, out TUnitValue<T, Unit> result) where TOther : INumberBase<TOther>
        {
            bool returnValue;
            T internalValue;
            if (typeof(T) == typeof(TOther))
            {
                returnValue = true;
                internalValue = (T)(object)value;
            }
            else
                returnValue = T.TryConvertFromChecked(value, out internalValue);

            result = new TUnitValue<T, Unit>(internalValue);
            return returnValue;
        }

        public static bool TryConvertFromSaturating<TOther>(TOther value, out TUnitValue<T, Unit> result) where TOther : INumberBase<TOther>
        {
            bool returnValue;
            T internalValue;
            if (typeof(T) == typeof(TOther))
            {
                returnValue = true;
                internalValue = (T)(object)value;
            }
            else
                returnValue = T.TryConvertFromSaturating(value, out internalValue);

            result = new TUnitValue<T, Unit>(internalValue);
            return returnValue;
        }

        public static bool TryConvertFromTruncating<TOther>(TOther value, out TUnitValue<T, Unit> result) where TOther : INumberBase<TOther>
        {
            bool returnValue;
            T internalValue;
            if (typeof(T) == typeof(TOther))
            {
                returnValue = true;
                internalValue = (T)(object)value;
            }
            else
                returnValue = T.TryConvertFromTruncating(value, out internalValue);

            result = new TUnitValue<T, Unit>(internalValue);
            return returnValue;
        }

        public static bool TryConvertToChecked<TOther>(TUnitValue<T, Unit> value, out TOther result) where TOther : INumberBase<TOther>
        {
            if (typeof(TOther) == typeof(T))
            {
                result = (TOther)(object)value.Value;
                return true;
            }

            return T.TryConvertToChecked(value.Value, out result);
        }

        public static bool TryConvertToSaturating<TOther>(TUnitValue<T, Unit> value, out TOther result) where TOther : INumberBase<TOther>
        {
            if (typeof(TOther) == typeof(T))
            {
                result = (TOther)(object)value.Value;
                return true;
            }

            return T.TryConvertToSaturating(value.Value, out result);
        }

        public static bool TryConvertToTruncating<TOther>(TUnitValue<T, Unit> value, out TOther result) where TOther : INumberBase<TOther>
        {
            if (typeof(TOther) == typeof(T))
            {
                result = (TOther)(object)value.Value;
                return true;
            }

            return T.TryConvertToTruncating(value.Value, out result);
        }
    }

    /** @} */
}
