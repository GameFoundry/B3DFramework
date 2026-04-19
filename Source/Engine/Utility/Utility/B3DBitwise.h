//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DMath.h"

#if B3D_COMPILER_MSVC
#	include <intrin.h>
#endif

namespace b3d
{
	/** @addtogroup Utility
	 *  @{
	 */

	/** Floating point number broken down into components for easier access. */
	union Float754
	{
		u32 Raw;
		float Value;

		struct
		{
#if B3D_ENDIAN == B3D_ENDIAN_BIG
			u32 negative : 1;
			u32 exponent : 8;
			u32 mantissa : 23;
#else
			u32 Mantissa : 23;
			u32 Exponent : 8;
			u32 Negative : 1;
#endif
		} Field;
	};

	/** 10-bit floating point number broken down into components for easier access. */
	union Float10
	{
		u32 Raw;

		struct
		{
#if B3D_ENDIAN == B3D_ENDIAN_BIG
			u32 exponent : 5;
			u32 mantissa : 5;
#else
			u32 Mantissa : 5;
			u32 Exponent : 5;
#endif
		} Field;
	};

	/** 11-bit floating point number broken down into components for easier access. */
	union Float11
	{
		u32 Raw;

		struct
		{
#if B3D_ENDIAN == B3D_ENDIAN_BIG
			u32 exponent : 5;
			u32 mantissa : 6;
#else
			u32 Mantissa : 6;
			u32 Exponent : 5;
#endif
		} Field;
	};

	/** Class for manipulating bit patterns. */
	class Bitwise
	{
	public:
		static constexpr u32 kBytesInKilobyte = 1024;
		static constexpr u32 kBytesInMegabyte = 1024 * 1024;
		static constexpr u32 kBytesInGigabyte = 1024 * 1024 * 1024;

		/** Returns the power-of-two number greater or equal to the provided value. */
		static u32 NextPow2(u32 n)
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

		/** Returns the power-of-two number closest to the provided value. */
		static u32 ClosestPow2(u32 n)
		{
			u32 next = NextPow2(n);

			u32 prev = next >> 1;
			if(n - prev < next - n)
				return prev;

			return next;
		}

		/** Returns base-2 logarithm for common bit counts (8, 16, 32, 64), as a constant expression. */
		static constexpr u32 BitsLog2(u32 v)
		{
			switch(v)
			{
			default:
			case 8: return 3;
			case 16: return 4;
			case 32: return 5;
			case 64: return 6;
			}
		}

		/** Returns modular exponentiation for integers. */
		static u32 ModPow(u32 val1, u32 val2, u32 t)
		{
			int res = 1;

			while(val2 != 0)
			{
				if(val2 & 1)
					res = (res * val1) % t;

				val2 >>= 1;
				val1 = (val1 * val1) % t;
			}

			return res;
		}
#if B3D_COMPILER_MSVC
#	pragma intrinsic(_BitScanReverse, _BitScanForward)
#endif

		/** Finds the most-significant non-zero bit in the provided value and returns the index of that bit. */
		static u32 MostSignificantBit(u32 val)
		{
#if B3D_COMPILER_MSVC
			unsigned long index;
			_BitScanReverse(&index, val);
			return index;
#elif B3D_COMPILER_GCC || B3D_COMPILER_CLANG
			return 31 - __builtin_clz(val);
#else
			static_assert(false, "Not implemented");
#endif
		}

		/** Finds the least-significant non-zero bit in the provided value and returns the index of that bit. */
		static u32 LeastSignificantBit(u32 val)
		{
#if B3D_COMPILER_MSVC
			unsigned long index;
			_BitScanForward(&index, val);
			return index;
#elif B3D_COMPILER_GCC || B3D_COMPILER_CLANG
			return __builtin_ctz(val);
#else
			static_assert(false, "Not implemented");
#endif
		}

