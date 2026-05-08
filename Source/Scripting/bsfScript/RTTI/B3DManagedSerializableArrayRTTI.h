//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Scene/B3DGameObjectManager.h"
#include "Serialization/B3DManagedSerializableArray.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DMonoManager.h"
#include "B3DMonoClass.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableArrayRTTI : public TRTTIType<ManagedSerializableArray, IReflectable, ManagedSerializableArrayRTTI>
	{
		TArray<TShared<ManagedSerializableFieldData>> mArrayEntries;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mArrayTypeInfo, 0)
			B3D_RTTI_MEMBER(mElemSize, 1)
			B3D_RTTI_MEMBER_CONTAINER(mNumElements, 2)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mArrayEntries, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedSerializableArray& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				const u32 arrayLength = object.GetTotalLength();
				mArrayEntries.reserve(arrayLength);

				for(u32 arrayElementIndex = 0; arrayElementIndex < arrayLength; ++arrayElementIndex)
					mArrayEntries.Add(object.GetFieldData(arrayElementIndex));
			}
		}

		void OnOperationEnded(ManagedSerializableArray& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				const u32 arrayLength = (u32)mArrayEntries.size();
				object.mCachedEntries.resize(arrayLength);

				for(u32 arrayElementIndex = 0; arrayElementIndex < arrayLength; ++arrayElementIndex)
					object.SetFieldData(arrayElementIndex, mArrayEntries[arrayElementIndex]);
			}
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableArray";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptSerializableArray;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return ManagedSerializableArray::CreateNew();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
