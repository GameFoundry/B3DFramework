//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Utility/B3DBitLength.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <>
	struct RTTIPlainType<BitLength>
	{
		enum
		{
			id = TID_BitLength
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const BitLength& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += B3DRTTIWrite(data.Bytes, stream);
			size += B3DRTTIWrite(data.Bits, stream);

			return size;
		}

		static BitLength FromMemory(BitLength& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += B3DRTTIRead(data.Bytes, stream);
			size += B3DRTTIRead(data.Bits, stream);

			return size;
		}

		static BitLength GetSize(const BitLength& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTISize(data.Bytes) + B3DRTTISize(data.Bits);
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