		/** Finds the most-significant non-zero bit in the provided value and returns the index of that bit. */
		static u32 MostSignificantBit(u64 val)
		{
#if B3D_COMPILER_MSVC
#	if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
			unsigned long index;
			_BitScanReverse64(&index, val);
			return index;
#	else // B3D_ARCHITECTURE
			if(static_cast<u32>(val >> 32) != 0)
			{
				_BitScanReverse(&index, static_cast<u32>(val >> 32));
				return index + 32;
			}
			else
			{
				_BitScanReverse(&index, static_cast<u32>(val));
				return index;
			}
#	endif // B3D_ARCHITECTURE
#elif B3D_COMPILER_GCC || B3D_COMPILER_CLANG
			return 31 - __builtin_clzll(val);
#else // B3D_COMPILER
			static_assert(false, "Not implemented");
#endif // B3D_COMPILER
		}

		/** Finds the least-significant non-zero bit in the provided value and returns the index of that bit. */
		static u32 LeastSignificantBit(u64 val)
		{
#if B3D_COMPILER_MSVC
#	if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
			unsigned long index;
			_BitScanForward64(&index, val);
			return index;
#	else // B3D_ARCHITECTURE
			if(static_cast<u32>(val) != 0)
			{
				_BitScanForward(&index, static_cast<u32>(val));
				return index;
			}
			else
			{
				_BitScanForward(&index, static_cast<u32>(val >> 32));
				return index + 32;
			}
#	endif // B3D_ARCHITECTURE
#elif B3D_COMPILER_GCC || B3D_COMPILER_CLANG
			return __builtin_ctzll(val);
#else // B3D_COMPILER
			static_assert(false, "Not implemented");
#endif // B3D_COMPILER
		}

		/** Determines whether the number is power-of-two or not. */
		template <typename T>
		static bool IsPow2(T n)
		{
			return (n & (n - 1)) == 0;
		}

		/** Returns the number of bits a pattern must be shifted right by to remove right-hand zeros. */
		template <typename T>
		static uint32_t GetBitShift(T mask)
		{
			if(mask == 0)
				return 0;

			uint32_t result = 0;
			while((mask & 1) == 0)
			{
				++result;
				mask >>= 1;
			}
			return result;
		}

		/** Count the number of set bits in a mask. */
		static uint32_t CountSetBits(uint32_t val)
		{
			uint32_t count = 0;
			for(count = 0; val; count++)
				val &= val - 1;

			return count;
		}

		/** Takes a value with a given src bit mask, and produces another value with a desired bit mask. */
		template <typename SrcT, typename DestT>
		static DestT ConvertBitPattern(SrcT srcValue, SrcT srcBitMask, DestT destBitMask)
		{
			// Mask off irrelevant source value bits (if any)
			srcValue = srcValue & srcBitMask;

			// Shift source down to bottom of DWORD
			const uint32_t srcBitShift = GetBitShift(srcBitMask);
			srcValue >>= srcBitShift;

			// Get max value possible in source from srcMask
			const SrcT srcMax = srcBitMask >> srcBitShift;

			// Get max available in dest
			const uint32_t destBitShift = GetBitShift(destBitMask);
			const DestT destMax = destBitMask >> destBitShift;

			// Scale source value into destination, and shift back
			DestT destValue = (srcValue * destMax) / srcMax;
			return (destValue << destBitShift);
		}

		/**
		 * Convert N bit color channel value to P bits. It fills P bits with the bit pattern repeated.
		 * (this is /((1<<n)-1) in fixed point).
		 */
		static uint32_t FixedToFixed(u32 value, uint32_t n, uint32_t p)
		{
			if(n > p)
			{
				// Less bits required than available; this is easy
				value >>= n - p;
			}
			else if(n < p)
			{
				// More bits required than are there, do the fill
				// Use old fashioned division, probably better than a loop
				if(value == 0)
					value = 0;
				else if(value == (static_cast<uint32_t>(1) << n) - 1)
					value = (1 << p) - 1;
				else
					value = value * (1 << p) / ((1 << n) - 1);
			}
			return value;
		}

		/**
		 * Converts floating point value in range [0, 1] to an unsigned integer of a certain number of bits. Works for any
		 * value of bits between 0 and 31.
		 */
		static uint32_t UnormToUint(float value, uint32_t bits)
		{
			if(value <= 0.0f) return 0;
			if(value >= 1.0f) return (1 << bits) - 1;
			return Math::RoundToI32(value * (1 << bits));
		}

