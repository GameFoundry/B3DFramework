//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsManagedSerializableList.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableListRTTI : public RTTIType<ManagedSerializableList, IReflectable, ManagedSerializableListRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoList> getTypeInfo(ManagedSerializableList* obj)
		{
			return obj->mListTypeInfo;
		}

		void setTypeInfo(ManagedSerializableList* obj, SPtr<ManagedSerializableTypeInfoList> val)
		{
			obj->mListTypeInfo = val;
		}

		UINT32& getNumElements(ManagedSerializableList* obj)
		{
			return (UINT32&)obj->mNumElements;
		}

		void setNumElements(ManagedSerializableList* obj, UINT32& numElements)
		{
			obj->mNumElements = numElements;
		}

		SPtr<ManagedSerializableFieldData> getListEntry(ManagedSerializableList* obj, UINT32 arrayIdx)
		{
			return obj->getFieldData(arrayIdx);
		}

		void setListEntry(ManagedSerializableList* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->setFieldData(arrayIdx, val);
		}

		UINT32 getNumListEntries(ManagedSerializableList* obj)
		{
			return (UINT32)obj->mNumElements;
		}

		void setNumListEntries(ManagedSerializableList* obj, UINT32 numEntries)
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

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableList";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptSerializableList;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableList::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}