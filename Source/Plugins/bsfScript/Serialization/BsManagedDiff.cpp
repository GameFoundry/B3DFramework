//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedDiff.h"
#include "Serialization/BsManagedSerializableDiff.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Reflection/BsRTTIType.h"
#include "Utility/BsUtility.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	SPtr<SerializedObject> ManagedDiff::generateDiff(const SPtr<SerializedObject>& orgSerzObj,
		const SPtr<SerializedObject>& newSerzObj, ObjectMap& objectMap)
	{
		CoreSerializationContext context;
		context.goState = bs_shared_ptr_new<GameObjectDeserializationState>(GODM_UseOriginalIds | GODM_RestoreExternal);

		const auto orgObj = std::static_pointer_cast<ManagedSerializableObject>(orgSerzObj->decode(&context));
		const auto newObj = std::static_pointer_cast<ManagedSerializableObject>(newSerzObj->decode(&context));

		context.goState->resolve();

		SPtr<ManagedSerializableDiff> diff = ManagedSerializableDiff::create(orgObj, newObj);
		if (diff == nullptr)
			return nullptr;

		SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();
		output->subObjects.push_back(SerializedSubObject());

		SerializedSubObject& subObject = output->subObjects.back();
		subObject.typeId = ManagedSerializableObject::getRTTIStatic()->getRTTIId();

		SerializedEntry entry;
		entry.fieldId = 0;
		entry.serialized = SerializedObject::create(*diff);

		subObject.entries[0] = entry;

		return output;
	}

	void ManagedDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff,
		FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
	{
		SPtr<SerializedObject> diffObj = std::static_pointer_cast<SerializedObject>(serzDiff->subObjects[0].entries[0].serialized);

		SPtr<ManagedSerializableDiff> diff = std::static_pointer_cast<ManagedSerializableDiff>(diffObj->decode(context));
		
		if (diff != nullptr)
		{
			SPtr<ManagedSerializableObject> managedObj = std::static_pointer_cast<ManagedSerializableObject>(object);
			diff->apply(managedObj);
		}
	}
}