		/**
		 * Converts floating point value in range [-1, 1] to an unsigned integer of a certain number of bits. Works for any
		 * value of bits between 0 and 31.
		 */
		static uint32_t SnormToUint(float value, uint32_t bits)
		{
			return UnormToUint((value + 1.0f) * 0.5f, bits);
		}

		/** Converts an unsigned integer to a floating point in range [0, 1]. */
		static float UintToUnorm(uint32_t value, uint32_t bits)
		{
			return (float)value / (float)((1 << bits) - 1);
		}

		/** Converts an unsigned integer to a floating point in range [-1, 1]. */
		static float UintToSnorm(uint32_t value, uint32_t bits)
		{
			return UintToUnorm(value) * 2.0f - 1.0f;
		}

		/**
		 * Converts floating point value in range [0, 1] to an unsigned integer of a certain number of bits. Works for any
		 * value of bits between 0 and 31.
		 */
		template <uint32_t bits = 8>
		static uint32_t UnormToUint(float value)
		{
			if(value <= 0.0f) return 0;
			if(value >= 1.0f) return (1 << bits) - 1;
			return Math::RoundToI32(value * (1 << bits));
		}

		/**
		 * Converts floating point value in range [-1, 1] to an unsigned integer of a certain number of bits. Works for any
		 * value of bits between 0 and 31.
		 */
		template <uint32_t bits = 8>
		static uint32_t SnormToUint(float value)
		{
			return UnormToUint<bits>((value + 1.0f) * 0.5f);
		}

		/** Converts an unsigned integer to a floating point in range [0, 1]. */
		template <uint32_t bits = 8>
		static float UintToUnorm(uint32_t value)
		{
			return (float)value / (float)((1 << bits) - 1);
		}

		/** Converts an unsigned integer to a floating point in range [-1, 1]. */
		template <uint32_t bits = 8>
		static float UintToSnorm(uint32_t value)
		{
			return UintToUnorm<bits>(value) * 2.0f - 1.0f;
		}

		/**
		 * Interpolates between two values using the @p t parameter. All parameters must be in [0, 255] range. When @p t
		 * is zero, @p from value will be returned, and when it is 255 @p to value will be returned, and interpolation
		 * between @p from and @p to will occurr for in-between values.
		 */
		static uint32_t LerpByte(uint32_t from, uint32_t to, uint32_t t)
		{
			B3D_ASSERT((from & 0xFF) == from);
			B3D_ASSERT((to & 0xFF) == to);
			B3D_ASSERT((t & 0xFF) == t);
			B3D_ASSERT(from <= to);

			return (from + (((to - from) * t) >> 8)) & 0xFF;
		}

		/**
		 * Interpolates between two values using the @p t parameter. All parameters must be in [0, 65536] range. When @p t
		 * is zero, @p from value will be returned, and when it is 65536 @p to value will be returned, and interpolation
		 * between @p from and @p to will occurr for in-between values.
		 */
		static uint32_t LerpWord(uint32_t from, uint32_t to, uint32_t t)
		{
			B3D_ASSERT((from & 0xFFFF) == from);
			B3D_ASSERT((to & 0xFFFF) == to);
			B3D_ASSERT((t & 0xFFFF) == t);
			B3D_ASSERT(from <= to);

			return (from + (((to - from) * t) >> 16)) & 0xFFFF;
		}

		/**
		 * Determines the position of the @p val parameter in the [from, to] range, returned in [0, 255] range where 0 is
		 * returned if @p val is less or equal than @p from, and 255 is returned if @p val is equal to greater to @p to,
		 * and in-between values returned accordingly. All values must be in [0, 255] range.
		 */
		static uint32_t InvLerpByte(uint32_t from, uint32_t to, uint32_t val)
		{
			B3D_ASSERT((from & 0xFF) == from);
			B3D_ASSERT((to & 0xFF) == to);
			B3D_ASSERT((val & 0xFF) == val);
			B3D_ASSERT(from <= to);

			return ((val - from) << 8) / std::max(to - from, 1u);
		}

