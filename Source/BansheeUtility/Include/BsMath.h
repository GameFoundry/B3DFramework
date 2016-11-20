//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsDegree.h"
#include "BsRadian.h"
#include "BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

    /** Utility class providing common scalar math operations. */
    class BS_UTILITY_EXPORT Math 
    {
    public:
		static Radian acos(float val);
		static Radian asin(float val);
		static Radian atan(float val) { return Radian(std::atan(val)); }
		static Radian atan2(float y, float x) { return Radian(std::atan2(y,x)); }

        static float cos(const Radian& val) { return (float)std::cos(val.valueRadians()); }
        static float cos(float val) { return (float)std::cos(val); }

        static float sin(const Radian& val) { return (float)std::sin(val.valueRadians()); }
        static float sin(float val) { return (float)std::sin(val); }

		static float tan(const Radian& val) { return (float)std::tan(val.valueRadians()); }
		static float tan(float val) { return (float)std::tan(val); }

		static float sqrt(float val) { return (float)std::sqrt(val); }
        static Radian sqrt(const Radian& val) { return Radian(std::sqrt(val.valueRadians())); }
        static Degree sqrt(const Degree& val) { return Degree(std::sqrt(val.valueDegrees())); }

		static float invSqrt(float val);
		static float sqr(float val) { return val*val; }

		static float pow(float base, float exponent) { return (float)std::pow(base, exponent); }
		static float exp(float val) { return (float)std::exp(val); }

		static float log(float val) { return (float)std::log(val); }
		static float log2(float val) { return (float)(std::log(val)/LOG2); }
		static float logN(float base, float val) { return (float)(std::log(val)/std::log(base)); }

		static float sign(float val);
		static Radian sign(const Radian& val) { return Radian(sign(val.valueRadians())); }
		static Degree sign(const Degree& val) { return Degree(sign(val.valueDegrees())); }

		static float abs(float val) { return float(std::fabs(val)); }
		static Degree abs(const Degree& val) { return Degree(std::fabs(val.valueDegrees())); }
		static Radian abs(const Radian& val) { return Radian(std::fabs(val.valueRadians())); }

		static float ceil(float val) { return (float)std::ceil(val); }
		static int ceilToInt(float val) { return (int)std::ceil(val); }

		static float round(float val) { return (float)std::floor(val + 0.5f); }
		static int roundToInt(float val) { return (int)std::floor(val + 0.5f); }

		static float floor(float val) { return (float)std::floor(val); }
		static int floorToInt(float val) { return (int)std::floor(val); }

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

		/** Checks is the specified value a power of two. Only works on integer values. */
		template <typename T>
		static bool isPow2(T val)
		{
			return (val & (val - 1)) == 0;
		}

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
		 * Interpolates between min and max. Returned value is in [0, 1] range where min = 0, max = 1 and 0.5 is 
		 * the average of min and max.
		 */
		template <typename T>
		static float lerp01(T val, T min, T max)
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

        static const float POS_INFINITY;
        static const float NEG_INFINITY;
        static const float PI;
        static const float TWO_PI;
        static const float HALF_PI;
		static const float DEG2RAD;
		static const float RAD2DEG;
		static const float LOG2;
    };

	/** @} */
}
