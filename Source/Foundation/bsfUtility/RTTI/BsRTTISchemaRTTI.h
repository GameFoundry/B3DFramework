//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class RTTIFieldSchemaRTTI : public RTTIType<RTTIFieldSchema, IReflectable, RTTIFieldSchemaRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(id, 0)
			BS_RTTI_MEMBER_PLAIN(type, 1)
			BS_RTTI_MEMBER_PLAIN(isArray, 2)
			BS_RTTI_MEMBER_PLAIN(hasDynamicSize, 3)
			BS_RTTI_MEMBER_PLAIN(size, 4)
			BS_RTTI_MEMBER_PLAIN(fieldTypeId, 5)
			BS_RTTI_MEMBER_REFLPTR(fieldTypeSchema, 6)
			BS_RTTI_MEMBER_PLAIN(info, 7)
		BS_END_RTTI_MEMBERS
		
	public:
		const String& getRTTIName() override
		{
			static String name = "RTTIFieldSchema";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_RTTIFieldSchema;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<RTTIFieldSchema>();
		}
	};

	class RTTISchemaRTTI : public RTTIType<RTTISchema, IReflectable, RTTISchemaRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(typeId, 0)
			BS_RTTI_MEMBER_REFLPTR(baseTypeSchema, 1)
			BS_RTTI_MEMBER_PLAIN_ARRAY(fieldSchemas, 2)
		BS_END_RTTI_MEMBERS
		
	public:
		const String& getRTTIName() override
		{
			static String name = "RTTISchema";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_RTTISchema;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<RTTISchema>();
		}
	};

	/** @} */
	/** @endcond */
}
