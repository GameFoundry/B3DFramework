//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Contains information about a style of a serializable field. */
	struct SerializableMemberStyle // Note: Must match C# struct SerializableFieldStyle
	{
		/** Returns the lower bound of the range. Only relevant if @see hasRange is true. */
		float RangeMin = 0;

		/** Returns the upper bound of the range. Only relevant if @see hasRange is true. */
		float RangeMax = 0;

		/** Minimum increment the field value can be increment/decremented by. Only relevant if @see hasStep is true. */
		float StepIncrement = 0;

		/** If true, number fields will be displayed as sliders instead of regular input boxes. */
		bool DisplayAsSlider = false;

		/** Name of the category to display in inspector, if the member is part of one. */
		MonoString* CategoryName = nullptr;

		/** Determines ordering in inspector relative to other members. */
		int Order = 0;
	};

	/**	Interop class between C++ & CLR for ManagedSerializableFieldInfo. */
	class BS_SCR_BE_EXPORT ScriptSerializableField : public ScriptObject<ScriptSerializableField>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SerializableField")

		/**
		 * Creates a new serializable field interop object that references a specific field on a specific object.
		 *
		 * @param[in]	parentObject	Instance of the parent object the field belongs to.
		 * @param[in]	fieldInfo		Information about the field. Caller must ensure the type matches the type of the
		 *								provided parent object.
		 */
		static MonoObject* Create(MonoObject* parentObject, const SPtr<ManagedSerializableMemberInfo>& fieldInfo);

	private:
		ScriptSerializableField(MonoObject* instance, const SPtr<ManagedSerializableMemberInfo>& fieldInfo);

		SPtr<ManagedSerializableMemberInfo> mFieldInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalCreateProperty(ScriptSerializableField* nativeInstance);
		static MonoObject* InternalGetValue(ScriptSerializableField* nativeInstance, MonoObject* instance);
		static void InternalSetValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value);
		static void InternalGetStyle(ScriptSerializableField* nativeInstance, SerializableMemberStyle* style);
	};

	/** Interop class between C++ & CLR for SerializableFieldStyle. */
	class BS_SCR_BE_EXPORT ScriptSerializableFieldStyle : public ScriptObject<ScriptSerializableFieldStyle>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SerializableFieldStyle")

	private:
		ScriptSerializableFieldStyle(MonoObject* managedInstance);
	};

	/** @} */
} // namespace bs
