//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "BsManagedResourceMetaData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedResourceMetaDataRTTI : public RTTIType<ManagedResourceMetaData, ResourceMetaData, ManagedResourceMetaDataRTTI>
	{
	private:
		String& GetNamespace(ManagedResourceMetaData* obj)
		{
			return obj->TypeNamespace;
		}

		void SetNamespace(ManagedResourceMetaData* obj, String& val)
		{
			obj->TypeNamespace = val;
		}

		String& GetTypename(ManagedResourceMetaData* obj)
		{
			return obj->TypeName;
		}

		void SetTypename(ManagedResourceMetaData* obj, String& val)
		{
			obj->TypeName = val;
		}

	public:
		ManagedResourceMetaDataRTTI()
		{
			AddPlainField("mTypeNamespace", 0, &ManagedResourceMetaDataRTTI::GetNamespace, &ManagedResourceMetaDataRTTI::SetNamespace);
			AddPlainField("mTypeName", 1, &ManagedResourceMetaDataRTTI::GetTypename, &ManagedResourceMetaDataRTTI::SetTypename);
		}

		const String& GetRttiName()
		{
			static String name = "ManagedResourceMetaData";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ManagedResourceMetaData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
