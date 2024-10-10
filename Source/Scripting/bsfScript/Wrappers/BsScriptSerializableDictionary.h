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

	/**	Interop class between C++ & CLR for ManagedSerializableDictionary. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSerializableDictionary : public ScriptObject<ScriptSerializableDictionary>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SerializableDictionary")

		/**
		 * Creates a new serializable dictionary interop object from the data in the provided property. Caller must ensure
		 * the property references a System.Collections.Generic.Dictionary.
		 */
		static MonoObject* Create(const ScriptSerializableProperty* native, MonoObject* managed);

	private:
		ScriptSerializableDictionary(MonoObject* instance, const SPtr<ManagedTypeInfoDictionary>& typeInfo);

		SPtr<ManagedTypeInfoDictionary> mTypeInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalCreateKeyProperty(ScriptSerializableDictionary* nativeInstance);
		static MonoObject* InternalCreateValueProperty(ScriptSerializableDictionary* nativeInstance);
	};

	/** @} */
} // namespace bs