		/**
		 * Determines the position of the @p val parameter in the [from, to] range, returned in [0, 65536] range where 0 is
		 * returned if @p val is less or equal than @p from, and 65536 is returned if @p val is equal to greater to @p to,
		 * and in-between values returned accordingly. All values must be in [0, 65536] range.
		 */
		static uint32_t InvLerpWord(uint32_t from, uint32_t to, uint32_t val)
		{
			B3D_ASSERT((from & 0xFFFF) == from);
			B3D_ASSERT((to & 0xFFFF) == to);
			B3D_ASSERT((val & 0xFFFF) == val);
			B3D_ASSERT(from <= to);

			return ((val - from) << 16) / std::max(to - from, 1u);
		}

		/** Write a n*8 bits integer value to memory in native endian. */
		static void IntWrite(void* dest, const int32_t n, const uint32_t value)
		{
			switch(n)
			{
			case 1:
				((u8*)dest)[0] = (u8)value;
				break;
			case 2:
				((u16*)dest)[0] = (u16)value;
				break;
			case 3:
#if B3D_ENDIAN == B3D_ENDIAN_BIG
				((u8*)dest)[0] = (u8)((value >> 16) & 0xFF);
				((u8*)dest)[1] = (u8)((value >> 8) & 0xFF);
				((u8*)dest)[2] = (u8)(value & 0xFF);
#else
				((u8*)dest)[2] = (u8)((value >> 16) & 0xFF);
				((u8*)dest)[1] = (u8)((value >> 8) & 0xFF);
				((u8*)dest)[0] = (u8)(value & 0xFF);
#endif
				break;
			case 4:
				((u32*)dest)[0] = (u32)value;
				break;
			}
		}

		/** Read a n*8 bits integer value to memory in native endian. */
		static uint32_t IntRead(const void* src, int32_t n)
		{
			switch(n)
			{
			case 1:
				return ((u8*)src)[0];
			case 2:
				return ((u16*)src)[0];
			case 3:
#if B3D_ENDIAN == B3D_ENDIAN_BIG
				return ((u32)((u8*)src)[0] << 16) |
					((u32)((u8*)src)[1] << 8) |
					((u32)((u8*)src)[2]);
#else
				return ((u32)((u8*)src)[0]) |
					((u32)((u8*)src)[1] << 8) |
					((u32)((u8*)src)[2] << 16);
#endif
			case 4:
				return ((u32*)src)[0];
			}
			return 0; // ?
		}

		/** Convert a float32 to a float16 (NV_half_float). */
		static u16 FloatToHalf(float i)
		{
			union
			{
				float F;
				u32 I;
			} v;

			v.F = i;
			return FloatToHalfI(v.I);
		}

		/** Converts float in u32 format to a a half in u16 format. */
		static u16 FloatToHalfI(u32 i)
		{
			int32_t s = (i >> 16) & 0x00008000;
			int32_t e = ((i >> 23) & 0x000000ff) - (127 - 15);
			int32_t m = i & 0x007fffff;

			if(e <= 0)
			{
				if(e < -10)
				{
					return 0;
				}
				m = (m | 0x00800000) >> (1 - e);

				return static_cast<u16>(s | (m >> 13));
			}
			else if(e == 0xff - (127 - 15))
			{
				if(m == 0) // Inf
				{
					return static_cast<u16>(s | 0x7c00);
				}
				else // NAN
				{
					m >>= 13;
					return static_cast<u16>(s | 0x7c00 | m | (m == 0));
				}
			}
			else
			{
				if(e > 30) // Overflow
				{
					return static_cast<u16>(s | 0x7c00);
				}

				return static_cast<u16>(s | (e << 10) | (m >> 13));
			}
		}

		/** Convert a float16 (NV_half_float) to a float32. */
		static float HalfToFloat(u16 y)
		{
			union
			{
				float F;
				u32 I;
			} v;

			v.I = HalfToFloatI(y);
			return v.F;
		}

		/** Converts a half in u16 format to a float in u32 format. */
		static u32 HalfToFloatI(u16 y)
		{
			int32_t s = (y >> 15) & 0x00000001;
			int32_t e = (y >> 10) & 0x0000001f;
			int32_t m = y & 0x000003ff;

			if(e == 0)
			{
				if(m == 0) // Plus or minus zero
				{
					return s << 31;
				}
				else // Denormalized number -- renormalize it
				{
					while(!(m & 0x00000400))
					{
						m <<= 1;
						e -= 1;
					}

					e += 1;
					m &= ~0x00000400;
				}
			}
			else if(e == 31)
			{
				if(m == 0) // Inf
				{
					return (s << 31) | 0x7f800000;
				}
				else // NaN
				{
					return (s << 31) | 0x7f800000 | (m << 13);
				}
			}

			e = e + (127 - 15);
			m = m << 13;

			return (s << 31) | (e << 23) | m;
		}

