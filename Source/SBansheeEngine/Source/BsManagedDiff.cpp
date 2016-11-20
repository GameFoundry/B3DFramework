//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedDiff.h"
#include "BsManagedSerializableDiff.h"
#include "BsBinarySerializer.h"
#include "BsMemorySerializer.h"
#include "BsManagedSerializableObject.h"
#include "BsGameObjectManager.h"
#include "BsRTTIType.h"

namespace bs
{
	SPtr<SerializedObject> ManagedDiff::generateDiff(const SPtr<SerializedObject>& orgSerzObj,
		const SPtr<SerializedObject>& newSerzObj, ObjectMap& objectMap)
	{
		BinarySerializer bs;

		// Need to call GameObjectManager because GameObject handles call it during deserialization, but we don't really need it
		GameObjectManager::instance().startDeserialization();
		SPtr<ManagedSerializableObject> orgObj = std::static_pointer_cast<ManagedSerializableObject>(bs._decodeFromIntermediate(orgSerzObj));
		SPtr<ManagedSerializableObject> newObj = std::static_pointer_cast<ManagedSerializableObject>(bs._decodeFromIntermediate(newSerzObj));
		GameObjectManager::instance().endDeserialization();

		SPtr<ManagedSerializableDiff> diff = ManagedSerializableDiff::create(orgObj, newObj);
		if (diff == nullptr)
			return nullptr;

		SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();
		output->subObjects.push_back(SerializedSubObject());

		SerializedSubObject& subObject = output->subObjects.back();
		subObject.typeId = ManagedSerializableObject::getRTTIStatic()->getRTTIId();

		SerializedEntry entry;
		entry.fieldId = 0;
		entry.serialized = bs._encodeToIntermediate(diff.get());

		subObject.entries[0] = entry;

		return output;
	}

	void ManagedDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff,
		DiffObjectMap& objectMap, Vector<DiffCommand>& diffCommands)
	{
		SPtr<SerializedObject> diffObj = std::static_pointer_cast<SerializedObject>(serzDiff->subObjects[0].entries[0].serialized);

		BinarySerializer bs;
		SPtr<ManagedSerializableDiff> diff = std::static_pointer_cast<ManagedSerializableDiff>(bs._decodeFromIntermediate(diffObj));
		
		if (diff != nullptr)
		{
			SPtr<ManagedSerializableObject> managedObj = std::static_pointer_cast<ManagedSerializableObject>(object);
			diff->apply(managedObj);
		}
	}
}