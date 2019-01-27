//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsManagedDiff.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableObjectRTTI : public RTTIType<ManagedSerializableObject, IReflectable, ManagedSerializableObjectRTTI>
	{
	private:
		SPtr<ManagedSerializableObjectInfo> getInfo(ManagedSerializableObject* obj)
		{
			return obj->mObjInfo;
		}

		void setInfo(ManagedSerializableObject* obj, SPtr<ManagedSerializableObjectInfo> val)
		{
			obj->mObjInfo = val;
		}

		SPtr<ManagedSerializableFieldDataEntry> getFieldEntry(ManagedSerializableObject* obj, UINT32 arrayIdx)
		{
			SPtr<ManagedSerializableMemberInfo> field = mSequentialFields[arrayIdx];

			SPtr<ManagedSerializableFieldKey> fieldKey = ManagedSerializableFieldKey::create(field->mParentTypeId, field->mFieldId);
			SPtr<ManagedSerializableFieldData> fieldData = obj->getFieldData(field);

			return ManagedSerializableFieldDataEntry::create(fieldKey, fieldData);
		}

		void setFieldsEntry(ManagedSerializableObject* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldDataEntry> val)
		{
			obj->mCachedData[*val->mKey] = val->mValue;
		}

		UINT32 getNumFieldEntries(ManagedSerializableObject* obj)
		{
			return (UINT32)mSequentialFields.size();
		}

		void setNumFieldEntries(ManagedSerializableObject* obj, UINT32 numEntries)
		{
			// Do nothing
		}

	public:
		ManagedSerializableObjectRTTI()
		{
			addReflectablePtrField("mObjInfo", 0, &ManagedSerializableObjectRTTI::getInfo, &ManagedSerializableObjectRTTI::setInfo);
			addReflectablePtrArrayField("mFieldEntries", 1, &ManagedSerializableObjectRTTI::getFieldEntry, &ManagedSerializableObjectRTTI::getNumFieldEntries, 
				&ManagedSerializableObjectRTTI::setFieldsEntry, &ManagedSerializableObjectRTTI::setNumFieldEntries);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedSerializableObject* castObj = static_cast<ManagedSerializableObject*>(obj);

			SPtr<ManagedSerializableObjectInfo> curType = castObj->mObjInfo;
			while (curType != nullptr)
			{
				for (auto& field : curType->mFields)
				{
					if (field.second->isSerializable())
						mSequentialFields.push_back(field.second);
				}

				curType = curType->mBaseClass;
			}
		}

		IDiff& getDiffHandler() const override
		{
			static ManagedDiff managedDiffHandler;
			return managedDiffHandler;
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptSerializableObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableObject::createEmpty();
		}

	private:
		Vector<SPtr<ManagedSerializableMemberInfo>> mSequentialFields;
	};

	/** @} */
	/** @endcond */
}