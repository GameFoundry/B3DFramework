//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
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