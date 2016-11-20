//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
        bool hasRange; /**< True if the range of the field is limited, false if unlimited. */
        float rangeMin; /**< Returns the lower bound of the range. Only relevant if @see hasRange is true. */
        float rangeMax; /**< Returns the upper bound of the range. Only relevant if @see hasRange is true. */
        bool hasStep; /**< True if the field value can only be incremented in specific increments. */
		/** Minimum increment the field value can be increment/decremented by. Only relevant if @see hasStep is true. */
        float stepIncrement; 
        bool displayAsSlider; /**< If true, number fields will be displayed as sliders instead of regular input boxes. */
	};

	/**	Interop class between C++ & CLR for ManagedSerializableFieldInfo. */
	class BS_SCR_BE_EXPORT ScriptSerializableField : public ScriptObject<ScriptSerializableField>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableField")

		/**
		 * Creates a new serializable field interop object that references a specific field on a specific object.
		 *
		 * @param[in]	parentObject	Instance of the parent object the field belongs to.
		 * @param[in]	fieldInfo		Information about the field. Caller must ensure the type matches the type of the
		 *								provided parent object.
		 */
		static ScriptSerializableField* create(MonoObject* parentObject, const SPtr<ManagedSerializableMemberInfo>& fieldInfo);
	private:
		ScriptSerializableField(MonoObject* instance, const SPtr<ManagedSerializableMemberInfo>& fieldInfo);

		SPtr<ManagedSerializableMemberInfo> mFieldInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_createProperty(ScriptSerializableField* nativeInstance);
		static MonoObject* internal_getValue(ScriptSerializableField* nativeInstance, MonoObject* instance);
		static void internal_setValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value);
		static void internal_getStyle(ScriptSerializableField* nativeInstance, SerializableMemberStyle* style);
	};

	/** @} */
}