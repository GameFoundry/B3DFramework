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
			return obj->typeNamespace;
		}

		void SetNamespace(ManagedResourceMetaData* obj, String& val)
		{
			obj->typeNamespace = val;
		}

		String& GetTypename(ManagedResourceMetaData* obj)
		{
			return obj->typeName;
		}

		void SetTypename(ManagedResourceMetaData* obj, String& val)
		{
			obj->typeName = val;
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

		UINT32 GetRttiId() 
		{
			return TID_ManagedResourceMetaData;
		}

		SPtr<IReflectable> NewRttiObject() 
		{
			return bs_shared_ptr_new<ManagedResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
}
