//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Utility/BsFlags.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	template<class Enum, class Storage> struct RTTIPlainType<Flags<Enum, Storage>>
	{
		enum { id = TID_Flags }; enum { hasDynamicSize = 0 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const Flags<Enum, Storage>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			Storage storageData = (Storage)data;
			return RTTIPlainType<Storage>::toMemory(storageData, stream, fieldInfo, compress);
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(Flags<Enum, Storage>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			Storage storageData;
			RTTIPlainType<Storage>::fromMemory(storageData, stream, fieldInfo, compress);

			data = Flags<Enum, Storage>(storageData);
			return sizeof(Flags<Enum, Storage>);
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const Flags<Enum, Storage>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(Flags<Enum, Storage>);
		}
	};

	/** @} */
	/** @endcond */
}
