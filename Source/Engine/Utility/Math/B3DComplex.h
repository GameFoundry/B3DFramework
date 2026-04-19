//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Complex numbers. */
	template <class Type>
	class Complex
	{
	public:
		Complex() = default;

		Complex(const Type& r, const Type& i)
			: mReal(r), mImag(i) {}

		Complex(const Complex& other)
			: mReal(other.Real()), mImag(other.Imag()) {}

		Complex<Type>& operator=(const Type& other)
		{
			mReal = other;
			mImag = Type();

			return *this;
		}

		Complex<Type>& operator+=(const Type& other)
		{
			mReal += other;

			return *this;
		}

		Complex<Type>& operator-=(const Type& other)
		{
			mReal -= other;

			return *this;
		}

		Complex<Type>& operator*=(const Type& other)
		{
			mReal *= other;
			mImag *= other;

			return *this;
		}

		Complex<Type>& operator/=(const Type& other)
		{
			mReal /= other;
			mImag /= other;

			return *this;
		}

		Complex<Type>& operator=(const Complex<Type>& other)
		{
			mReal = other.Real();
			mImag = other.Imag();

			return *this;
		}

		Complex<Type>& operator+=(const Complex<Type>& other)
		{
			mReal += other.Real();
			mImag += other.Imag();

			return *this;
		}

		Complex<Type>& operator-=(const Complex<Type>& other)
		{
			mReal -= other.Real();
			mImag -= other.Imag();

			return *this;
		}

		Complex<Type>& operator*=(const Complex<Type>& other)
		{
			const Type r = mReal * other.Real() - mImag * other.Imag();
			mImag = mReal * other.Imag() + mImag * other.Real();
			mReal = r;

			return *this;
		}

		Complex<Type>& operator/=(const Complex<Type>& other)
		{
			const Type r = mReal * other.Real() + mImag * other.Imag();
			const Type n = Complex::Norm(other);
			mImag = (mImag * other.Real() - mReal * other.Imag()) / n;
			mReal = r / n;

			return *this;
		}

		Complex<Type> operator+(const Type& other) const
		{
			return Complex(mReal + other, mImag);
		}

		Complex<Type> operator-(const Type& other) const
		{
			return Complex(mReal - other, mImag);
		}

		Complex<Type> operator*(const Type& other) const
		{
			return Complex(mReal * other, mImag);
		}

		Complex<Type> operator/(const Type& other) const
		{
			return Complex(mReal / other, mImag);
		}

		Complex<Type> operator+(const Complex<Type>& other) const
		{
			return Complex(mReal + other.Real(), mImag + other.Imag());
		}

		Complex<Type> operator-(const Complex<Type>& other) const
		{
			return Complex(mReal - other.Real(), mImag - other.Imag());
		}

		Complex<Type> operator*(const Complex<Type>& other) const
		{
			Complex<Type> res = *this;

			res *= other;

			return res;
		}

		Complex<Type> operator/(const Complex<Type>& other) const
		{
			Complex<Type> res = *this;

			res /= other;

			return res;
		}

		bool operator==(const Complex<Type>& other) const
		{
			return mReal == other.Real() && mImag == other.Imag();
		}

		bool operator==(const Type& other) const
		{
			return mReal == other && mImag == Type();
		}

		bool operator!=(const Complex<Type>& other) const
		{
			return mReal != other.Real() || mImag != other.Imag();
		}

		bool operator!=(const Type& other) const
		{
			return mReal != other || mImag != Type();
		}

		Type& Real() { return mReal; }

		Type& Imag() { return mImag; }

		const Type& Real() const { return mReal; }

		const Type& Imag() const { return mImag; }

		static Type Abs(const Complex<Type>& other)
		{
			Type x = other.Real();
			Type y = other.Imag();
			const Type s = std::max(std::abs(x), std::abs(y));
			if(s == Type())
				return s;

			x /= s;
			y /= s;

			return s * std::sqrt(x * x + y * y);
		}

		static Type Arg(const Complex<Type>& other)
		{
			return std::atan2(other.Imag(), other.Real());
		}

		static Type Norm(const Complex<Type>& other)
		{
			const Type x = other.Real();
			const Type y = other.Imag();

			return x * x + y * y;
		}

		static Complex<Type> Conj(const Complex<Type>& other)
		{
			return Complex(other.Real(), -other.Imag());
		}

		static Complex<Type> Polar(const Type& r, const Type& t = 0)
		{
			return Complex(r * std::cos(t), r * std::sin(t));
		}

		static Complex<Type> Cos(const Complex<Type>& other)
		{
			const Type x = other.Real();
			const Type y = other.Imag();

			return Complex(std::cos(x) * std::cosh(y), -std::sin(x) * std::sinh(y));
		}

		static Complex<Type> Cosh(const Complex<Type>& other)
		{
			const Type x = other.Real();
			const Type y = other.Imag();

			return Complex(std::cosh(x) * std::cos(y), std::sinh(x) * std::sin(y));
		}

		static Complex<Type> Exp(const Complex<Type>& other)
		{
			return Complex::Polar(std::exp(other.Real()), other.Imag());
		}

		static Complex<Type> Log(const Complex<Type>& other)
		{
			return Complex(std::log(Complex::Abs(other)), Complex::Arg(other));
		}

		static Complex<Type> Log10(const Complex<Type>& other)
		{
			return Complex::Log(other) / std::log(Type(10));
		}

		static Complex<Type> Pow(const Complex<Type>& other, const Type& i)
		{
			if(other.Imag() == Type() && other.Real() > Type())
				return Complex(std::pow(other.Real(), i), other.Imag());

			Complex<Type> t = Complex::Log(other);
			return Complex::Polar(std::exp(i * t.Real()), i * t.Imag());
		}

		static Complex<Type> Pow(const Complex<Type>& x, const Complex<Type>& y)
		{
			return Complex::Exp(y * Complex::Log(x));
		}

		static Complex<Type> Pow(const Type& i, const Complex<Type>& other)
		{
			return i > Type() ? Complex::Polar(std::pow(i, other.Real()), other.Imag() * std::log(i))
							  : Complex::Pow(Complex(i, Type()), other);
		}

		static Complex<Type> Sin(const Complex<Type>& other)
		{
			const Type x = other.Real();
			const Type y = other.Imag();

			return Complex(std::sin(x) * std::cosh(y), std::cos(x) * std::sinh(y));
		}

		static Complex<Type> Sinh(const Complex<Type>& other)
		{
			const Type x = other.Real();
			const Type y = other.Imag();

			return Complex(std::sinh(x) * std::cos(y), std::cosh(x) * std::sin(y));
		}

		static Complex<Type> Sqrt(const Complex<Type>& other)
		{
			const Type x = other.Real();
			const Type y = other.Imag();

			if(x == Type())
			{
				Type t = std::sqrt(std::abs(y) / 2);
				return Complex(t, y < Type() ? -t : t);
			}
			else
			{
				Type t = std::sqrt(2 * (Complex::Abs(other) + std::abs(x)));
				Type u = t / 2;
				return x > Type() ? Complex(u, y / t)
								  : Complex(std::abs(y) / t, y < Type() ? -u : u);
			}
		}

		static Complex<Type> Tan(const Complex<Type>& other)
		{
			return Complex::Sin(other) / Complex::Cos(other);
		}

		static Complex<Type> Tanh(const Complex<Type>& other)
		{
			return Complex::Sinh(other) / Complex::Cosh(other);
		}

		friend std::ostream& operator<<(std::ostream& os, const Complex<Type> other)
		{
			return os << other.Real() << ", " << other.Imag();
		}

	private:
		Type mReal;
		Type mImag;
	};

	/** @} */
} // namespace b3d
