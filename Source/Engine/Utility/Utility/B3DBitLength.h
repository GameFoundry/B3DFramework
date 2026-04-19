//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Encodes a length in bits, as a number of bytes and the number of leftover bits. */
	struct BitLength
	{
		BitLength(uint32_t bytes = 0, uint8_t bits = 0)
			: Bytes(bytes), Bits(bits)
		{
			B3D_ASSERT(bits < 8);
		}

		bool operator==(const BitLength& rhs) const
		{
			return Bytes == rhs.Bytes && Bits == rhs.Bits;
		}

		bool operator!=(const BitLength& rhs) const
		{
			return (Bytes != rhs.Bytes || Bits != rhs.Bits);
		}

		bool operator<(const BitLength& rhs) const
		{
			return Bytes < rhs.Bytes || (Bytes == rhs.Bytes && Bits < rhs.Bits);
		}

		bool operator<=(const BitLength& rhs) const
		{
			return Bytes < rhs.Bytes || (Bytes == rhs.Bytes && Bits <= rhs.Bits);
		}

		bool operator>(const BitLength& rhs) const
		{
			return Bytes > rhs.Bytes || (Bytes == rhs.Bytes && Bits > rhs.Bits);
		}

		bool operator>=(const BitLength& rhs) const
		{
			return Bytes > rhs.Bytes || (Bytes == rhs.Bytes && Bits >= rhs.Bits);
		}

		BitLength operator+(const BitLength& rhs) const
		{
			uint8_t totalBits = Bits + rhs.Bits;
			return BitLength(Bytes + rhs.Bytes + (totalBits / 8), totalBits % 8);
		}

		BitLength operator-(const BitLength& rhs) const
		{
			if(rhs.Bytes > Bytes)
				return BitLength();
			else if(rhs.Bytes == Bytes)
			{
				if(rhs.Bits >= Bits)
					return BitLength();
			}

			uint32_t newBytes = Bytes - rhs.Bytes;
			uint32_t newBits;
			if(rhs.Bits > Bits)
			{
				newBytes--;
				newBits = 8 - (rhs.Bits - Bits);
			}
			else
				newBits = Bits - rhs.Bits;

			return BitLength(newBytes, newBits);
		}

		BitLength operator*(const uint32_t& rhs) const
		{
			uint64_t newBits = ((uint64_t)Bytes * 8 + Bits) * rhs;
			return BitLength((uint32_t)newBits / 8, (uint32_t)newBits % 8);
		}

		BitLength& operator+=(const BitLength& rhs)
		{
			*this = *this + rhs;
			return *this;
		}

		BitLength& operator-=(const BitLength& rhs)
		{
			*this = *this - rhs;
			return *this;
		}

		BitLength& operator*=(const uint32_t& rhs)
		{
			*this = *this * rhs;
			return *this;
		}

		/** Returns the encoded length in number of bits. */
		uint64_t GetBits() const { return ((uint64_t)Bytes * 8) + Bits; }

		/** Constructs a new bit length object from a number of bits. */
		static BitLength FromBits(uint64_t bits)
		{
			return BitLength((uint32_t)(bits / 8), (uint32_t)(bits % 8));
		}

		uint32_t Bytes;
		uint8_t Bits;
	};

	/** @} */
} // namespace b3d
