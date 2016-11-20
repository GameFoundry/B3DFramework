//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace bs 
{
	/** @addtogroup General
	 *  @{
	 */

    /** Class for manipulating bit patterns. */
    class Bitwise 
	{
    public:
		/** Returns the most significant bit set in a value. */
		static unsigned int mostSignificantBitSet(unsigned int value)
        {
            unsigned int result = 0;
            while (value != 0) {
                ++result;
                value >>= 1;
            }
            return result-1;
        }

		/** Returns the closest power-of-two number greater or equal to value. */
        static UINT32 firstPO2From(UINT32 n)
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

		/** Determines whether the number is power-of-two or not. */
        template<typename T>
        static bool isPO2(T n)
        {
            return (n & (n-1)) == 0;
        }

		/** Returns the number of bits a pattern must be shifted right by to remove right-hand zeros. */
		template<typename T>
        static unsigned int getBitShift(T mask)
		{
			if (mask == 0)
				return 0;

			unsigned int result = 0;
			while ((mask & 1) == 0) {
				++result;
				mask >>= 1;
			}
			return result;
		}

		/** Takes a value with a given src bit mask, and produces another value with a desired bit mask. */
		template<typename SrcT, typename DestT>
        static DestT convertBitPattern(SrcT srcValue, SrcT srcBitMask, DestT destBitMask)
		{
			// Mask off irrelevant source value bits (if any)
			srcValue = srcValue & srcBitMask;

			// Shift source down to bottom of DWORD
			const unsigned int srcBitShift = getBitShift(srcBitMask);
			srcValue >>= srcBitShift;

			// Get max value possible in source from srcMask
			const SrcT srcMax = srcBitMask >> srcBitShift;

			// Get max available in dest
			const unsigned int destBitShift = getBitShift(destBitMask);
			const DestT destMax = destBitMask >> destBitShift;

			// Scale source value into destination, and shift back
			DestT destValue = (srcValue * destMax) / srcMax;
			return (destValue << destBitShift);
		}

		/** 
		 * Convert N bit colour channel value to P bits. It fills P bits with the bit pattern repeated. 
		 * (this is /((1<<n)-1) in fixed point).
		 */
        static unsigned int fixedToFixed(UINT32 value, unsigned int n, unsigned int p) 
        {
            if(n > p) 
            {
                // Less bits required than available; this is easy
                value >>= n-p;
            } 
            else if(n < p)
            {
                // More bits required than are there, do the fill
                // Use old fashioned division, probably better than a loop
                if(value == 0)
                        value = 0;
                else if(value == (static_cast<unsigned int>(1)<<n)-1)
                        value = (1<<p)-1;
                else    value = value*(1<<p)/((1<<n)-1);
            }
            return value;    
        }

		/** 
		 * Convert floating point color channel value between 0.0 and 1.0 (otherwise clamped) to integer of a certain 
		 * number of bits. Works for any value of bits between 0 and 31.
		 */
        static unsigned int floatToFixed(const float value, const unsigned int bits)
        {
            if(value <= 0.0f) return 0;
            else if (value >= 1.0f) return (1<<bits)-1;
            else return (unsigned int)(value * (1<<bits));     
        }

		/** Fixed point to float. */
        static float fixedToFloat(unsigned value, unsigned int bits)
        {
            return (float)value/(float)((1<<bits)-1);
        }

		/** Write a n*8 bits integer value to memory in native endian. */
        static void intWrite(void *dest, const int n, const unsigned int value)
        {
            switch(n) {
                case 1:
                    ((UINT8*)dest)[0] = (UINT8)value;
                    break;
                case 2:
                    ((UINT16*)dest)[0] = (UINT16)value;
                    break;
                case 3:
#if BS_ENDIAN == BS_ENDIAN_BIG      
                    ((UINT8*)dest)[0] = (UINT8)((value >> 16) & 0xFF);
                    ((UINT8*)dest)[1] = (UINT8)((value >> 8) & 0xFF);
                    ((UINT8*)dest)[2] = (UINT8)(value & 0xFF);
#else
                    ((UINT8*)dest)[2] = (UINT8)((value >> 16) & 0xFF);
                    ((UINT8*)dest)[1] = (UINT8)((value >> 8) & 0xFF);
                    ((UINT8*)dest)[0] = (UINT8)(value & 0xFF);
#endif
                    break;
                case 4:
                    ((UINT32*)dest)[0] = (UINT32)value;                
                    break;                
            }        
        }

		/** Read a n*8 bits integer value to memory in native endian. */
        static unsigned int intRead(const void *src, int n) {
            switch(n) {
                case 1:
                    return ((UINT8*)src)[0];
                case 2:
                    return ((UINT16*)src)[0];
                case 3:
#if BS_ENDIAN == BS_ENDIAN_BIG      
                    return ((UINT32)((UINT8*)src)[0]<<16)|
                            ((UINT32)((UINT8*)src)[1]<<8)|
                            ((UINT32)((UINT8*)src)[2]);
#else
                    return ((UINT32)((UINT8*)src)[0])|
                            ((UINT32)((UINT8*)src)[1]<<8)|
                            ((UINT32)((UINT8*)src)[2]<<16);
#endif
                case 4:
                    return ((UINT32*)src)[0];
            } 
            return 0; // ?
        }

		/** Convert a float32 to a float16 (NV_half_float). */
        static UINT16 floatToHalf(float i)
        {
            union { float f; UINT32 i; } v;
            v.f = i;
            return floatToHalfI(v.i);
        }

		/** Converts float in UINT32 format to a a half in UINT16 format. */
        static UINT16 floatToHalfI(UINT32 i)
        {
            int s =  (i >> 16) & 0x00008000;
            int e = ((i >> 23) & 0x000000ff) - (127 - 15);
            int m =   i        & 0x007fffff;
        
            if (e <= 0)
            {
                if (e < -10)
                {
                    return 0;
                }
                m = (m | 0x00800000) >> (1 - e);
        
                return static_cast<UINT16>(s | (m >> 13));
            }
            else if (e == 0xff - (127 - 15))
            {
                if (m == 0) // Inf
                {
                    return static_cast<UINT16>(s | 0x7c00);
                } 
                else    // NAN
                {
                    m >>= 13;
                    return static_cast<UINT16>(s | 0x7c00 | m | (m == 0));
                }
            }
            else
            {
                if (e > 30) // Overflow
                {
                    return static_cast<UINT16>(s | 0x7c00);
                }
        
                return static_cast<UINT16>(s | (e << 10) | (m >> 13));
            }
        }
        
		/** Convert a float16 (NV_half_float) to a float32. */
        static float halfToFloat(UINT16 y)
        {
            union { float f; UINT32 i; } v;
            v.i = halfToFloatI(y);
            return v.f;
        }

		/** Converts a half in UINT16 format to a float in UINT32 format. */
        static UINT32 halfToFloatI(UINT16 y)
        {
            int s = (y >> 15) & 0x00000001;
            int e = (y >> 10) & 0x0000001f;
            int m =  y        & 0x000003ff;
        
            if (e == 0)
            {
                if (m == 0) // Plus or minus zero
                {
                    return s << 31;
                }
                else // Denormalized number -- renormalize it
                {
                    while (!(m & 0x00000400))
                    {
                        m <<= 1;
                        e -=  1;
                    }
        
                    e += 1;
                    m &= ~0x00000400;
                }
            }
            else if (e == 31)
            {
                if (m == 0) // Inf
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
    };

	/** @} */
}