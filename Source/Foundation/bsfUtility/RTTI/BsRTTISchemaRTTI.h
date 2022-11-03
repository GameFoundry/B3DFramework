//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "RTTI/BsFlagsRTTI.h"
#include "RTTI/BsBitLengthRTTI.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <>
	struct RTTIPlainType<RTTIFieldInfo>
	{
		enum
		{
			id = TID_RTTIFieldInfo
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const RTTIFieldInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += B3DRTTIWrite(data.Flags, stream);

			return size;
		}

		static BitLength FromMemory(RTTIFieldInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += B3DRTTIRead(data.Flags, stream);

			return size;
		}

		static BitLength GetSize(const RTTIFieldInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTISize(data.Flags);
		}
	};

	class RTTIFieldSchemaRTTI : public RTTIType<RTTIFieldSchema, IReflectable, RTTIFieldSchemaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Id, 0)
			B3D_RTTI_MEMBER_PLAIN(Type, 1)
			B3D_RTTI_MEMBER_PLAIN(IsArray, 2)
			B3D_RTTI_MEMBER_PLAIN(HasDynamicSize, 3)
			B3D_RTTI_MEMBER_PLAIN(Size, 4)
			B3D_RTTI_MEMBER_PLAIN(FieldTypeId, 5)
			B3D_RTTI_MEMBER_REFLPTR(FieldTypeSchema, 6)
			B3D_RTTI_MEMBER_PLAIN(Info, 7)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "RTTIFieldSchema";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_RTTIFieldSchema;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<RTTIFieldSchema>();
		}
	};

	class RTTISchemaRTTI : public RTTIType<RTTISchema, IReflectable, RTTISchemaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(TypeId, 0)
			B3D_RTTI_MEMBER_REFLPTR(BaseTypeSchema, 1)
			B3D_RTTI_MEMBER_REFL_ARRAY(FieldSchemas, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "RTTISchema";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_RTTISchema;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<RTTISchema>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
