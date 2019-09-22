//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Utility/BsBitLength.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template<> struct RTTIPlainType<BitLength>
	{
		enum { id = TID_BitLength }; enum { hasDynamicSize = 0 };

		static BitLength toMemory(const BitLength& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += rtti_write(data.bytes, stream);
			size += rtti_write(data.bits, stream);

			return size;
		}

		static BitLength fromMemory(BitLength& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += rtti_read(data.bytes, stream);
			size += rtti_read(data.bits, stream);

			return size;
		}

		static BitLength getSize(const BitLength& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_size(data.bytes) + rtti_size(data.bits);
		}
	};

	/** @} */
	/** @endcond */
}
