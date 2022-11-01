//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsRTTISchemaRTTI.h"
#include "Error/BsException.h"

using namespace bs;

RTTITypeBase::~RTTITypeBase()
{
	for(const auto& item : mFields)
		B3DDelete(item);
}

RTTIField* RTTITypeBase::FindField(const String& name)
{
	auto foundElement = std::find_if(mFields.begin(), mFields.end(), [&name](RTTIField* x)
									 { return x->Name == name; });

	if(foundElement == mFields.end())
	{
		BS_EXCEPT(InternalErrorException, "Cannot find a field with the specified name: " + name);
	}

	return *foundElement;
}

RTTIField* RTTITypeBase::FindField(int uniqueFieldId)
{
	auto foundElement = std::find_if(mFields.begin(), mFields.end(), [&uniqueFieldId](RTTIField* x)
									 { return x->Schema.Id == uniqueFieldId; });

	if(foundElement == mFields.end())
		return nullptr;

	return *foundElement;
}

void RTTITypeBase::AddNewField(RTTIField* field)
{
	if(field == nullptr)
		BS_EXCEPT(InvalidParametersException, "Field argument can't be null.");

	int uniqueId = field->Schema.Id;
	auto foundElementById = std::find_if(mFields.begin(), mFields.end(), [uniqueId](RTTIField* x)
										 { return x->Schema.Id == uniqueId; });

	if(foundElementById != mFields.end())
		BS_EXCEPT(InternalErrorException, "Field with the same ID already exists.");

	String& name = field->Name;
	auto foundElementByName = std::find_if(mFields.begin(), mFields.end(), [&name](RTTIField* x)
										   { return x->Name == name; });

	if(foundElementByName != mFields.end())
		BS_EXCEPT(InternalErrorException, "Field with the same name already exists.");

	mFields.push_back(field);
}

void RTTITypeBase::InitSchemaInternal()
{
	mSchema = B3DMakeShared<RTTISchema>();
	mSchema->TypeId = GetRttiId();

	RTTITypeBase* baseType = GetBaseClass();
	if(baseType)
		mSchema->BaseTypeSchema = baseType->GetSchema();

	for(auto& entry : mFields)
	{
		entry->InitSchema();
		mSchema->FieldSchemas.push_back(entry->Schema);
	}
}

RTTITypeBase* RTTISchema::GetRttiStatic()
{
	return RTTISchemaRTTI::Instance();
}

RTTITypeBase* RTTISchema::GetRtti() const
{
	return GetRttiStatic();
}

class SerializationContextRTTI : public RTTIType<SerializationContext, IReflectable, SerializationContextRTTI>
{
	const String& GetRttiName() override
	{
		static String name = "SerializationContext";
		return name;
	}

	u32 GetRttiId() override
	{
		return TID_SerializationContext;
	}

	SPtr<IReflectable> NewRttiObject()
	{
		BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
		return nullptr;
	}
};

RTTITypeBase* SerializationContext::GetRttiStatic()
{
	return SerializationContextRTTI::Instance();
}

RTTITypeBase* SerializationContext::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs
{
SPtr<IReflectable> B3DRTTICreate(u32 rttiId)
{
	return IReflectable::CreateInstanceFromTypeId(rttiId);
}
} // namespace bs
