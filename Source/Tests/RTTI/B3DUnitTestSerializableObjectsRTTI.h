//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "RTTI/B3DStringRTTI.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	class UnitTestSerializationObjectARTTI : public TRTTIType<UnitTestSerializationObjectA, IReflectable, UnitTestSerializationObjectARTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(IntA, 0)
			B3D_RTTI_MEMBER(StrA, 1)
			B3D_RTTI_MEMBER(StrB, 2)

			B3D_RTTI_MEMBER(ObjA, 3)
			B3D_RTTI_MEMBER(ObjB, 4)

			B3D_RTTI_MEMBER(ObjPtrA, 5)
			B3D_RTTI_MEMBER(ObjPtrB, 6)
			B3D_RTTI_MEMBER(ObjPtrC, 7)
			B3D_RTTI_MEMBER(ObjPtrD, 8)
			B3D_RTTI_MEMBER(ObjPtrE, 9)

			B3D_RTTI_MEMBER_CONTAINER(ArrStrA, 10)
			B3D_RTTI_MEMBER_CONTAINER(ArrStrB, 11)
			B3D_RTTI_MEMBER_CONTAINER(ArrStrC, 12)

			B3D_RTTI_MEMBER_CONTAINER(ArrObjA, 13)
			B3D_RTTI_MEMBER_CONTAINER(ArrObjB, 14)

			B3D_RTTI_MEMBER_CONTAINER(ArrObjPtrA, 15)
			B3D_RTTI_MEMBER_CONTAINER(ArrObjPtrB, 16)

			B3D_RTTI_MEMBER_CONTAINER(PlainMapA, 17)
			B3D_RTTI_MEMBER_CONTAINER(PlainMapB, 18)
			B3D_RTTI_MEMBER_CONTAINER(PlainMapC, 19)
			B3D_RTTI_MEMBER_CONTAINER(PlainMapD, 20)
			B3D_RTTI_MEMBER_CONTAINER(PlainMapE, 21)

			B3D_RTTI_MEMBER_CONTAINER(ObjectMapA, 22)
			B3D_RTTI_MEMBER_CONTAINER(ObjectMapB, 23)
			B3D_RTTI_MEMBER_CONTAINER(ObjectMapC, 24)

			B3D_RTTI_MEMBER_CONTAINER(ObjectPointerMap, 25)

		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "UnitTestSerializationObjectA";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_UnitTestSerializationObjectA;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<UnitTestSerializationObjectA>();
		}
	};

	class UnitTestSerializationObjectBRTTI : public TRTTIType<UnitTestSerializationObjectB, IReflectable, UnitTestSerializationObjectBRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(IntA, 0)
			B3D_RTTI_MEMBER(StrA, 1)
		B3D_RTTI_END_MEMBERS

		TShared<DataStream> GetDataBlock(UnitTestSerializationObjectB* obj, u32& size)
		{
			size = (u32)obj->DataBlock.size();
			return B3DMakeShared<MemoryDataStream>(obj->DataBlock.data(), size);
		}

		void SetDataBlock(UnitTestSerializationObjectB* obj, const TShared<DataStream>& value, u32 size)
		{
			// Record the stream's reported size so tests can verify the serializer's in-memory data-block path hands the
			// consumer a correctly-sized stream (regression guard for the MemoryDataStream capacity-ctor Size() fix).
			obj->DataBlockStreamSize = (u32)value->Size();

			obj->DataBlock.resize(size);
			if(size > 0)
				value->Read(obj->DataBlock.data(), size);
		}

	public:
		UnitTestSerializationObjectBRTTI()
		{
			AddDataBlockField("dataBlock", 2, &UnitTestSerializationObjectBRTTI::GetDataBlock, &UnitTestSerializationObjectBRTTI::SetDataBlock);
		}

		const String& GetRttiName() override
		{
			static String name = "UnitTestSerializationObjectB";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_UnitTestSerializationObjectB;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<UnitTestSerializationObjectB>();
		}
	};
}
