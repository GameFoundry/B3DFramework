//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "RTTI/B3DBitLengthRTTI.h"
#include "Reflection/B3DRTTIType.h"

namespace b3d
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

	class RTTIFieldDataTypeSchemaRTTI : public TRTTIType<RTTIFieldDataTypeSchema, IReflectable, RTTIFieldDataTypeSchemaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Type, 0)
			B3D_RTTI_MEMBER(HasDynamicSize, 1)
			B3D_RTTI_MEMBER(FixedSize, 2)
			B3D_RTTI_MEMBER(FieldTypeId, 3)
			B3D_RTTI_MEMBER(FieldTypeSchema, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "RTTIFieldDataTypeSchema";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RTTIFieldDataTypeSchema;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<RTTIFieldDataTypeSchema>();
		}
	};

	class RTTIFieldSchemaRTTI : public TRTTIType<RTTIFieldSchema, IReflectable, RTTIFieldSchemaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Id, 0)
			B3D_RTTI_MEMBER(IsContainer, 2)
			B3D_RTTI_MEMBER(Info, 7)
			B3D_RTTI_MEMBER_CONTAINER(FieldDataTypes, 8)
			//B3D_RTTI_MEMBER(IsIterator, 9)
			B3D_RTTI_MEMBER(FieldType, 10)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "RTTIFieldSchema";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RTTIFieldSchema;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<RTTIFieldSchema>();
		}
	};

	class RTTISchemaRTTI : public TRTTIType<RTTISchema, IReflectable, RTTISchemaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(TypeId, 0)
			B3D_RTTI_MEMBER(BaseTypeSchema, 1)
			B3D_RTTI_MEMBER_CONTAINER(FieldSchemas, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "RTTISchema";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace b3d
