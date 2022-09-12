//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsManagedDiff.h"
#include "Serialization/BsManagedCompare.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableObjectRTTI : public RTTIType<ManagedSerializableObject, IReflectable, ManagedSerializableObjectRTTI>
	{
	private:
		SPtr<ManagedSerializableObjectInfo> GetInfo(ManagedSerializableObject* obj)
		{
			return obj->mObjInfo;
		}

		void SetInfo(ManagedSerializableObject* obj, SPtr<ManagedSerializableObjectInfo> val)
		{
			obj->mObjInfo = val;
		}

		SPtr<ManagedSerializableFieldDataEntry> GetFieldEntry(ManagedSerializableObject* obj, UINT32 arrayIdx)
		{
			SPtr<ManagedSerializableMemberInfo> field = mSequentialFields[arrayIdx];

			SPtr<ManagedSerializableFieldKey> fieldKey = ManagedSerializableFieldKey::create(field->mParentTypeId, field->mFieldId);
			SPtr<ManagedSerializableFieldData> fieldData = obj->GetFieldData(field);

			return ManagedSerializableFieldDataEntry::Create(fieldKey, fieldData);
		}

		void SetFieldsEntry(ManagedSerializableObject* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldDataEntry> val)
		{
			obj->mCachedData[*val->mKey] = val->mValue;
		}

		UINT32 GetNumFieldEntries(ManagedSerializableObject* obj)
		{
			return (UINT32)mSequentialFields.Size();
		}

		void SetNumFieldEntries(ManagedSerializableObject* obj, UINT32 numEntries)
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

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedSerializableObject* castObj = static_cast<ManagedSerializableObject*>(obj);

			SPtr<ManagedSerializableObjectInfo> curType = castObj->mObjInfo;
			while (curType != nullptr)
			{
				for (auto& field : curType->mFields)
				{
					if (field.second->IsSerializable())
						mSequentialFields.push_back(field.second);
				}

				curType = curType->mBaseClass;
			}
		}

		IDiff& GetDiffHandler() const override
		{
			static ManagedDiff managedDiffHandler;
			return managedDiffHandler;
		}

		ICompare& GetCompareHandler() const override
		{
			static ManagedCompare managedCompareHandler;
			return managedCompareHandler;
		}

		const String& GetRTTIName() override
		{
			static String name = "ScriptSerializableObject";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ScriptSerializableObject;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return ManagedSerializableObject::CreateEmpty();
		}

	private:
		Vector<SPtr<ManagedSerializableMemberInfo>> mSequentialFields;
	};

	/** @} */
	/** @endcond */
}
