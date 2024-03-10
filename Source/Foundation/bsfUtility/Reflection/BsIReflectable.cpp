//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/BsIReflectable.h"
#include "Reflection/BsRTTIType.h"
#include "Error/BsException.h"
#include "RTTI/BsIReflectableRTTI.h"

using namespace bs;

void IReflectable::RegisterRTTITypeInternal(RTTITypeBase* rttiType)
{
	if(IsTypeIdDuplicateInternal(rttiType->GetRttiId()))
	{
		B3D_EXCEPT(InternalErrorException, "RTTI type \"" + rttiType->GetRttiName() + "\" has a duplicate ID: " + ToString(rttiType->GetRttiId()));
	}

	GetAllRttiTypes()[rttiType->GetRttiId()] = rttiType;
}

SPtr<IReflectable> IReflectable::CreateInstanceFromTypeId(u32 rttiTypeId)
{
	RTTITypeBase* type = GetRTTITypeFromTypeId(rttiTypeId);

	SPtr<IReflectable> output;
	if(type != nullptr)
		output = type->NewRttiObject();

	return output;
}

RTTITypeBase* IReflectable::GetRTTITypeFromTypeId(u32 rttiTypeId)
{
	const auto iterFind = GetAllRttiTypes().find(rttiTypeId);
	if(iterFind != GetAllRttiTypes().end())
		return iterFind->second;

	return nullptr;
}

bool IReflectable::IsTypeIdDuplicateInternal(u32 typeId)
{
	if(typeId == TID_Abstract)
		return false;

	return IReflectable::GetRTTITypeFromTypeId(typeId) != nullptr;
}

bool IReflectable::IsDerivedFrom(const RTTITypeBase* base) const
{
	return GetRtti()->IsDerivedFrom(base);
}

void IReflectable::CheckForCircularReferencesInternal()
{
	Stack<RTTITypeBase*> todo;

	const UnorderedMap<u32, RTTITypeBase*>& allTypes = GetAllRttiTypes();
	for(auto& entry : allTypes)
	{
		RTTITypeBase* myType = entry.second;

		u32 myNumFields = myType->GetFieldCount();
		for(u32 i = 0; i < myNumFields; i++)
		{
			RTTIField* myField = myType->GetField(i);

			if(myField->Schema.Type != SerializableFT_ReflectablePtr)
				continue;

			auto* myReflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(myField);

			RTTITypeBase* otherType = myReflectablePtrField->GetType();
			u32 otherNumFields = otherType->GetFieldCount();
			for(u32 j = 0; j < otherNumFields; j++)
			{
				RTTIField* otherField = otherType->GetField(j);

				if(otherField->Schema.Type != SerializableFT_ReflectablePtr)
					continue;

				auto* otherReflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(otherField);

				if(myType->GetRttiId() == otherReflectablePtrField->GetType()->GetRttiId() &&
				   (!myReflectablePtrField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef) &&
					!otherReflectablePtrField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef)))
				{
					B3D_EXCEPT(InternalErrorException, "Found circular reference on RTTI type: " + myType->GetRttiName() + " to type: " + otherType->GetRttiName() + ". Either remove one of the references or mark it" + " as a weak reference when defining the RTTI field.");
				}
			}
		}
	}
}

u32 IReflectable::GetTypeId() const
{
	return GetRtti()->GetRttiId();
}

const String& IReflectable::GetTypeName() const
{
	return GetRtti()->GetRttiName();
}

RTTITypeBase* IReflectable::GetRttiStatic()
{
	return IReflectableRTTI::Instance();
}
