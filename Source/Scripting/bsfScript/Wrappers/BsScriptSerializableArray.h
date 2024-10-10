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

	/**	Interop class between C++ & CLR for ManagedSerializableArray. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSerializableArray : public ScriptObject<ScriptSerializableArray>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SerializableArray")

		/**
		 * Creates a new serializable array interop object from the data in the provided property. Caller must ensure the
		 * property references an array.
		 */
		static MonoObject* Create(const ScriptSerializableProperty* native, MonoObject* managed);

	private:
		ScriptSerializableArray(MonoObject* instance, const SPtr<ManagedTypeInfoArray>& typeInfo);

		SPtr<ManagedTypeInfoArray> mTypeInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalCreateProperty(ScriptSerializableArray* nativeInstance);
	};

	/** @} */
} // namespace bs
