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
		static void toMemory(const Flags<Enum, Storage>& data, char* memory)
		{
			Storage storageData = (Storage)data;
			RTTIPlainType<Storage>::toMemory(storageData, memory);
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(Flags<Enum, Storage>& data, char* memory)
		{
			Storage storageData;
			RTTIPlainType<Storage>::fromMemory(storageData, memory);

			data = Flags<Enum, Storage>(storageData);
			return sizeof(Flags<Enum, Storage>);
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const Flags<Enum, Storage>& data)
		{
			assert(false);
			return sizeof(Flags<Enum, Storage>);
		}
	};

	/** @} */
	/** @endcond */
}
