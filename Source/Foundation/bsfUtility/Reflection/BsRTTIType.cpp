//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsRTTISchemaRTTI.h"
#include "Error/BsException.h"

namespace bs
{
	RTTITypeBase::~RTTITypeBase()
	{
		for(const auto& item : mFields)
			bs_delete(item);
	}

	RTTIField* RTTITypeBase::findField(const String& name)
	{
		auto foundElement = std::find_if(mFields.begin(), mFields.end(), [&name](RTTIField* x) { return x->name == name; });

		if(foundElement == mFields.end())
		{
			BS_EXCEPT(InternalErrorException,
				"Cannot find a field with the specified name: " + name);
		}

		return *foundElement;
	}

	RTTIField* RTTITypeBase::findField(int uniqueFieldId)
	{
		auto foundElement = std::find_if(mFields.begin(), mFields.end(), [&uniqueFieldId](RTTIField* x) { return x->schema.id == uniqueFieldId; });

		if(foundElement == mFields.end())
			return nullptr;

		return *foundElement;
	}

	void RTTITypeBase::addNewField(RTTIField* field)
	{
		if(field == nullptr)
			BS_EXCEPT(InvalidParametersException, "Field argument can't be null.");

		int uniqueId = field->schema.id;
		auto foundElementById = std::find_if(mFields.begin(), mFields.end(), [uniqueId](RTTIField* x) { return x->schema.id == uniqueId; });

		if(foundElementById != mFields.end())
			BS_EXCEPT(InternalErrorException, "Field with the same ID already exists.");

		String& name = field->name;
		auto foundElementByName = std::find_if(mFields.begin(), mFields.end(), [&name](RTTIField* x) { return x->name == name; });

		if(foundElementByName != mFields.end())
			BS_EXCEPT(InternalErrorException, "Field with the same name already exists.");

		mFields.push_back(field);
	}

	void RTTITypeBase::_initSchema()
	{
		mSchema = bs_shared_ptr_new<RTTISchema>();
		mSchema->typeId = getRTTIId();
		
		RTTITypeBase* baseType = getBaseClass();
		if (baseType)
			mSchema->baseTypeSchema = baseType->getSchema();
		
		for (auto& entry : mFields)
		{
			entry->initSchema();
			mSchema->fieldSchemas.push_back(entry->schema);
		}
	}

	RTTITypeBase* RTTISchema::getRTTIStatic()
	{
		return RTTISchemaRTTI::instance();
	}

	RTTITypeBase* RTTISchema::getRTTI() const
	{
		return getRTTIStatic();
	}

	class SerializationContextRTTI : public RTTIType<SerializationContext, IReflectable, SerializationContextRTTI>
	{
		const String& getRTTIName() override
		{
			static String name = "SerializationContext";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializationContext;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	RTTITypeBase* SerializationContext::getRTTIStatic()
	{
		return SerializationContextRTTI::instance();
	}

	RTTITypeBase* SerializationContext::getRTTI() const
	{
		return getRTTIStatic();
	}

	SPtr<IReflectable> rtti_create(UINT32 rttiId)
	{
		return IReflectable::createInstanceFromTypeId(rttiId);
	}
}
