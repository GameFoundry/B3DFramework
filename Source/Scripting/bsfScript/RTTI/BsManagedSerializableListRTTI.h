//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "Serialization/BsManagedSerializableList.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableListRTTI : public RTTIType<ManagedSerializableList, IReflectable, ManagedSerializableListRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoList> GetTypeInfo(ManagedSerializableList* obj)
		{
			return obj->mListTypeInfo;
		}

		void SetTypeInfo(ManagedSerializableList* obj, SPtr<ManagedSerializableTypeInfoList> val)
		{
			obj->mListTypeInfo = val;
		}

		UINT32& GetNumElements(ManagedSerializableList* obj)
		{
			return (UINT32&)obj->mNumElements;
		}

		void SetNumElements(ManagedSerializableList* obj, UINT32& numElements)
		{
			obj->mNumElements = numElements;
		}

		SPtr<ManagedSerializableFieldData> GetListEntry(ManagedSerializableList* obj, UINT32 arrayIdx)
		{
			return obj->GetFieldData(arrayIdx);
		}

		void SetListEntry(ManagedSerializableList* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->SetFieldData(arrayIdx, val);
		}

		UINT32 GetNumListEntries(ManagedSerializableList* obj)
		{
			return (UINT32)obj->mNumElements;
		}

		void SetNumListEntries(ManagedSerializableList* obj, UINT32 numEntries)
		{
			obj->mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

	public:
		ManagedSerializableListRTTI()
		{
			addReflectablePtrField("mListTypeInfo", 0, &ManagedSerializableListRTTI::getTypeInfo, &ManagedSerializableListRTTI::setTypeInfo);
			addPlainField("mNumElements", 1, &ManagedSerializableListRTTI::getNumElements, &ManagedSerializableListRTTI::setNumElements);
			addReflectablePtrArrayField("mListEntries", 2, &ManagedSerializableListRTTI::getListEntry, &ManagedSerializableListRTTI::getNumListEntries,
				&ManagedSerializableListRTTI::setListEntry, &ManagedSerializableListRTTI::setNumListEntries);
		}

		const String& GetRTTIName() override
		{
			static String name = "ScriptSerializableList";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ScriptSerializableList;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return ManagedSerializableList::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
