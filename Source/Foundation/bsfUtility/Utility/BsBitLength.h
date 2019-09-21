//************************************ bs::framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Encodes a length in bits, as a number of bytes and the number of leftover bits. */
	struct BitLength
	{
		BitLength(uint32_t bytes = 0, uint8_t bits = 0)
			:bytes(bytes), bits(bits)
		{
			assert(bits < 8);
		}

		bool operator== (const BitLength& rhs) const
		{
			return bytes == rhs.bytes && bits == rhs.bits;
		}

		bool operator!= (const BitLength& rhs) const
		{
			return (bytes != rhs.bytes || bits != rhs.bits);
		}

		bool operator< (const BitLength& rhs) const
		{
			return bytes < rhs.bytes || (bytes == rhs.bytes && bits < rhs.bits);
		}
		
		bool operator<= (const BitLength& rhs) const
		{
			return bytes < rhs.bytes || (bytes == rhs.bytes && bits <= rhs.bits);
		}

		bool operator> (const BitLength& rhs) const
		{
			return bytes > rhs.bytes || (bytes == rhs.bytes && bits > rhs.bits);
		}
		
		bool operator>= (const BitLength& rhs) const
		{
			return bytes > rhs.bytes || (bytes == rhs.bytes && bits >= rhs.bits);
		}
		
		BitLength operator+ (const BitLength& rhs) const
		{
			uint8_t totalBits = bits + rhs.bits;
			return BitLength(bytes + rhs.bytes + (totalBits / 8), totalBits % 8);
		}

		BitLength operator- (const BitLength& rhs) const
		{
			if (rhs.bytes > bytes)
				return BitLength();
			else if(rhs.bytes == bytes)
			{
				if (rhs.bits >= bits)
					return BitLength();
			}

			uint32_t newBytes = bytes - rhs.bytes;
			uint32_t newBits;
			if(rhs.bits > bits)
			{
				newBytes--;
				newBits = 8 - (rhs.bits - bits);
			}
			else
				newBits = bits - rhs.bits;
			
			return BitLength(newBytes, newBits);
		}

		BitLength operator* (const uint32_t& rhs) const
		{
			uint64_t newBits = ((uint64_t)bytes * 8 + bits) * rhs;
			return BitLength((uint32_t)newBits / 8, (uint32_t)newBits % 8);
		}

		BitLength& operator+= (const BitLength& rhs)
		{
			*this = *this + rhs;
			return *this;
		}

		BitLength& operator-= (const BitLength& rhs)
		{
			*this = *this - rhs;
			return *this;
		}

		BitLength& operator*= (const uint32_t& rhs)
		{
			*this = *this * rhs;
			return *this;
		}

		/** Returns the encoded length in number of bits. */
		uint64_t getBits() const { return ((uint64_t)bytes * 8) + bits; }

		/** Constructs a new bit length object from a number of bits. */
		static BitLength fromBits(uint64_t bits)
		{
			return BitLength((uint32_t)(bits / 8), (uint32_t)(bits % 8));
		}

		uint32_t bytes;
		uint8_t bits;
	};

	/** @} */
}
