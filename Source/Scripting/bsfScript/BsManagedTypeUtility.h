//********************************* bs::framework - Copyright 2024 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Provides various utility methods for reflection-like operations on managed objects. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT(Static) ManagedTypeUtility
	{
	public:
		/** Deduces information about the object's type. Object must be serializable in order for type information to be present. */
		B3D_SCRIPT_EXPORT()
		static SPtr<ManagedTypeInfo> GetTypeInfo(MonoObject* scriptObject);

		/**
		 * Clones the specified object. Non-serializable types and fields are ignored in clone. A deep copy is performed
		 * on all serializable elements except for resources or game objects.
		 *
		 * @param	original	Non-null reference to the object to clone. Object type must be serializable.
		 * @return				Deep copy of the original object
		 */
		B3D_SCRIPT_EXPORT()
		static MonoObject* CloneObject(MonoObject* original);

		/**
		 * Creates an uninitialized object of the specified type.
		 *
		 * @param	type	Type of the object to create. Must be serializable
		 * @return			New instance of the specified type, or null if the type is not serializable.
		 */
		B3D_SCRIPT_EXPORT()
		static MonoObject* CreateObjectOfType(MonoReflectionType* type);
	};

	/** @} */
} // namespace bs
