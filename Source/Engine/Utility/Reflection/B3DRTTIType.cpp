//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DRTTISchemaRTTI.h"

using namespace b3d;

RTTIType::~RTTIType()
{
	for(const auto& item : mFields)
		B3DDelete(item);
}

RTTIField* RTTIType::FindField(const String& name)
{
	auto foundElement = std::find_if(mFields.begin(), mFields.end(), [&name](RTTIField* x)
									 { return x->Name == name; });

	if(!B3D_CHECK_LOG(foundElement != mFields.end(), "Cannot find a file with the specified name."))
		return nullptr;

	return *foundElement;
}

RTTIField* RTTIType::FindField(int uniqueFieldId)
{
	auto foundElement = std::find_if(mFields.begin(), mFields.end(), [&uniqueFieldId](RTTIField* x)
									 { return x->Schema.Id == uniqueFieldId; });

	if(foundElement == mFields.end())
		return nullptr;

	return *foundElement;
}

void RTTIType::AddNewField(RTTIField* field)
{
	if(!B3D_CHECK_LOG(field != nullptr, "Field argument can't be null."))
		return;

	int uniqueId = field->Schema.Id;
	auto foundElementById = std::find_if(mFields.begin(), mFields.end(), [uniqueId](RTTIField* x)
										 { return x->Schema.Id == uniqueId; });

	if(!B3D_CHECK_LOG(foundElementById == mFields.end(), "Field with the same ID already exists."))
		return;

	String& name = field->Name;
	auto foundElementByName = std::find_if(mFields.begin(), mFields.end(), [&name](RTTIField* x)
										   { return x->Name == name; });

	if(!B3D_CHECK_LOG(foundElementByName == mFields.end(), "Field with the same name already exists."))
		return;

	mFields.push_back(field);
}

void RTTIType::InitSchemaInternal()
{
	mSchema = B3DMakeShared<RTTISchema>();
	mSchema->TypeId = GetRttiId();

	RTTIType* baseType = GetBaseClass();
	if(baseType)
		mSchema->BaseTypeSchema = baseType->GetSchema();

	for(auto& entry : mFields)
	{
		entry->InitSchema();
		mSchema->FieldSchemas.push_back(entry->Schema);
	}
}

RTTIType* RTTISchema::GetRttiStatic()
{
	return RTTISchemaRTTI::Instance();
}

RTTIType* RTTISchema::GetRtti() const
{
	return GetRttiStatic();
}

class RTTIOperationContextRTTI : public TRTTIType<RTTIOperationContext, IReflectable, RTTIOperationContextRTTI>
{
	const String& GetRttiName() override
	{
		static String name = "RTTIOperationContext";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_RTTIOperationContext;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		B3D_ASSERT(false && "Cannot instantiate an abstract class.");
		return nullptr;
	}
};

RTTIType* RTTIOperationContext::GetRttiStatic()
{
	return RTTIOperationContextRTTI::Instance();
}

RTTIType* RTTIOperationContext::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d
{
SPtr<IReflectable> B3DRTTICreate(u32 rttiId)
{
	return IReflectable::CreateInstanceFromTypeId(rttiId);
}
} // namespace b3d