		/** Converts a 32-bit float to a 10-bit float according to OpenGL packed_float extension. */
		static u32 FloatToFloat10(float v)
		{
			Float754 f;
			f.Value = v;

			if(f.Field.Exponent == 0xFF)
			{
				// NAN or INF
				if(f.Field.Mantissa > 0)
					return 0x3E0 | (((f.Raw >> 18) | (f.Raw >> 13) | (f.Raw >> 3) | f.Raw) & 0x1F);
				else if(f.Field.Negative)
					return 0; // Negative infinity clamped to 0
				else
					return 0x3E0; // Positive infinity
			}
			else if(f.Field.Negative)
				return 0; // Negative clamped to 0, no negatives allowed
			else if(f.Raw > 0x477C0000)
				return 0x3DF; // Too large, clamp to max value
			else
			{
				u32 val;
				if(f.Raw < 0x38800000U)
				{
					// Too small to be represented as a normalized float, convert to denormalized value
					u32 shift = 113 - f.Field.Exponent;
					val = (0x800000U | f.Field.Mantissa) >> shift;
				}
				else
				{
					// Rebias exponent
					val = f.Raw + 0xC8000000;
				}

				return ((val + 0x1FFFFU + ((val >> 18) & 1)) >> 18) & 0x3FF;
			}
		}

		/** Converts a 32-bit float to a 11-bit float according to OpenGL packed_float extension. */
		static u32 FloatToFloat11(float v)
		{
			Float754 f;
			f.Value = v;

			if(f.Field.Exponent == 0xFF)
			{
				// NAN or INF
				if(f.Field.Mantissa > 0)
					return 0x7C0 | (((f.Raw >> 17) | (f.Raw >> 11) | (f.Raw >> 6) | f.Raw) & 0x3F);
				else if(f.Field.Negative)
					return 0; // Negative infinity clamped to 0
				else
					return 0x7C0; // Positive infinity
			}
			else if(f.Field.Negative)
				return 0; // Negative clamped to 0, no negatives allowed
			else if(f.Raw > 0x477E0000)
				return 0x7BF; // Too large, clamp to max value
			else
			{
				u32 val;
				if(f.Raw < 0x38800000U)
				{
					// Too small to be represented as a normalized float, convert to denormalized value
					u32 shift = 113 - f.Field.Exponent;
					val = (0x800000U | f.Field.Mantissa) >> shift;
				}
				else
				{
					// Rebias exponent
					val = f.Raw + 0xC8000000;
				}

				return ((val + 0xFFFFU + ((val >> 17) & 1)) >> 17) & 0x7FF;
			}
		}

		/** Converts a 10-bit float to a 32-bit float according to OpenGL packed_float extension. */
		static float Float10ToFloat(u32 v)
		{
			Float10 f;
			f.Raw = v;

			u32 output;
			if(f.Field.Exponent == 0x1F) // INF or NAN
			{
				output = 0x7f800000 | (f.Field.Mantissa << 17);
			}
			else
			{
				u32 exponent;
				u32 mantissa = f.Field.Mantissa;

				if(f.Field.Exponent != 0) // The value is normalized
					exponent = f.Field.Exponent;
				else if(mantissa != 0) // The value is denormalized
				{
					// Normalize the value in the resulting float
					exponent = 1;

					do
					{
						exponent--;
						mantissa <<= 1;
					}
					while((mantissa & 0x20) == 0);

					mantissa &= 0x1F;
				}
				else // The value is zero
					exponent = (u32)-112;

				output = ((exponent + 112) << 23) | (mantissa << 18);
			}

			float result;
			std::memcpy(&result, &output, sizeof(float));
			return result;
		}

