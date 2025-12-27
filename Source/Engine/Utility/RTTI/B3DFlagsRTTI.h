//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Utility/B3DFlags.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <class Enum, class Storage>
	struct RTTIPlainType<Flags<Enum, Storage>>
	{
		enum
		{
			id = TID_Flags
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const Flags<Enum, Storage>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			Storage storageData = (Storage)data;
			return RTTIPlainType<Storage>::ToMemory(storageData, stream, fieldInfo, compress);
		}

		static BitLength FromMemory(Flags<Enum, Storage>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			Storage storageData;
			RTTIPlainType<Storage>::FromMemory(storageData, stream, fieldInfo, compress);

			data = Flags<Enum, Storage>(storageData);
			return sizeof(Flags<Enum, Storage>);
		}

		static BitLength GetSize(const Flags<Enum, Storage>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(Flags<Enum, Storage>);
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
