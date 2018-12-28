//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsDegree.h"
#include "Math/BsRadian.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Utility class providing common scalar math operations. */
	class BS_UTILITY_EXPORT Math 
	{
	public:
		static constexpr float BIGGEST_FLOAT_SMALLER_THAN_ONE = 0.99999994f;

		/** Inverse cosine. */
		static Radian acos(float val);

		/** Inverse sine. */
		static Radian asin(float val);

		/** Inverse tangent. */
		static Radian atan(float val) { return Radian(std::atan(val)); }

		/** Inverse tangent with two arguments, returns angle between the X axis and the point. */
		static Radian atan2(float y, float x) { return Radian(std::atan2(y,x)); }

		/** Cosine. */
		static float cos(const Radian& val) { return (float)std::cos(val.valueRadians()); }

		/** Cosine. */
		static float cos(float val) { return (float)std::cos(val); }

		/** Sine. */
		static float sin(const Radian& val) { return (float)std::sin(val.valueRadians()); }

		/** Sine. */
		static float sin(float val) { return (float)std::sin(val); }

		/** Tangent. */
		static float tan(const Radian& val) { return (float)std::tan(val.valueRadians()); }

		/** Tangent. */
		static float tan(float val) { return (float)std::tan(val); }

		/** Square root. */
		static float sqrt(float val) { return (float)std::sqrt(val); }

		/** Square root. */
		static Radian sqrt(const Radian& val) { return Radian(std::sqrt(val.valueRadians())); }

		/** Square root. */
		static Degree sqrt(const Degree& val) { return Degree(std::sqrt(val.valueDegrees())); }

		/** Square root followed by an inverse. */
		static float invSqrt(float val);

		/** Returns square of the provided value. */
		static float sqr(float val) { return val*val; }

		/** Returns base raised to the provided power. */
		static float pow(float base, float exponent) { return (float)std::pow(base, exponent); }

		/** Returns euler number (e) raised to the provided power. */
		static float exp(float val) { return (float)std::exp(val); }

		/** Returns natural (base e) logarithm of the provided value. */
		static float log(float val) { return (float)std::log(val); }

		/** Returns base 2 logarithm of the provided value. */
		static float log2(float val) { return (float)(std::log(val)/LOG2); }

		/** Returns base N logarithm of the provided value. */
		static float logN(float base, float val) { return (float)(std::log(val)/std::log(base)); }

		/** Returns the sign of the provided value as 1 or -1. */
		static float sign(float val);

		/** Returns the sign of the provided value as 1 or -1. */
		static Radian sign(const Radian& val) { return Radian(sign(val.valueRadians())); }

		/** Returns the sign of the provided value as 1 or -1. */
		static Degree sign(const Degree& val) { return Degree(sign(val.valueDegrees())); }

		/** Returns the absolute value. */
		static float abs(float val) { return float(std::fabs(val)); }

		/** Returns the absolute value. */
		static Degree abs(const Degree& val) { return Degree(std::fabs(val.valueDegrees())); }

		/** Returns the absolute value. */
		static Radian abs(const Radian& val) { return Radian(std::fabs(val.valueRadians())); }

		/** Returns the nearest integer equal or higher to the provided value. */
		static float ceil(float val) { return (float)std::ceil(val); }

		/** 
		 * Returns the nearest integer equal or higher to the provided value. If you are sure the input is positive use
		 * ceilToPosInt() for a slightly faster operation.
		 */
		static int32_t ceilToInt(float val)
		{
			assert(val >= std::numeric_limits<int32_t>::min() && val <= std::numeric_limits<int32_t>::max());

			// Positive values need offset in order to truncate towards positive infinity (cast truncates towards zero) 
			return val >= 0.0f ? (int32_t)(val + BIGGEST_FLOAT_SMALLER_THAN_ONE) : (int32_t)val;
		}

		/** 
		 * Returns the nearest integer equal or higher to the provided value. Value must be non-negative. Slightly faster
		 * than ceilToInt().
		 */
		static uint32_t ceilToPosInt(float val)
		{
			assert(val >= 0 && val <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)(val + BIGGEST_FLOAT_SMALLER_THAN_ONE);
		}

		/** Returns the integer nearest to the provided value. */
		static float round(float val) { return (float)std::floor(val + 0.5f); }

		/** 
		 * Returns the integer nearest to the provided value. If you are sure the input is positive use roundToPosInt()
		 * for a slightly faster operation.
		 */
		static int32_t roundToInt(float val) { return floorToInt(val + 0.5f); }

		/** 
		 * Returns the integer nearest to the provided value. Value must be non-negative. Slightly faster than roundToInt().
		 */
		static uint32_t roundToPosInt(float val) { return floorToPosInt(val + 0.5f); }

		/** 
		 * Divides an integer by another integer and returns the result, rounded up. Only works if both integers are
		 * positive. 
		 */
		template<class T>
		static constexpr T divideAndRoundUp(T n, T d) { return (n + d - 1) / d; }

		/** Returns the nearest integer equal or lower of the provided value. */
		static float floor(float val) { return (float)std::floor(val); }

		/** 
		 * Returns the nearest integer equal or lower of the provided value. If you are sure the input is positive
		 * use floorToPosInt() for a slightly faster operation.
		 */
		static int floorToInt(float val)
		{
			assert(val >= std::numeric_limits<int32_t>::min() && val <= std::numeric_limits<int32_t>::max());

			// Negative values need offset in order to truncate towards negative infinity (cast truncates towards zero) 
			return val >= 0.0f ? (int32_t)val : (int32_t)(val - BIGGEST_FLOAT_SMALLER_THAN_ONE);
		}

		/** 
		 * Returns the nearest integer equal or lower of the provided value. Value must be non-negative. Slightly faster
		 * than floorToInt(). 
		 */
		static uint32_t floorToPosInt(float val)
		{
			assert(val >= 0 && val <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)val;
		}
		
		/** Rounds @p x to the nearest multiple of @p multiple. */
		static float roundToMultiple(float x, float multiple)
		{
			return floor((x + multiple * 0.5f) / multiple) * multiple;
		}

		/** Clamp a value within an inclusive range. */
		template <typename T>
		static T clamp(T val, T minval, T maxval)
		{
			assert (minval <= maxval && "Invalid clamp range");
			return std::max(std::min(val, maxval), minval);
		}

		/** Clamp a value within an inclusive range [0..1]. */
		template <typename T>
		static T clamp01(T val)
		{
			return std::max(std::min(val, (T)1), (T)0);
		}

		/** Returns the fractional part of a floating point number. */
		static float frac(float val)
		{
			return val - (float)(int32_t)val;
		}

		/** Returns a floating point remainder for (@p val / @p length). */
		static float repeat(float val, float length)
		{
			return val - floor(val / length) * length;
		}

		/** 
		 * Wraps the value in range [0, length) and reverses the direction every @p length increment. This results in
		 * @p val incrementing until @p length, then decrementing back to 0, and so on.
		 */
		static float pingPong(float val, float length)
		{
			val = repeat(val, length * 2.0f);
			return length - fabs(val - length);
		}

		/** Checks if the value is a valid number. */
		static bool isNaN(float f)
		{
			return f != f;
		}

		/** Compare two floats, using tolerance for inaccuracies. */
		static bool approxEquals(float a, float b, 
			float tolerance = std::numeric_limits<float>::epsilon())
		{
			return fabs(b - a) <= tolerance;
		}

		/** Compare two doubles, using tolerance for inaccuracies. */
		static bool approxEquals(double a, double b, 
			double tolerance = std::numeric_limits<double>::epsilon())
		{
			return fabs(b - a) <= tolerance;
		}

		/** Compare two 2D vectors, using tolerance for inaccuracies. */
		static bool approxEquals(const Vector2& a, const Vector2& b, 
			float tolerance = std::numeric_limits<float>::epsilon());

		/** Compare two 3D vectors, using tolerance for inaccuracies. */
		static bool approxEquals(const Vector3& a, const Vector3& b, 
			float tolerance = std::numeric_limits<float>::epsilon());

		/** Compare two 4D vectors, using tolerance for inaccuracies. */
		static bool approxEquals(const Vector4& a, const Vector4& b, 
			float tolerance = std::numeric_limits<float>::epsilon());

		/** Compare two quaternions, using tolerance for inaccuracies. */
		static bool approxEquals(const Quaternion& a, const Quaternion& b, 
			float tolerance = std::numeric_limits<float>::epsilon());

		/** Calculates the tangent space vector for a given set of positions / texture coords. */
		static Vector3 calculateTriTangent(const Vector3& position1, const Vector3& position2, 
			const Vector3& position3, float u1, float v1, float u2, float v2, float u3, float v3);

		/************************************************************************/
		/* 							TRIG APPROXIMATIONS                      	*/
		/************************************************************************/

		/**
		 * Sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastSin0(const Radian& val) { return (float)fastASin0(val.valueRadians()); }

		/**
		 * Sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastSin0(float val);

		/**
		 * Sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastSin0.
		 */
		static float fastSin1(const Radian& val) { return (float)fastASin1(val.valueRadians()); }

		/**
		 * Sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastSin0.
		 */
		static float fastSin1(float val);

		/**
		 * Cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastCos0(const Radian& val) { return (float)fastACos0(val.valueRadians()); }

		/**
		 * Cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastCos0(float val);

		/**
		 * Cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastCos0.
		 */
		static float fastCos1(const Radian& val) { return (float)fastACos1(val.valueRadians()); }

		/**
		 * Cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/2].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastCos0.
		 */
		static float fastCos1(float val);

		/**
		 * Tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/4].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastTan0(const Radian& val) { return (float)fastATan0(val.valueRadians()); }

		/**
		 * Tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/4].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastTan0(float val);

		/**
		 * Tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/4].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastTan0.
		 */
		static float fastTan1(const Radian& val) { return (float)fastATan1(val.valueRadians()); }

		/**
		 * Tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [0, pi/4].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastTan0.
		 */
		static float fastTan1(float val);

		/**
		 * Inverse sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastASin0(const Radian& val) { return (float)fastASin0(val.valueRadians()); }

		/**
		 * Inverse sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastASin0(float val);

		/**
		 * Inverse sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastASin0.
		 */
		static float fastASin1(const Radian& val) { return (float)fastASin1(val.valueRadians()); }

		/**
		 * Inverse sine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastASin0.
		 */
		static float fastASin1(float val);

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastACos0(const Radian& val) { return (float)fastACos0(val.valueRadians()); }

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastACos0(float val);

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastACos0.
		 */
		static float fastACos1(const Radian& val) { return (float)fastACos1(val.valueRadians()); }

		/**
		 * Inverse cosine function approximation.
		 *
		 * @param[in]	val	Angle in range [0, 1].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastACos0.
		 */
		static float fastACos1(float val);

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [-1, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastATan0(const Radian& val) { return (float)fastATan0(val.valueRadians()); }

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [-1, 1].
		 *
		 * @note	Evaluates trigonometric functions using polynomial approximations.
		 */
		static float fastATan0(float val);

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [-1, 1].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastATan0.
		 */
		static float fastATan1(const Radian& val) { return (float)fastATan1(val.valueRadians()); }

		/**
		 * Inverse tangent function approximation.
		 *
		 * @param[in]	val	Angle in range [-1, 1].
		 *
		 * @note	
		 * Evaluates trigonometric functions using polynomial approximations. Slightly better (and slower) than fastATan0.
		 */
		static float fastATan1(float val);

		/**
		 * Linearly interpolates between the two values using @p t. t should be in [0, 1] range, where t = 0 corresponds
		 * to @p min value, while t = 1 corresponds to @p max value.
		 */
		template <typename T>
		static T lerp(float t, T min, T max)
		{
			return (1.0f - t) * min + t * max;
		}
		/**
		 * Determines the position of a value between two other values. Returns 0 if @p value is less or equal than
		 * @p min, 1 if @p value is equal or greater than @p max, and value in range (0, 1) otherwise.
		 */
		template <typename T>
		static float invLerp(T val, T min, T max)
		{
			return clamp01((val - min) / std::max(max - min, 0.0001F));
		}

		/**
		 * Solves the linear equation with the parameters A, B. Returns number of roots found and the roots themselves will
		 * be output in the @p roots array.
		 *
		 * @param[in]	A		First variable.
		 * @param[in]	B		Second variable.
		 * @param[out]	roots	Must be at least size of 1.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static UINT32 solveLinear(T A, T B, T* roots)
		{
			if (!approxEquals(A, (T)0))
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
		 * @param[in]	A		First variable.
		 * @param[in]	B		Second variable.
		 * @param[in]	C		Third variable.
		 * @param[out]	roots	Must be at least size of 2.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static UINT32 solveQuadratic(T A, T B, T C, T* roots)
		{
			if (!approxEquals(A, (T)0))
			{
				T p = B / (2 * A);
				T q = C / A;
				T D = p * p - q;

				if (!approxEquals(D, (T)0))
				{
					if (D < (T)0)
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
				return solveLinear(B, C, roots);
			}
		}

		/**
		 * Solves the cubic equation with the parameters A, B, C, D. Returns number of roots found and the roots themselves 
		 * will be output in the @p roots array.
		 *
		 * @param[in]	A		First variable.
		 * @param[in]	B		Second variable.
		 * @param[in]	C		Third variable.
		 * @param[in]	D		Fourth variable.
		 * @param[out]	roots	Must be at least size of 3.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static UINT32 solveCubic(T A, T B, T C, T D, T* roots)
		{
			static const T THIRD = (1 / (T)3);

			T invA = 1 / A;
			A = B * invA;
			B = C * invA;
			C = D * invA;

			T sqA = A * A;
			T p = THIRD * (-THIRD * sqA + B);
			T q = ((T)0.5) * ((2 / (T)27) * A * sqA - THIRD * A * B + C);

			T cbp = p * p * p;
			D = q * q + cbp;

			UINT32 numRoots = 0;
			if (!approxEquals(D, (T)0))
			{
				if (D < 0.0)
				{
					T phi = THIRD * ::acos(-q / sqrt(-cbp));
					T t = 2 * sqrt(-p);

					roots[0] = t * cos(phi);
					roots[1] = -t * cos(phi + PI * THIRD);
					roots[2] = -t * cos(phi - PI * THIRD);

					numRoots = 3;
				}
				else
				{
					T sqrtD = sqrt(D);
					T u = cbrt(sqrtD + fabs(q));

					if (q > (T)0)
						roots[0] = -u + p / u;
					else
						roots[0] = u - p / u;

					numRoots = 1;
				}
			}
			else
			{
				if (!approxEquals(q, (T)0))
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

			T sub = THIRD * A;
			for (UINT32 i = 0; i < numRoots; i++)
				roots[i] -= sub;

			return numRoots;
		}

		/**
		 * Solves the quartic equation with the parameters A, B, C, D, E. Returns number of roots found and the roots 
		 * themselves will be output in the @p roots array.
		 *
		 * @param[in]	A		First variable.
		 * @param[in]	B		Second variable.
		 * @param[in]	C		Third variable.
		 * @param[in]	D		Fourth variable.
		 * @param[in]	E		Fifth variable.
		 * @param[out]	roots	Must be at least size of 4.
		 *
		 * @note	Only returns real roots.
		 */
		template <typename T>
		static UINT32 solveQuartic(T A, T B, T C, T D, T E, T* roots)
		{
			T invA = 1 / A;
			A = B * invA;
			B = C * invA;
			C = D * invA;
			D = E * invA;

			T sqA = A*A;
			T p = -(3 / (T)8) * sqA + B;
			T q = (1 / (T)8) * sqA * A - (T)0.5 * A * B + C;
			T r = -(3 / (T)256) * sqA * sqA + (1 / (T)16) * sqA * B - (1 / (T)4) * A * C + D;

			UINT32 numRoots = 0;
			if (!approxEquals(r, (T)0))
			{
				T cubicA = 1;
				T cubicB = -(T)0.5 * p ;
				T cubicC = -r;
				T cubicD = (T)0.5 * r * p - (1 / (T)8) * q * q;

				solveCubic(cubicA, cubicB, cubicC, cubicD, roots);
				T z = roots[0];

				T u = z * z - r;
				T v = 2 * z - p;

				if (approxEquals(u, T(0)))
					u = 0;
				else if (u > 0)
					u = sqrt(u);
				else
					return 0;

				if (approxEquals(v, T(0)))
					v = 0;
				else if (v > 0)
					v = sqrt(v);
				else
					return 0;

				T quadraticA = 1;
				T quadraticB = q < 0 ? -v : v;
				T quadraticC = z - u;

				numRoots = solveQuadratic(quadraticA, quadraticB, quadraticC, roots);

				quadraticA = 1;
				quadraticB = q < 0 ? v : -v;
				quadraticC = z + u;

				numRoots += solveQuadratic(quadraticA, quadraticB, quadraticC, roots + numRoots);
			}
			else
			{
				numRoots = solveCubic(q, p, (T)0, (T)1, roots);
				roots[numRoots++] = 0;
			}

			T sub = (1/(T)4) * A;
			for (UINT32 i = 0; i < numRoots; i++)
				roots[i] -= sub;

			return numRoots;
		}

		/**
		 * Evaluates a cubic Hermite curve at a specific point.
		 *
		 * @param[in]	t			Parameter that at which to evaluate the curve, in range [0, 1].
		 * @param[in]	pointA		Starting point (at t=0).
		 * @param[in]	pointB		Ending point (at t=1).
		 * @param[in]	tangentA	Starting tangent (at t=0).
		 * @param[in]	tangentB	Ending tangent (at t = 1).
		 * @return					Evaluated value at @p t.
		 */
		template<class T>
		static T cubicHermite(float t, const T& pointA, const T& pointB, const T& tangentA, const T& tangentB)
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
		 * @param[in]	t			Parameter that at which to evaluate the curve, in range [0, 1].
		 * @param[in]	pointA		Starting point (at t=0).
		 * @param[in]	pointB		Ending point (at t=1).
		 * @param[in]	tangentA	Starting tangent (at t=0).
		 * @param[in]	tangentB	Ending tangent (at t = 1).
		 * @return					Evaluated value at @p t.
		 */
		template<class T>
		static T cubicHermiteD1(float t, const T& pointA, const T& pointB, const T& tangentA, const T& tangentB)
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
		 * @param[in]	pointA			Starting point (at t=0).
		 * @param[in]	pointB			Ending point (at t=1).
		 * @param[in]	tangentA		Starting tangent (at t=0).
		 * @param[in]	tangentB		Ending tangent (at t = 1).
		 * @param[out]	coefficients	Four coefficients for the cubic curve, in order [t^3, t^2, t, 1].
		 */
		template<class T>
		static void cubicHermiteCoefficients(const T& pointA, const T& pointB, const T& tangentA, const T& tangentB, 
			T (&coefficients)[4])
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
		 * @param[in]	pointA			Starting point (at t=0).
		 * @param[in]	pointB			Ending point (at t=length).
		 * @param[in]	tangentA		Starting tangent (at t=0).
		 * @param[in]	tangentB		Ending tangent (at t=length).
		 * @param[in]	length			Maximum value the curve will be evaluated at.
		 * @param[out]	coefficients	Four coefficients for the cubic curve, in order [t^3, t^2, t, 1].
		 */
		template<class T>
		static void cubicHermiteCoefficients(const T& pointA, const T& pointB, const T& tangentA, const T& tangentB, 
			float length, T (&coefficients)[4])
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
		* @param[in]  a				Lower bound.
		* @param[in]  b				Upper bound.
		* @param[in]  order			Order of the function.
		* @param[in]  integrand		Function to integrate.
		* @return					Integrated function.
		*/
		template <typename T>
		static T rombergIntegration(T a, T b, int order, const std::function<T(T)> integrand) 
		{
			T h[order + 1]; 
			T r[order + 1][order + 1];

			for (int i = 1; i < order + 1; ++i)
				h[i] = (b - a) / Math::pow(2, i - 1);

			r[1][1] = h[1] / 2 * (integrand(a) + integrand(b));

			for (int i = 2; i < order + 1; ++i) 
			{
				T coeff = 0;
				for (int k = 1; k <= Math::pow(2, i - 2); ++k)
					coeff += integrand(a + (2 * k - 1) * h[i]);

				r[i][1] = 0.5 * (r[i - 1][1] + h[i - 1] * coeff);
			}

			for (int i = 2; i < order + 1; ++i) 
			{
				for (int j = 2; j <= i; ++j)
					r[i][j] = r[i][j - 1] + (r[i][j - 1] - r[i - 1][j - 1]) / (Math::pow(4, j - 1) - 1);
			}

			return r[order][order];
		}

		/**
		* Calculates the Gaussian Quadrature.
		*
		* @param[in]  a				Lower bound.
		* @param[in]  b				Upper bound.
		* @param[in]  roots			Roots of the function.
		* @param[in]  coefficients  Coefficients of the function.
		* @param[in]  integrand		Function to integrate.
		* @return					Gaussian Quadrature integration.
		*/
		template <typename T>
		static T gaussianQuadrature(T a, T b, T* roots, T* coefficients, const std::function <T(T)>& integrand)
		{
			const T half = (T)0.5;
			const T radius = half * (b - a);
			const T center = half * (b + a);
			T res = (T)0;

			for (UINT32 i = 0; i < sizeof(roots) / sizeof(*roots); ++i)
				res += coefficients[i] * integrand(radius * roots[i] + center);

			res *= radius;

			return res;
		}
		
		static constexpr float POS_INFINITY = std::numeric_limits<float>::infinity();
		static constexpr float NEG_INFINITY = -std::numeric_limits<float>::infinity();
		static constexpr float PI = 3.14159265358979323846f;
		static constexpr float TWO_PI = (float)(2.0f * PI);
		static constexpr float HALF_PI = (float)(0.5f * PI);
		static constexpr float QUARTER_PI = (float)(0.25f * PI);
		static constexpr float INV_PI = (float)(1 / PI);
		static constexpr float INV_HALF_PI = (float)(INV_PI / 2);
		static constexpr float INV_TWO_PI = (float)(2.0f * INV_PI);
		static constexpr float DEG2RAD = PI / 180.0f;
		static constexpr float RAD2DEG = 180.0f / PI;
		static constexpr float SQRT2 = 1.4142135623730951f;
		static constexpr float INV_SQRT2 = (float)(1.0f / SQRT2);
		static const float LOG2;
	};

	/** @} */
}
