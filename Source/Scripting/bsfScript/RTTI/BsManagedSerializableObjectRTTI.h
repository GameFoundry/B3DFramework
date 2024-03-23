//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsManagedDelta.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableObjectRTTI : public RTTIType<ManagedSerializableObject, IReflectable, ManagedSerializableObjectRTTI>
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

		SPtr<ManagedSerializableFieldDataEntry> GetFieldEntry(ManagedSerializableObject* obj, u32 arrayIdx)
		{
			SPtr<ManagedSerializableMemberInfo> field = mSequentialFields[arrayIdx];

			SPtr<ManagedSerializableFieldKey> fieldKey = ManagedSerializableFieldKey::Create(field->MParentTypeId, field->MFieldId);
			SPtr<ManagedSerializableFieldData> fieldData = obj->GetFieldData(field);

			return ManagedSerializableFieldDataEntry::Create(fieldKey, fieldData);
		}

		void SetFieldsEntry(ManagedSerializableObject* obj, u32 arrayIdx, SPtr<ManagedSerializableFieldDataEntry> val)
		{
			obj->mCachedData[*val->MKey] = val->MValue;
		}

		u32 GetNumFieldEntries(ManagedSerializableObject* obj)
		{
			return (u32)mSequentialFields.size();
		}

		void SetNumFieldEntries(ManagedSerializableObject* obj, u32 numEntries)
		{
			// Do nothing
		}

	public:
		ManagedSerializableObjectRTTI()
		{
			AddReflectablePtrField("mObjInfo", 0, &ManagedSerializableObjectRTTI::GetInfo, &ManagedSerializableObjectRTTI::SetInfo);
			AddReflectablePtrArrayField("mFieldEntries", 1, &ManagedSerializableObjectRTTI::GetFieldEntry, &ManagedSerializableObjectRTTI::GetNumFieldEntries, &ManagedSerializableObjectRTTI::SetFieldsEntry, &ManagedSerializableObjectRTTI::SetNumFieldEntries);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context)
		{
			ManagedSerializableObject* castObj = static_cast<ManagedSerializableObject*>(obj);

			SPtr<ManagedSerializableObjectInfo> curType = castObj->mObjInfo;
			while(curType != nullptr)
			{
				for(auto& field : curType->MFields)
				{
					if(field.second->IsSerializable())
						mSequentialFields.push_back(field.second);
				}

				curType = curType->MBaseClass;
			}
		}

		IDeltaHandler& GetDeltaHandler() const
		{
			static ManagedDeltaHandler managedDiffHandler;
			return managedDiffHandler;
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptSerializableObject;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedSerializableObject::CreateEmpty();
		}

	private:
		Vector<SPtr<ManagedSerializableMemberInfo>> mSequentialFields;
	};

	/** @} */
	/** @endcond */
} // namespace bs
