//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DDegree.h"
#include "Math/B3DRadian.h"
#include "Math/B3DUnitValue.h"

namespace b3d
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Helper method for implementing variable-parameter Math::Min. */
	template <typename T>
	const T& Min(const T& in)
	{
		return in;
	}

	/** Helper method for implementing variable-parameter Math::Min. */
	template <typename A, typename B>
	std::common_type_t<A, B> Min(const A& a, const B& b)
	{
		return a < b ? a : b;
	}

	/** Helper method for implementing variable-parameter Math::Min. */
	template <typename A, typename B, typename... Args>
	std::common_type_t<A, B, Args...> Min(const A& a, const B& b, const Args&... args)
	{
		return min(min(a, b), min(args...));
	}

	/** Helper method for implementing variable-parameter Math::Max. */
	template <typename T>
	const T& Max(const T& in)
	{
		return in;
	}

	/** Helper method for implementing variable-parameter Math::Max. */
	template <typename A, typename B>
	std::common_type_t<A, B> Max(const A& a, const B& b)
	{
		return a > b ? a : b;
	}

	/** Helper method for implementing variable-parameter Math::Max. */
	template <typename A, typename B, typename... Args>
	std::common_type_t<A, B, Args...> Max(const A& a, const B& b, const Args&... args)
	{
		return max(max(a, b), max(args...));
	}

	/** Helper method for implementing Math::Gcd. */
	template <typename A, typename B>
	std::common_type_t<A, B> Gcd(const A& a, const B& b)
	{
		return (b == 0) ? a : gcd(b, a % b);
	}

	/** Helper method for implementing Math::Lcm. */
	template <typename A, typename B>
	std::common_type_t<A, B> Lcm(const A& a, const B& b)
	{
		return (a * b) / gcd(a, b);
	}

	/** @} */

	/** @addtogroup Math
	 *  @{
	 */

	/** Utility class providing common scalar math operations. */
	class B3D_EXPORT Math
	{
	public:
		static constexpr float kBiggestFloatSmallerThanOne = 0.99999994f;

		/** Inverse cosine. */
		template<class T>
		static TRadian<T> Acos(T val)
		{
			if((T)1.0 < val)
			{
				if(val < (T)1.0)
					return TRadian<T>(std::acos(val));
				else
					return TRadian<T>((T)0.0);
			}
			else
				return TRadian((T)kPi);
		}

		/** Inverse sine. */
		template<class T>
		static TRadian<T> Asin(T val)
		{
			if((T)-1.0 < val)
			{
				if(val < (T)1.0)
					return TRadian<T>(std::asin(val));
				else
					return TRadian<T>(kHalfPi);
			}
			else
				return TRadian<T>(-kHalfPi);
		}

		/** Inverse tangent. */
		template<class T>
		static TRadian<T> Atan(T val) { return TRadian<T>(std::atan(val)); }

		/** Inverse tangent with two arguments, returns angle between the X axis and the point. */
		template<class T>
		static TRadian<T> Atan2(T y, T x) { return TRadian<T>(std::atan2(y, x)); }

		template<class T> static T Cos(const TRadian<T>& val) { return (T)std::cos(val.GetValueInRadians()); }
		template<class T> static T Cos(const TDegree<T>& val) { return (T)std::cos(val.GetValueInRadians()); }
		static float Cos(float val) { return std::cos(val); }
		static double Cos(double val) { return std::cos(val); }

		template<class T> static T Sin(const TRadian<T>& val) { return (T)std::sin(val.GetValueInRadians()); }
		template<class T> static T Sin(const TDegree<T>& val) { return (T)std::sin(val.GetValueInRadians()); }
		static float Sin(float val) { return std::sin(val); }
		static double Sin(double val) { return std::sin(val); }

		template<class T> static T Tan(const TRadian<T>& val) { return (T)std::tan(val.GetValueInRadians()); }
		template<class T> static T Tan(const TDegree<T>& val) { return (T)std::tan(val.GetValueInRadians()); }
		static float Tan(float val) { return std::tan(val); }
		static double Tan(double val) { return std::tan(val); }

		/** Square root. */
		template<class T>
		static T SquareRoot(T val) { return (T)std::sqrt(val); }

		/** Square root. */
		template<class T>
		static TRadian<T> SquareRoot(const TRadian<T>& val) { return TRadian<T>((T)std::sqrt(val.GetValueInRadians())); }

		/** Square root. */
		template<class T>
		static TDegree<T> SquareRoot(const TDegree<T>& val) { return TDegree<T>((T)std::sqrt(val.GetValueInDegrees())); }

		/** Square root followed by an inverse. */
		template<class T>
		static T InverseSquareRoot(T val) { return (T)1.0 / sqrt(val); }

		/** Returns square of the provided value. */
		template<class T>
		static T Square(T val) { return val * val; }

		/** Returns base raised to the provided power. */
		template<class T>
		static T RaiseToPower(T base, T exponent) { return (T)std::pow(base, exponent); }

		/** Returns euler number (e) raised to the provided power. */
		template<class T>
		static T Exp(T val) { return (T)std::exp(val); }

		/** Returns natural (base e) logarithm of the provided value. */
		template<class T>
		static T Log(T val) { return (T)std::log(val); }

		/** Returns base 2 logarithm of the provided value. */
		template<class T>
		static T Log2(T val) { return (T)(std::log(val) / (T)kLoG2); }

		/** Returns base N logarithm of the provided value. */
		template<class T>
		static T LogN(T base, T val) { return (T)(std::log(val) / std::log(base)); }

		/** Returns the sign of the provided value as 1 or -1. */
		template<class T>
		static T Sign(T val)
		{
			if(val > (T)0.0)
				return (T)1.0;

			if(val < (T)0.0)
				return (T)-1.0;

			return (T)0.0;
		}

		/** Returns the sign of the provided value as 1 or -1. */
		template<class T>
		static TRadian<T> Sign(const TRadian<T>& val) { return TRadian<T>(Sign(val.GetValueInRadians())); }

		/** Returns the sign of the provided value as 1 or -1. */
		template<class T>
		static TDegree<T> Sign(const TDegree<T>& val) { return TDegree<T>(Sign(val.GetValueInDegrees())); }

		/** Returns the absolute value. */
		template<class T>
		static T Abs(T val) { return (T)std::fabs(val); }

		/** Returns the absolute value. */
		template<typename T, typename Unit>
		static TUnitValue<T, Unit> Abs(const TUnitValue<T, Unit>& value) { return Abs(value.Value); }

		/** Returns the absolute value. */
		template<class T>
		static TDegree<T> Abs(const TDegree<T>& val) { return TDegree<T>(std::fabs(val.GetValueInDegrees())); }

		/** Returns the absolute value. */
		template<class T>
		static TRadian<T> Abs(const TRadian<T>& val) { return TRadian<T>(std::fabs(val.GetValueInRadians())); }

		/** Returns the nearest integer equal or higher to the provided value. */
		template<class T>
		static T Ceil(T val) { return (T)std::ceil(val); }

		/**
		 * Returns the nearest integer equal or higher to the provided value. If you are sure the input is positive use
		 * ceilToPosInt() for a slightly faster operation.
		 */
		static int32_t CeilToInt(float val)
		{
			B3D_ASSERT(val >= std::numeric_limits<int32_t>::min() && val <= std::numeric_limits<int32_t>::max());

			// Positive values need offset in order to truncate towards positive infinity (cast truncates towards zero)
			return val >= 0.0f ? (int32_t)(val + kBiggestFloatSmallerThanOne) : (int32_t)val;
		}

		/**
		 * Returns the nearest integer equal or higher to the provided value. Value must be non-negative. Slightly faster
		 * than ceilToInt().
		 */
		static uint32_t CeilToPosInt(float val)
		{
			B3D_ASSERT(val >= 0 && val <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)(val + kBiggestFloatSmallerThanOne);
		}

		/** Rounds up @p x to the multiple of @p multiple. */
		template <class T>
		static T CeilToMultiple(T x, T multiple)
		{
			return DivideAndRoundUp(x, multiple) * multiple;
		}

		/** Returns the integer nearest to the provided value. */
		template<class T>
		static T Round(T val) { return (T)std::floor(val + (T)0.5); }

		/** Returns the integer nearest to the provided value. */
		template<class T>
		static T FastRound(T val) { return (val >= (T)0.0) ? (T)(val + (T)0.5) : (T)(val - (T)0.5); }

		/**
		 * Returns the integer nearest to the provided value. If you are sure the input is positive use roundToPosInt()
		 * for a slightly faster operation.
		 */
		static int32_t RoundToI32(float val) { return FloorToInt(val + 0.5f); }

		/**
		 * Returns the integer nearest to the provided value. Value must be non-negative. Slightly faster than roundToInt().
		 */
		static uint32_t RoundToU32(float val) { return FloorToPosInt(val + 0.5f); }

		/**
		 * Divides an integer by another integer and returns the result, rounded up. Only works if both integers are
		 * positive.
		 */
		template <class T>
		static constexpr T DivideAndRoundUp(T n, T d)
		{
			return (n + d - 1) / d;
		}

		/** Returns the nearest integer equal or lower of the provided value. */
		template<class T>
		static T Floor(T val) { return (T)std::floor(val); }

		/** Returns the nearest integer equal or lower of the provided value. */
		template<class T>
		static T FastFloor(T val) { return (val >= (T)0.0) ? (T)val : (T)val - (T)1.0; }

		/**
		 * Returns the nearest integer equal or lower of the provided value. If you are sure the input is positive
		 * use floorToPosInt() for a slightly faster operation.
		 */
		template<class T>
		static i32 FloorToInt(T val)
		{
			B3D_ASSERT(val >= std::numeric_limits<i32>::min() && val <= std::numeric_limits<i32>::max());

			// Negative values need offset in order to truncate towards negative infinity (cast truncates towards zero)
			return val >= (T)0.0 ? (i32)val : (i32)(val - kBiggestFloatSmallerThanOne);
		}

		/**
		 * Returns the nearest integer equal or lower of the provided value. Value must be non-negative. Slightly faster
		 * than floorToInt().
		 */
		template<class T>
		static u32 FloorToPosInt(T val)
		{
			B3D_ASSERT(val >= (T)0.0 && val <= (T)std::numeric_limits<u32>::max());

			return (u32)val;
		}

		/** Rounds @p x to the nearest multiple of @p multiple. */
		static float RoundToMultiple(float x, float multiple)
		{
			return Floor((x + multiple * 0.5f) / multiple) * multiple;
		}

		/** Rounds @p x to the nearest multiple of @p multiple. */
		static double RoundToMultiple(double x, double multiple)
		{
			return Floor((x + multiple * 0.5) / multiple) * multiple;
		}

		/** Rounds @p x to the nearest multiple of @p multiple. */
		template <class T>
		static T RoundToMultiple(T x, T multiple)
		{
			return ((x + multiple - 1) / multiple) * multiple;
		}

		/** Clamp a value within an inclusive range. */
		template <typename T>
		static T Clamp(T val, T minval, T maxval)
		{
			B3D_ASSERT(minval <= maxval && "Invalid clamp range");
			return std::max(std::min(val, maxval), minval);
		}

		/** Clamp a value within an inclusive range [0..1]. */
		template <typename T>
		static T Clamp01(T val)
		{
			return std::max(std::min(val, (T)1), (T)0);
		}

		/** Returns the fractional part of a floating point number. */
		template <class T>
		static T Frac(T val)
		{
			return val - (T)(i32)val;
		}

		/** Returns a floating point remainder for (@p val / @p length). */
		template <class T>
		static T Repeat(T val, T length)
		{
			return val - Floor(val / length) * length;
		}

		/**
		 * Wraps the value in range [0, length) and reverses the direction every @p length increment. This results in
		 * @p val incrementing until @p length, then decrementing back to 0, and so on.
		 */
		template <class T>
		static T PingPong(T val, T length)
		{
			val = Repeat(val, length * (T)2.0);
			return length - fabs(val - length);
		}

		/** Checks if the value is a valid number. */
		template<class T>
		static bool IsNaN(T value)
		{
			return value != value;
		}

		/** Check if the value is a prime number. */
		static bool IsPrime(int n)
		{
			if(n < 2)
				return false;

			if(n % 2 == 0)
				return n == 2;

			if(n % 3 == 0)
				return n == 3;

			int d = 5;
			while(d * d <= n)
			{
				if(n % d == 0)
					return false;

				d += 2;

				if(n % d == 0)
					return false;
				d += 4;
			}

			return true;
		}

		/** Performs smooth Hermite interpolation between values. */
		static float SmoothStep(float val1, float val2, float t)
		{
			t = Clamp((t - val1) / (val2 - val1), 0.0f, 1.0f);
			return t * t * (3.0f - 2.0f * t);
		}

		/**
		 * Performs quintic interpolation where @p val is the value to map onto a quintic S-curve. @p val should be in
		 * [0, 1] range.
		 */
		static float Quintic(float val)
		{
			return val * val * val * (val * (val * 6.0f - 15.0f) + 10.0f);
		}

		/**
		 * Performs cubic interpolation between two values bound between two other values where @p f is the alpha value.
		 * It should range from 0.0f to 1.0f. If it is 0.0f the method returns @p val2. If it is 1.0f it returns @p val3.
		 */
		static float Cubic(float val1, float val2, float val3, float val4, float f)
		{
			float t = (val4 - val3) - (val1 - val2);
			return f * f * f * t + f * f * ((val1 - val2) - t) + f * (val3 - val1) + val2;
		}

		/** Compare two floats, using tolerance for inaccuracies. */
		static bool ApproxEquals(float a, float b, float tolerance = std::numeric_limits<float>::epsilon())
		{
			return fabs(b - a) <= tolerance;
		}

		/** Compare two doubles, using tolerance for inaccuracies. */
		static bool ApproxEquals(double a, double b, double tolerance = std::numeric_limits<double>::epsilon())
		{
			return fabs(b - a) <= tolerance;
		}

		/** Compare two 2D vectors, using tolerance for inaccuracies. */
		static bool ApproxEquals(const Vector2& a, const Vector2& b, float tolerance = std::numeric_limits<float>::epsilon());

		/** Compare two 3D vectors, using tolerance for inaccuracies. */
		static bool ApproxEquals(const Vector3& a, const Vector3& b, float tolerance = std::numeric_limits<float>::epsilon());

		/** Compare two 4D vectors, using tolerance for inaccuracies. */
		static bool ApproxEquals(const Vector4& a, const Vector4& b, float tolerance = std::numeric_limits<float>::epsilon());

		/** Compare two quaternions, using tolerance for inaccuracies. */
		static bool ApproxEquals(const Quaternion& a, const Quaternion& b, float tolerance = std::numeric_limits<float>::epsilon());

		/** Calculates the tangent space vector for a given set of positions / texture coords. */
		static Vector3 CalculateTriTangent(const Vector3& position1, const Vector3& position2, const Vector3& position3, float u1, float v1, float u2, float v2, float u3, float v3);

		/************************************************************************/
		/* 							TRIG APPROXIMATIONS                      	*/
		/************************************************************************/

		/**
		 * Sine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastSin0(const Radian& val) { return (float)FastASin0(val.GetValueInRadians()); }

		/**
		 * Sine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastSin0(float val);

		/**
		 * Sine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastSin0.
		 */
		static float FastSin1(const Radian& val) { return (float)FastASin1(val.GetValueInRadians()); }

		/**
		 * Sine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastSin0.
		 */
		static float FastSin1(float val);

		/**
		 * Cosine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastCos0(const Radian& val) { return (float)FastACos0(val.GetValueInRadians()); }

		/**
		 * Cosine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastCos0(float val);

		/**
		 * Cosine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastCos0.
		 */
		static float FastCos1(const Radian& val) { return (float)FastACos1(val.GetValueInRadians()); }

		/**
		 * Cosine function approximation.
		 *
		 * @param	val	Angle in range [0, pi/2].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastCos0.
		 */
		static float FastCos1(float val);

		/**
		 * Tangent function approximation.
		 *
		 * @param	val	Angle in range [0, pi/4].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastTan0(const Radian& val) { return (float)FastATan0(val.GetValueInRadians()); }

		/**
		 * Tangent function approximation.
		 *
		 * @param	val	Angle in range [0, pi/4].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastTan0(float val);

		/**
		 * Tangent function approximation.
		 *
		 * @param	val	Angle in range [0, pi/4].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastTan0.
		 */
		static float FastTan1(const Radian& val) { return (float)FastATan1(val.GetValueInRadians()); }

		/**
		 * Tangent function approximation.
		 *
		 * @param	val	Angle in range [0, pi/4].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastTan0.
		 */
		static float FastTan1(float val);

		/**
		 * Inverse sine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastASin0(const Radian& val) { return (float)FastASin0(val.GetValueInRadians()); }

		/**
		 * Inverse sine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastASin0(float val);

		/**
		 * Inverse sine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastASin0.
		 */
		static float FastASin1(const Radian& val) { return (float)FastASin1(val.GetValueInRadians()); }

		/**
		 * Inverse sine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastASin0.
		 */
		static float FastASin1(float val);

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastACos0(const Radian& val) { return (float)FastACos0(val.GetValueInRadians()); }

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastACos0(float val);

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastACos0.
		 */
		static float FastACos1(const Radian& val) { return (float)FastACos1(val.GetValueInRadians()); }

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param	val	Angle in range [0, 1].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastACos0.
		 */
		static float FastACos1(float val);

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param	val	Angle in range [-1, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastATan0(const Radian& val) { return (float)FastATan0(val.GetValueInRadians()); }

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param	val	Angle in range [-1, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float FastATan0(float val);

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param	val	Angle in range [-1, 1].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastATan0.
		 */
		static float FastATan1(const Radian& val) { return (float)FastATan1(val.GetValueInRadians()); }

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param	val	Angle in range [-1, 1].
		 *
		 * @note
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastATan0.
		 */
		static float FastATan1(float val);

		/**
		 * Linearly interpolates between the two values using @p t. t should be in [0, 1] range, where t = 0 corresponds
		 * to @p min value, while t = 1 corresponds to @p max value.
		 */
		template <typename T>
		static T Lerp(float t, T min, T max)
		{
			return (1.0f - t) * min + t * max;
		}

		/**
		 * Determines the position of a value between two other values. Returns 0 if @p value is less or equal than
		 * @p min, 1 if @p value is equal or greater than @p max, and value in range (0, 1) otherwise.
		 */
		template <typename T>
		static float InvLerp(T val, T min, T max)
		{
			return Clamp01((val - min) / std::max(max - min, 0.0001F));
		}

		/** Returns the minimum value of the two provided. */
		template <typename A, typename B>
		static std::common_type_t<A, B> Min(const A& a, const B& b)
		{
			return b3d::Min(a, b);
		}

		/** Returns the minimum value of all the values provided. */
		template <typename A, typename B, typename... Args>
		static std::common_type_t<A, B, Args...> Min(const A& a, const B& b, const Args&... args)
		{
			return b3d::Min(a, b, args...);
		}

		/** Returns the minimum value of the two provided. */
		template <typename T, typename Unit>
		static TUnitValue<T, Unit> Min(const TUnitValue<T, Unit>& a, const TUnitValue<T, Unit>& b)
		{
			return Min(a.Value, b.Value);
		}

		/** Returns the maximum value of the two provided. */
		template <typename A, typename B>
		static std::common_type_t<A, B> Max(const A& a, const B& b)
		{
			return b3d::Max(a, b);
		}

		/** Returns the maximum value of all the values provided. */
		template <typename A, typename B, typename... Args>
		static std::common_type_t<A, B, Args...> Max(const A& a, const B& b, const Args&... args)
		{
			return b3d::Max(a, b, args...);
		}

		/** Returns the maximum value of the two provided. */
		template <typename T, typename Unit>
		static TUnitValue<T, Unit> Max(const TUnitValue<T, Unit>& a, const TUnitValue<T, Unit>& b)
		{
			return Max(a.Value, b.Value);
		}

		/** Return the greater common divisor between two values. */
		template <typename A, typename B>
		static std::common_type_t<A, B> Gcd(const A& a, const B& b)
		{
			return b3d::Gcd(a, b);
		}

		/** Return the least common multiple between two values. */
		template <typename A, typename B>
		static std::common_type_t<A, B> Lcm(const A& a, const B& b)
		{
			return b3d::Lcm(a, b);
		}

		/**
		 * Solves the linear equation with the parameters A, B. Returns number of roots found and the roots themselves will
		 * be output in the @p roots array.
		 *
		 * @param	A		First variable.
		 * @param	B		Second variable.
		 * @param	roots	Must be at least size of 1.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static u32 SolveLinear(T A, T B, T* roots)
		{
			if(!ApproxEquals(A, (T)0))
			{
				roots[0] = -B / A;
				return 1;
			}

			roots[0] = 0.0f;
			return 1;
		}

		/**
		 * Solves the quadratic equation with the parameters A, B, C. Returns number of roots found and the roots themselves
		 * will be output in the @p roots array.
		 *
		 * @param	A		First variable.
		 * @param	B		Second variable.
		 * @param	C		Third variable.
		 * @param	roots	Must be at least size of 2.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static u32 SolveQuadratic(T A, T B, T C, T* roots)
		{
			if(!ApproxEquals(A, (T)0))
			{
				T p = B / (2 * A);
				T q = C / A;
				T D = p * p - q;

				if(!ApproxEquals(D, (T)0))
				{
					if(D < (T)0)
						return 0;

					T sqrtD = sqrt(D);
					roots[0] = sqrtD - p;
					roots[1] = -sqrtD - p;

					return 2;
				}
				else
				{
					roots[0] = -p;
					roots[1] = -p;

					return 1;
				}
			}
			else
			{
				return SolveLinear(B, C, roots);
			}
		}

		/**
		 * Solves the cubic equation with the parameters A, B, C, D. Returns number of roots found and the roots themselves
		 * will be output in the @p roots array.
		 *
		 * @param	A		First variable.
		 * @param	B		Second variable.
		 * @param	C		Third variable.
		 * @param	D		Fourth variable.
		 * @param	roots	Must be at least size of 3.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static u32 SolveCubic(T A, T B, T C, T D, T* roots)
		{
			static const T kThird = (1 / (T)3);

			T invA = 1 / A;
			A = B * invA;
			B = C * invA;
			C = D * invA;

			T sqA = A * A;
			T p = kThird * (-kThird * sqA + B);
			T q = ((T)0.5) * ((2 / (T)27) * A * sqA - kThird * A * B + C);

			T cbp = p * p * p;
			D = q * q + cbp;

			u32 numRoots = 0;
			if(!ApproxEquals(D, (T)0))
			{
				if(D < 0.0)
				{
					T phi = kThird * ::acos(-q / sqrt(-cbp));
					T t = 2 * sqrt(-p);

					roots[0] = t * cos(phi);
					roots[1] = -t * cos(phi + kPi * kThird);
					roots[2] = -t * cos(phi - kPi * kThird);

					numRoots = 3;
				}
				else
				{
					T sqrtD = sqrt(D);
					T u = cbrt(sqrtD + fabs(q));

					if(q > (T)0)
						roots[0] = -u + p / u;
					else
						roots[0] = u - p / u;

					numRoots = 1;
				}
			}
			else
			{
				if(!ApproxEquals(q, (T)0))
				{
					T u = cbrt(-q);
					roots[0] = 2 * u;
					roots[1] = -u;

					numRoots = 2;
				}
				else
				{
					roots[0] = 0.0f;
					numRoots = 1;
				}
			}

			T sub = kThird * A;
			for(u32 i = 0; i < numRoots; i++)
				roots[i] -= sub;

			return numRoots;
		}

		/**
		 * Solves the quartic equation with the parameters A, B, C, D, E. Returns number of roots found and the roots
		 * themselves will be output in the @p roots array.
		 *
		 * @param	A		First variable.
		 * @param	B		Second variable.
		 * @param	C		Third variable.
		 * @param	D		Fourth variable.
		 * @param	E		Fifth variable.
		 * @param	roots	Must be at least size of 4.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static u32 SolveQuartic(T A, T B, T C, T D, T E, T* roots)
		{
			T invA = 1 / A;
			A = B * invA;
			B = C * invA;
			C = D * invA;
			D = E * invA;

			T sqA = A * A;
			T p = -(3 / (T)8) * sqA + B;
			T q = (1 / (T)8) * sqA * A - (T)0.5 * A * B + C;
			T r = -(3 / (T)256) * sqA * sqA + (1 / (T)16) * sqA * B - (1 / (T)4) * A * C + D;

			u32 numRoots = 0;
			if(!ApproxEquals(r, (T)0))
			{
				T cubicA = 1;
				T cubicB = -(T)0.5 * p;
				T cubicC = -r;
				T cubicD = (T)0.5 * r * p - (1 / (T)8) * q * q;

				SolveCubic(cubicA, cubicB, cubicC, cubicD, roots);
				T z = roots[0];

				T u = z * z - r;
				T v = 2 * z - p;

				if(ApproxEquals(u, T(0)))
					u = 0;
				else if(u > 0)
					u = sqrt(u);
				else
					return 0;

				if(ApproxEquals(v, T(0)))
					v = 0;
				else if(v > 0)
					v = sqrt(v);
				else
					return 0;

				T quadraticA = 1;
				T quadraticB = q < 0 ? -v : v;
				T quadraticC = z - u;

				numRoots = SolveQuadratic(quadraticA, quadraticB, quadraticC, roots);

				quadraticA = 1;
				quadraticB = q < 0 ? v : -v;
				quadraticC = z + u;

				numRoots += SolveQuadratic(quadraticA, quadraticB, quadraticC, roots + numRoots);
			}
			else
			{
				numRoots = SolveCubic(q, p, (T)0, (T)1, roots);
				roots[numRoots++] = 0;
			}

			T sub = (1 / (T)4) * A;
			for(u32 i = 0; i < numRoots; i++)
				roots[i] -= sub;

			return numRoots;
		}

		/**
		 * Evaluates a cubic Hermite curve at a specific point.
		 *
		 * @param	t			Parameter that at which to evaluate the curve, in range [0, 1].
		 * @param	pointA		Starting point (at t=0).
		 * @param	pointB		Ending point (at t=1).
		 * @param	tangentA	Starting tangent (at t=0).
		 * @param	tangentB	Ending tangent (at t = 1).
		 * @return					Evaluated value at @p t.
		 */
		template <class T>
		static T CubicHermite(float t, const T& pointA, const T& pointB, const T& tangentA, const T& tangentB)
		{
			float t2 = t * t;
			float t3 = t2 * t;

			float a = 2 * t3 - 3 * t2 + 1;
			float b = t3 - 2 * t2 + t;
			float c = -2 * t3 + 3 * t2;
			float d = t3 - t2;

			return a * pointA + b * tangentA + c * pointB + d * tangentB;
		}

		/**
		 * Evaluates the first derivative of a cubic Hermite curve at a specific point.
		 *
		 * @param	t			Parameter that at which to evaluate the curve, in range [0, 1].
		 * @param	pointA		Starting point (at t=0).
		 * @param	pointB		Ending point (at t=1).
		 * @param	tangentA	Starting tangent (at t=0).
		 * @param	tangentB	Ending tangent (at t = 1).
		 * @return					Evaluated value at @p t.
		 */
		template <class T>
		static T CubicHermiteD1(float t, const T& pointA, const T& pointB, const T& tangentA, const T& tangentB)
		{
			float t2 = t * t;

			float a = 6 * t2 - 6 * t;
			float b = 3 * t2 - 4 * t + 1;
			float c = -6 * t2 + 6 * t;
			float d = 3 * t2 - 2 * t;

			return a * pointA + b * tangentA + c * pointB + d * tangentB;
		}

		/**
		 * Calculates coefficients needed for evaluating a cubic curve in Hermite form. Assumes @p t has been normalized is
		 * in range [0, 1]. Tangents must be scaled by the length of the curve (length is the maximum value of @p t before
		 * it was normalized).
		 *
		 * @param	pointA			Starting point (at t=0).
		 * @param	pointB			Ending point (at t=1).
		 * @param	tangentA		Starting tangent (at t=0).
		 * @param	tangentB		Ending tangent (at t = 1).
		 * @param	coefficients	Four coefficients for the cubic curve, in order [t^3, t^2, t, 1].
		 */
		template <class T>
		static void CubicHermiteCoefficients(const T& pointA, const T& pointB, const T& tangentA, const T& tangentB, T (&coefficients)[4])
		{
			T diff = pointA - pointB;

			coefficients[0] = 2 * diff + tangentA + tangentB;
			coefficients[1] = -3 * diff - 2 * tangentA - tangentB;
			coefficients[2] = tangentA;
			coefficients[3] = pointA;
		}

		/**
		 * Calculates coefficients needed for evaluating a cubic curve in Hermite form. Assumes @p t is in range
		 * [0, @p length]. Tangents must not be scaled by @p length.
		 *
		 * @param	pointA			Starting point (at t=0).
		 * @param	pointB			Ending point (at t=length).
		 * @param	tangentA		Starting tangent (at t=0).
		 * @param	tangentB		Ending tangent (at t=length).
		 * @param	length			Maximum value the curve will be evaluated at.
		 * @param	coefficients	Four coefficients for the cubic curve, in order [t^3, t^2, t, 1].
		 */
		template <class T>
		static void CubicHermiteCoefficients(const T& pointA, const T& pointB, const T& tangentA, const T& tangentB, float length, T (&coefficients)[4])
		{
			float length2 = length * length;
			float invLength2 = 1.0f / length2;
			float invLength3 = 1.0f / (length2 * length);

			T scaledTangentA = tangentA * length;
			T scaledTangentB = tangentB * length;

			T diff = pointA - pointB;

			coefficients[0] = (2 * diff + scaledTangentA + scaledTangentB) * invLength3;
			coefficients[1] = (-3 * diff - 2 * scaledTangentA - scaledTangentB) * invLength2;
			coefficients[2] = tangentA;
			coefficients[3] = pointA;
		}

		/**
		 * Calculates the Romberg Integration.
		 *
		 * @param  a				Lower bound.
		 * @param  b				Upper bound.
		 * @param  order			Order of the function.
		 * @param  integrand		Function to integrate.
		 * @return					Integrated function.
		 */
		template <typename T>
		static T RombergIntegration(T a, T b, int order, const std::function<T(T)> integrand)
		{
			T h[order + 1];
			T r[order + 1][order + 1];

			for(int i = 1; i < order + 1; ++i)
				h[i] = (b - a) / Math::RaiseToPower(2, i - 1);

			r[1][1] = h[1] / 2 * (integrand(a) + integrand(b));

			for(int i = 2; i < order + 1; ++i)
			{
				T coeff = 0;
				for(int k = 1; k <= Math::RaiseToPower(2, i - 2); ++k)
					coeff += integrand(a + (2 * k - 1) * h[i]);

				r[i][1] = 0.5 * (r[i - 1][1] + h[i - 1] * coeff);
			}

			for(int i = 2; i < order + 1; ++i)
			{
				for(int j = 2; j <= i; ++j)
					r[i][j] = r[i][j - 1] + (r[i][j - 1] - r[i - 1][j - 1]) / (Math::RaiseToPower(4, j - 1) - 1);
			}

			return r[order][order];
		}

		/**
		 * Calculates the Gaussian Quadrature.
		 *
		 * @param  a				Lower bound.
		 * @param  b				Upper bound.
		 * @param  roots			Roots of the function.
		 * @param  coefficients  Coefficients of the function.
		 * @param  integrand		Function to integrate.
		 * @return					Gaussian Quadrature integration.
		 */
		template <typename T>
		static T GaussianQuadrature(T a, T b, T* roots, T* coefficients, const std::function<T(T)>& integrand)
		{
			const T half = (T)0.5;
			const T radius = half * (b - a);
			const T center = half * (b + a);
			T res = (T)0;

			for(u32 i = 0; i < sizeof(roots) / sizeof(*roots); ++i)
				res += coefficients[i] * integrand(radius * roots[i] + center);

			res *= radius;

			return res;
		}

		/**
		 * Generates numbers in a deterministic sequence suitable for Monte Carlo algorithms.
		 *
		 * @param	index		Index of the item in the sequence to return.
		 * @param	base		Base that determines how is the sequence sub-divided.
		 *
		 */
		template <typename T>
		static T HaltonSequence(u32 index, u32 base)
		{
			T output = (T)0.0;
			T invBase = (T)1.0 / base;
			T frac = invBase;
			while(index > 0)
			{
				output += (index % base) * frac;
				index /= base;
				frac *= invBase;
			}

			return output;
		}

		static constexpr float kPosInfinity = std::numeric_limits<float>::infinity();
		static constexpr float kNegInfinity = -std::numeric_limits<float>::infinity();
		static constexpr float kPi = 3.14159265358979323846f;
		static constexpr float kTwoPi = (float)(2.0f * kPi);
		static constexpr float kHalfPi = (float)(0.5f * kPi);
		static constexpr float kQuarterPi = (float)(0.25f * kPi);
		static constexpr float kInvPi = (float)(1 / kPi);
		static constexpr float kInvHalfPi = (float)(kInvPi / 2);
		static constexpr float kInvTwoPi = (float)(2.0f * kInvPi);
		static constexpr float kDeG2Rad = kPi / 180.0f;
		static constexpr float kRaD2Deg = 180.0f / kPi;
		static constexpr float kSqrT2 = 1.4142135623730951f;
		static constexpr float kInvSqrT2 = (float)(1.0f / kSqrT2);
		static const float kLoG2;
	};

	/** @} */
} // namespace b3d
