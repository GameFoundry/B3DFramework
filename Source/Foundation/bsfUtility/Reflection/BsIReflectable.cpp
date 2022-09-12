//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/BsIReflectable.h"
#include "Reflection/BsRTTIType.h"
#include "Error/BsException.h"
#include "RTTI/BsIReflectableRTTI.h"

namespace bs
{
	void IReflectable::_registerRTTIType(RTTITypeBase* rttiType)
	{
		if(_isTypeIdDuplicate(rttiType->GetRTTIId()))
		{
			BS_EXCEPT(InternalErrorException, "RTTI type \"" + rttiType->GetRTTIName() +
											  "\" has a duplicate ID: " + ToString(rttiType->GetRTTIId()));
		}

		getAllRTTITypes()[rttiType->GetRTTIId()] = rttiType;
	}

	SPtr<IReflectable> IReflectable::CreateInstanceFromTypeId(UINT32 rttiTypeId)
	{
		RTTITypeBase* type = _getRTTIfromTypeId(rttiTypeId);

		SPtr<IReflectable> output;
		if(type != nullptr)
			output = type->NewRTTIObject();
		
		return output;
	}

	RTTITypeBase* IReflectable::_getRTTIfromTypeId(UINT32 rttiTypeId)
	{
		const auto iterFind = getAllRTTITypes().find(rttiTypeId);
		if(iterFind != getAllRTTITypes().end())
			return iterFind->second;

		return nullptr;
	}

	bool IReflectable::_isTypeIdDuplicate(UINT32 typeId)
	{
		if(typeId == TID_Abstract)
			return false;

		return IReflectable::_getRTTIfromTypeId(typeId) != nullptr;
	}

	bool IReflectable::IsDerivedFrom(RTTITypeBase* base)
	{
		return GetRTTI()->IsDerivedFrom(base);
	}

	void IReflectable::_checkForCircularReferences()
	{
		Stack<RTTITypeBase*> todo;

		const UnorderedMap<UINT32, RTTITypeBase*>& allTypes = getAllRTTITypes();
		for(auto& entry : allTypes)
		{
			RTTITypeBase* myType = entry.second;

			UINT32 myNumFields = myType->GetNumFields();
			for (UINT32 i = 0; i < myNumFields; i++)
			{
				RTTIField* myField = myType->GetField(i);

				if (myField->schema.type != SerializableFT_ReflectablePtr)
					continue;

				auto* myReflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(myField);
				
				RTTITypeBase* otherType = myReflectablePtrField->GetType();
				UINT32 otherNumFields = otherType->GetNumFields();
				for (UINT32 j = 0; j < otherNumFields; j++)
				{
					RTTIField* otherField = otherType->GetField(j);

					if (otherField->schema.type != SerializableFT_ReflectablePtr)
						continue;

					auto* otherReflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(otherField);

					if (myType->GetRTTIId() == otherReflectablePtrField->getType()->getRTTIId() &&
						(!myReflectablePtrField->schema.info.flags.IsSet(RTTIFieldFlag::WeakRef) &&
						!otherReflectablePtrField->schema.info.flags.IsSet(RTTIFieldFlag::WeakRef)))
					{
						BS_EXCEPT(InternalErrorException, "Found circular reference on RTTI type: " + myType->GetRTTIName()
							+ " to type: " + otherType->GetRTTIName() + ". Either remove one of the references or mark it"
							+ " as a weak reference when defining the RTTI field.");
					}
				}
			}
		}
	}

	UINT32 IReflectable::GetTypeId() const
	{
		return GetRTTI()->GetRTTIId();
	}

	const String& IReflectable::GetTypeName() const
	{
		return GetRTTI()->GetRTTIName();
	}

	RTTITypeBase* IReflectable::getRTTIStatic()
	{
		return IReflectableRTTI::Instance();
	}
}