		/** Converts a 11-bit float to a 32-bit float according to OpenGL packed_float extension. */
		static float Float11ToFloat(u32 v)
		{
			Float11 f;
			f.Raw = v;

			u32 output;
			if(f.Field.Exponent == 0x1F) // INF or NAN
			{
				output = 0x7f800000 | (f.Field.Mantissa << 17);
			}
			else
			{
				u32 exponent;
				u32 mantissa = f.Field.Mantissa;

				if(f.Field.Exponent != 0) // The value is normalized
					exponent = f.Field.Exponent;
				else if(mantissa != 0) // The value is denormalized
				{
					// Normalize the value in the resulting float
					exponent = 1;

					do
					{
						exponent--;
						mantissa <<= 1;
					}
					while((mantissa & 0x40) == 0);

					mantissa &= 0x3F;
				}
				else // The value is zero
					exponent = (u32)-112;

				output = ((exponent + 112) << 23) | (mantissa << 17);
			}

			float result;
			std::memcpy(&result, &output, sizeof(float));
			return result;
		}

		/**
		 * Encodes a 32-bit integer value as a base-128 varint. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes.
		 *
		 * @param	value		Value to encode.
		 * @param	output		Buffer to store the encoded bytes in. Must be at least 5 bytes in length.
		 * @return				Number of bytes required to store the value, in range [1, 5]
		 */
		static u32 EncodeVarInt(u32 value, u8* output)
		{
			u32 index = 0;
			if(value & 0xFFFFFF80U)
			{
				output[index++] = (u8)(value | 0x80);
				value >>= 7;

				if(value & 0xFFFFFF80U)
				{
					output[index++] = (u8)(value | 0x80);
					value >>= 7;

					if(value & 0xFFFFFF80U)
					{
						output[index++] = (u8)(value | 0x80);
						value >>= 7;

						if(value & 0xFFFFFF80U)
						{
							output[index++] = (u8)(value | 0x80);
							value >>= 7;
						}
					}
				}
			}

			output[index++] = (u8)value;
			return index;
		}

		/**
		 * Decodes a value encoded using encodeVarInt(u32, u8*).
		 *
		 * @param	outValue	Variable to receive the decoded value.
		 * @param	input		Input buffer to decode the data from.
		 * @param	size		Size of the input buffer.
		 * @return				Number of bytes read.
		 */
		static u32 DecodeVarInt(u32& outValue, const u8* input, u32 size)
		{
			if(size == 0)
				return 0;

			u32 index = 0;
			outValue = (u32)(input[index] & 0x7F);
			if(input[index++] & 0x80 && --size)
			{
				outValue |= (u32)(input[index] & 0x7F) << 7;

				if(input[index++] & 0x80 && --size)
				{
					outValue |= (u32)(input[index] & 0x7F) << 14;

					if(input[index++] & 0x80 && --size)
					{
						outValue |= (u32)(input[index] & 0x7F) << 21;

						if(input[index++] & 0x80 && --size)
							outValue |= (u32)(input[index++]) << 28;
					}
				}
			}

			return !size || input[index - 1] & 0x80 ? 0 : index;
		}

		/** @copydoc EncodeVarInt(u32, u8*) */
		static u32 EncodeVarInt(i32 value, u8* output)
		{
			// Encode using zig-zag pattern so that negative values don't take up max byte count
			u32 temp = (value << 1) ^ (value >> 31);
			return EncodeVarInt(temp, output);
		}

		/** @copydoc DecodeVarInt(u32&, const u8*, u32) */
		static u32 DecodeVarInt(i32& outValue, const u8* input, u32 size)
		{
			u32 temp;

			u32 readBytes = DecodeVarInt(temp, input, size);
			outValue = (i32)((temp >> 1) ^ -((i32)temp & 1));

			return readBytes;
		}

