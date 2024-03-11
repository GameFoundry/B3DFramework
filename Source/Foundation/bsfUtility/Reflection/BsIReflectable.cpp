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
