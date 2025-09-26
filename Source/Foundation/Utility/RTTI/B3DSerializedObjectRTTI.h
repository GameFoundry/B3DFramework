//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Serialization/B3DSerializedObject.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class B3D_EXPORT ISerializedRTTI : public TRTTIType<ISerialized, IReflectable, ISerializedRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "ISerialized";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ISerialized;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return nullptr;
		}
	};

	class B3D_EXPORT SerializedPlainDataRTTI : public TRTTIType<SerializedPlainData, ISerialized, SerializedPlainDataRTTI>
	{
	private:
		SPtr<DataStream> GetData(SerializedPlainData* obj, u32& size)
		{
			size = obj->Size;

			return B3DMakeShared<MemoryDataStream>(obj->Value, obj->Size);
		}

		void SetData(SerializedPlainData* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->Value = (u8*)B3DAllocate(size);
			obj->Size = size;
			obj->OwnsMemory = true;

			value->Read(obj->Value, size);
		}

	public:
		SerializedPlainDataRTTI()
		{
			AddDataBlockField("data", 0, &SerializedPlainDataRTTI::GetData, &SerializedPlainDataRTTI::SetData);
		}

		const String& GetRttiName()
		{
			static String name = "SerializedPlainData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedPlainData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SerializedPlainData>();
		}
	};

	class B3D_EXPORT SerializedDataBlockRTTI : public TRTTIType<SerializedDataBlock, ISerialized, SerializedDataBlockRTTI>
	{
	private:
		SPtr<DataStream> GetData(SerializedDataBlock* obj, u32& size)
		{
			size = obj->Size;
			obj->Stream->Seek(obj->Offset);

			return obj->Stream;
		}

		void SetData(SerializedDataBlock* obj, const SPtr<DataStream>& value, u32 size)
		{
			SPtr<MemoryDataStream> memStream = B3DMakeShared<MemoryDataStream>(size);
			value->Read(memStream->Data(), size);

			obj->Stream = memStream;
			obj->Size = size;
			obj->Offset = 0;
		}

	public:
		SerializedDataBlockRTTI()
		{
			AddDataBlockField("data", 0, &SerializedDataBlockRTTI::GetData, &SerializedDataBlockRTTI::SetData);
		}

		const String& GetRttiName()
		{
			static String name = "SerializedDataBlock";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedDataBlock;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SerializedDataBlock>();
		}
	};

	class B3D_EXPORT SerializedSubObjectRTTI : public TRTTIType<SerializedSubObject, IReflectable, SerializedSubObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(TypeId, 0)
			B3D_RTTI_MEMBER_CONTAINER(FieldEntries, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedSubObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedSubObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedSubObject>();
		}
	};

	class B3D_EXPORT SerializedObjectRTTI : public TRTTIType<SerializedObject, ISerialized, SerializedObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(SubObjects, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedObject>();
		}
	};

	class B3D_EXPORT SerializedArrayRTTI : public TRTTIType<SerializedArray, ISerialized, SerializedArrayRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Entries, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedArray";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedArray>();
		}
	};

	class B3D_EXPORT SerializedMapRTTI : public TRTTIType<SerializedMap, ISerialized, SerializedMapRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Entries, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedMap";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedMap;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedMap>();
		}
	};

	class B3D_EXPORT SerializedTupleRTTI : public TRTTIType<SerializedTuple, ISerialized, SerializedTupleRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Values, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedTuple";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedTuple;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedTuple>();
		}
	};

	class B3D_EXPORT SerializedFieldRTTI : public TRTTIType<SerializedField, IReflectable, SerializedFieldRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(FieldId, 0)
			B3D_RTTI_MEMBER(Value, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedField";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedField;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedField>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
