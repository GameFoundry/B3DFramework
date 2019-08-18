//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Scene/BsGameObjectManager.h"
#include "Serialization/BsManagedSerializableArray.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableArrayRTTI : public RTTIType<ManagedSerializableArray, IReflectable, ManagedSerializableArrayRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoArray> getTypeInfo(ManagedSerializableArray* obj)
		{
			return obj->mArrayTypeInfo;
		}

		void setTypeInfo(ManagedSerializableArray* obj, SPtr<ManagedSerializableTypeInfoArray> val)
		{
			obj->mArrayTypeInfo = val;
		}

		UINT32& getElementSize(ManagedSerializableArray* obj)
		{
			return (UINT32&)obj->mElemSize;
		}

		void setElementSize(ManagedSerializableArray* obj, UINT32& numElements)
		{
			obj->mElemSize = numElements;
		}

		UINT32& getNumElements(ManagedSerializableArray* obj, UINT32 arrayIdx)
		{
			return (UINT32&)obj->mNumElements[arrayIdx];
		}

		void setNumElements(ManagedSerializableArray* obj, UINT32 arrayIdx, UINT32& numElements)
		{
			obj->mNumElements[arrayIdx] = numElements;
		}

		UINT32 getNumElementsNumEntries(ManagedSerializableArray* obj)
		{
			return (UINT32)obj->mNumElements.size();
		}

		void setNumElementsNumEntries(ManagedSerializableArray* obj, UINT32 numEntries)
		{
			obj->mNumElements.resize(numEntries);
		}

		SPtr<ManagedSerializableFieldData> getArrayEntry(ManagedSerializableArray* obj, UINT32 arrayIdx)
		{
			return obj->getFieldData(arrayIdx);
		}

		void setArrayEntry(ManagedSerializableArray* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->setFieldData(arrayIdx, val);
		}

		UINT32 getNumArrayEntries(ManagedSerializableArray* obj)
		{
			return obj->getTotalLength();
		}

		void setNumArrayEntries(ManagedSerializableArray* obj, UINT32 numEntries)
		{
			obj->mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

	public:
		ManagedSerializableArrayRTTI()
		{
			addReflectablePtrField("mArrayTypeInfo", 0, &ManagedSerializableArrayRTTI::getTypeInfo, &ManagedSerializableArrayRTTI::setTypeInfo);
			addPlainField("mElementSize", 1, &ManagedSerializableArrayRTTI::getElementSize, &ManagedSerializableArrayRTTI::setElementSize);
			addPlainArrayField("mNumElements", 2, &ManagedSerializableArrayRTTI::getNumElements, &ManagedSerializableArrayRTTI::getNumElementsNumEntries,
				&ManagedSerializableArrayRTTI::setNumElements, &ManagedSerializableArrayRTTI::setNumElementsNumEntries);
			addReflectablePtrArrayField("mArrayEntries", 3, &ManagedSerializableArrayRTTI::getArrayEntry, &ManagedSerializableArrayRTTI::getNumArrayEntries,
				&ManagedSerializableArrayRTTI::setArrayEntry, &ManagedSerializableArrayRTTI::setNumArrayEntries);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableArray";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptSerializableArray;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableArray::createNew();
		}
	};

	/** @} */
	/** @endcond */
}