		/**
		 * Encodes a 64-bit integer value as a base-128 varint. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes.
		 *
		 * @param	value		Value to encode.
		 * @param	output		Buffer to store the encoded bytes in. Must be at least 10 bytes in length.
		 * @return				Number of bytes required to store the value, in range [1, 10]
		 */
		static u32 EncodeVarInt(u64 value, u8* output)
		{
			u32 index = 0;
			if(value & 0xFFFFFFFFFFFFFF80ULL)
			{
				output[index++] = (u8)(value | 0x80);
				value >>= 7;

				if(value & 0xFFFFFFFFFFFFFF80ULL)
				{
					output[index++] = (u8)(value | 0x80);
					value >>= 7;

					if(value & 0xFFFFFFFFFFFFFF80ULL)
					{
						output[index++] = (u8)(value | 0x80);
						value >>= 7;

						if(value & 0xFFFFFFFFFFFFFF80ULL)
						{
							output[index++] = (u8)(value | 0x80);
							value >>= 7;

							if(value & 0xFFFFFFFFFFFFFF80ULL)
							{
								output[index++] = (u8)(value | 0x80);
								value >>= 7;

								if(value & 0xFFFFFFFFFFFFFF80ULL)
								{
									output[index++] = (u8)(value | 0x80);
									value >>= 7;

									if(value & 0xFFFFFFFFFFFFFF80ULL)
									{
										output[index++] = (u8)(value | 0x80);
										value >>= 7;

										if(value & 0xFFFFFFFFFFFFFF80ULL)
										{
											output[index++] = (u8)(value | 0x80);
											value >>= 7;

											if(value & 0xFFFFFFFFFFFFFF80ULL)
											{
												output[index++] = (u8)(value | 0x80);
												value >>= 7;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			output[index++] = (u8)value;
			return index;
		}

		/**
		 * Decodes a value encoded using encodeVarInt(u64, u8*).
		 *
		 * @param	outValue	Variable to receive the decoded value.
		 * @param	input		Input buffer to decode the data from.
		 * @param	size		Size of the input buffer.
		 * @return				Number of bytes read.
		 */
		static u32 DecodeVarInt(u64& outValue, const u8* input, u32 size)
		{
			if(size == 0)
				return 0;

			u32 index = 0;
			outValue = (u64)(input[index] & 0x7F);
			if(input[index++] & 0x80 && --size)
			{
				outValue |= (u64)(input[index] & 0x7F) << 7;

				if(input[index++] & 0x80 && --size)
				{
					outValue |= (u64)(input[index] & 0x7F) << 14;

					if(input[index++] & 0x80 && --size)
					{
						outValue |= (u64)(input[index] & 0x7F) << 21;

						if(input[index++] & 0x80 && --size)
						{
							outValue |= (u64)(input[index] & 0x7F) << 28;

							if(input[index++] & 0x80 && --size)
							{
								outValue |= (u64)(input[index] & 0x7F) << 35;

								if(input[index++] & 0x80 && --size)
								{
									outValue |= (u64)(input[index] & 0x7F) << 42;

									if(input[index++] & 0x80 && --size)
									{
										outValue |= (u64)(input[index] & 0x7F) << 49;

										if(input[index++] & 0x80 && --size)
										{
											outValue |= (u64)(input[index] & 0x7F) << 56;

											if(input[index++] & 0x80 && --size)
												outValue |= (u64)(input[index++]) << 63;
										}
									}
								}
							}
						}
					}
				}
			}

			return !size || input[index - 1] & 0x80 ? 0 : index;
		}

		/** @copydoc EncodeVarInt(u64, u8*) */
		static u32 EncodeVarInt(i64 value, u8* output)
		{
			// Encode using zig-zag pattern so that negative values don't take up max byte count
			u64 temp = (value << 1) ^ (value >> 63);
			return EncodeVarInt(temp, output);
		}

		/** @copydoc DecodeVarInt(u64&, const u8*, u32) */
		static u32 DecodeVarInt(i64& outValue, const u8* input, u32 size)
		{
			u64 temp;

			u32 readBytes = DecodeVarInt(temp, input, size);
			outValue = (i64)((temp >> 1) ^ -((i64)temp & 1));

			return readBytes;
		}

		/** Converts a float in range [-1,1] into an unsigned 8-bit integer. */
		static u8 Quantize8BitSigned(float v)
		{
			return Quantize8BitUnsigned(v * 0.5f + 0.5f);
		}

		/** Converts a float in range [0,1] into an unsigned 8-bit integer. */
		static u8 Quantize8BitUnsigned(float v)
		{
			return (u8)(v * 255.999f);
		}
	};

	/** @} */
} // namespace b3d
