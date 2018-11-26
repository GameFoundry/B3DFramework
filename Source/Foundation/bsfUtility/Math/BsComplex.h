#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	template <class Type>
	class Complex
	{
	public:
		Complex() = default;

		Complex(const Type& r, const Type& i)
			: mReal(r), mImag(i) {}

		Complex(const Complex& other)
			: mReal(other.real()), mImag(other.imag()) {}

		Complex<Type>& operator= (const Type& other)
		{
			mReal = other;
			mImag = Type();

			return *this;
		}

		Complex<Type>& operator+= (const Type& other)
		{
			mReal += other;

			return *this;
		}

		Complex<Type>& operator-= (const Type& other)
		{
			mReal -= other;

			return *this;
		}

		Complex<Type>& operator*= (const Type& other)
		{
			mReal *= other;
			mImag *= other;

			return *this;
		}

		Complex<Type>& operator/= (const Type& other)
		{
			mReal /= other;
			mImag /= other;

			return *this;
		}

		Complex<Type>& operator= (const Complex<Type>& other)
		{
			mReal = other.real();
			mImag = other.imag();

			return *this;
		}

		Complex<Type>& operator+= (const Complex<Type>& other)
		{
			mReal += other.real();
			mImag += other.imag();

			return *this;
		}

		Complex<Type>& operator-= (const Complex<Type>& other)
		{
			mReal -= other.real();
			mImag -= other.imag();

			return *this;
		}

		Complex<Type>& operator*= (const Complex<Type>& other)
		{
			const Type r = mReal * other.real() - mImag * other.imag();
			mImag = mReal * other.imag() + mImag * other.real();
			mReal = r;

			return *this;
		}

		Complex<Type>& operator/= (const Complex<Type>& other)
		{
			const Type r = mReal * other.real() + mImag * other.imag();
			const Type n = Complex::norm(other);
			mImag = (mImag * other.real() - mReal * other.imag()) / n;
			mReal = r / n;

			return *this;
		}

		Complex<Type> operator+ (const Type& other) const
		{
			return Complex(mReal + other, mImag);
		}

		Complex<Type> operator- (const Type& other) const
		{
			return Complex(mReal - other, mImag);
		}

		Complex<Type> operator* (const Type& other) const
		{
			return Complex(mReal * other, mImag);
		}

		Complex<Type> operator/ (const Type& other) const
		{
			return Complex(mReal / other, mImag);
		}

		Complex<Type> operator+ (const Complex<Type>& other) const
		{
			return Complex(mReal + other.real(), mImag + other.imag());
		}

		Complex<Type> operator- (const Complex<Type>& other) const
		{
			return Complex(mReal - other.real(), mImag - other.imag());
		}

		Complex<Type> operator* (const Complex<Type>& other) const
		{
			Complex<Type> res = *this;

			res *= other;

			return res;
		}

		Complex<Type> operator/ (const Complex<Type>& other) const
		{
			Complex<Type> res = *this;

			res /= other;

			return res;
		}

		bool operator== (const Complex<Type>& other) const
		{
			return mReal == other.real() && mImag == other.imag();
		}

		bool operator== (const Type& other) const
		{
			return mReal == other && mImag == Type();
		}

		bool operator!= (const Complex<Type>& other) const
		{
			return mReal != other.real() || mImag != other.imag();
		}

		bool operator!= (const Type& other) const
		{
			return mReal != other || mImag != Type();
		}

		Type& real() { return mReal; }
		Type& imag() { return mImag; }

		const Type& real() const { return mReal; }
		const Type& imag() const { return mImag; }

		static Type abs(const Complex<Type>& other)
		{
			Type x = other.real();
			Type y = other.imag();
			const Type s = std::max(std::abs(x), std::abs(y));
			if (s == Type())
				return s;

			x /= s;
			y /= s;

			return s * std::sqrt(x * x + y * y);
		}

		static Type arg(const Complex<Type>& other)
		{
			return std::atan2(other.imag(), other.real());
		}

		static Type norm(const Complex<Type>& other)
		{
			const Type x = other.real();
			const Type y = other.imag();

			return x * x + y * y;
		}

		static Complex<Type> conj(const Complex<Type>& other)
		{
			return Complex(other.real(), -other.imag());
		}

		static Complex<Type> polar(const Type& r, const Type& t = 0)
		{
			return Complex(r * std::cos(t), r * std::sin(t));
		}

		static Complex<Type> cos(const Complex<Type>& other)
		{
			const Type x = other.real();
			const Type y = other.imag();

			return Complex(std::cos(x) * std::cosh(y), -std::sin(x) * std::sinh(y));
		}

		static Complex<Type> cosh(const Complex<Type>& other)
		{
			const Type x = other.real();
			const Type y = other.imag();

			return Complex(std::cosh(x) * std::cos(y), std::sinh(x) * std::sin(y));
		}

		static Complex<Type> exp(const Complex<Type>& other)
		{
			return Complex::polar(std::exp(other.real()), other.imag());
		}

		static Complex<Type> log(const Complex<Type>& other)
		{
			return Complex(std::log(Complex::abs(other)), Complex::arg(other));
		}

		static Complex<Type> log10(const Complex<Type>& other)
		{
			return Complex::log(other) / std::log(Type(10));
		}

		static Complex<Type> pow(const Complex<Type>& other, const Type& i)
		{
			if (other.imag() == Type() && other.real() > Type())
				return Complex(std::pow(other.real(), i), other.imag());

			Complex<Type> t = Complex::log(other);
			return Complex::polar(std::exp(i * t.real()), i * t.imag());
		}

		static Complex<Type> pow(const Complex<Type>& x, const Complex<Type>& y)
		{
			return Complex::exp(y * Complex::log(x));
		}

		static Complex<Type> pow(const Type& i, const Complex<Type>& other)
		{
			return i > Type() ?
				Complex::polar(std::pow(i, other.real()), other.imag() * std::log(i))
				: Complex::pow(Complex(i, Type()), other);
		}

		static Complex<Type> sin(const Complex<Type>& other)
		{
			const Type x = other.real();
			const Type y = other.imag();

			return Complex(std::sin(x) * std::cosh(y), std::cos(x) * std::sinh(y));
		}

		static Complex<Type> sinh(const Complex<Type>& other)
		{
			const Type x = other.real();
			const Type y = other.imag();

			return Complex(std::sinh(x) * std::cos(y), std::cosh(x) * std::sin(y));
		}

		static Complex<Type> sqrt(const Complex<Type>& other)
		{
			const Type x = other.real();
			const Type y = other.imag();

			if (x == Type())
			{
				Type t = std::sqrt(std::abs(y) / 2);
				return Complex(t, y < Type() ? -t : t);
			}
			else
			{
				Type t = std::sqrt(2 * (Complex::abs(other) + std::abs(x)));
				Type u = t / 2;
				return x > Type() ? Complex(u, y / t)
					: Complex(std::abs(y) / t, y < Type() ? -u : u);
			}
		}

		static Complex<Type> tan(const Complex<Type>& other)
		{
			return Complex::sin(other) / Complex::cos(other);
		}

		static Complex<Type> tanh(const Complex<Type>& other)
		{
			return Complex::sinh(other) / Complex::cosh(other);
		}

		friend std::ostream& operator<< (std::ostream& os, const Complex<Type> other)
		{
			return os << other.real() << ", " << other.imag();
		}

	private:
		Type mReal;
		Type mImag;
	};
}
