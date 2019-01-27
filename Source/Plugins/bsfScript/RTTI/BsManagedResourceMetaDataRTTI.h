//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
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
		String& getNamespace(ManagedResourceMetaData* obj)
		{
			return obj->typeNamespace;
		}

		void setNamespace(ManagedResourceMetaData* obj, String& val)
		{
			obj->typeNamespace = val;
		}

		String& getTypename(ManagedResourceMetaData* obj)
		{
			return obj->typeName;
		}

		void setTypename(ManagedResourceMetaData* obj, String& val)
		{
			obj->typeName = val;
		}

	public:
		ManagedResourceMetaDataRTTI()
		{
			addPlainField("mTypeNamespace", 0, &ManagedResourceMetaDataRTTI::getNamespace, &ManagedResourceMetaDataRTTI::setNamespace);
			addPlainField("mTypeName", 1, &ManagedResourceMetaDataRTTI::getTypename, &ManagedResourceMetaDataRTTI::setTypename);
		}

		const String& getRTTIName() override
		{
			static String name = "ManagedResourceMetaData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ManagedResourceMetaData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
}