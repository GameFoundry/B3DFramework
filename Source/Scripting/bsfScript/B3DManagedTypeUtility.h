//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Provides various utility methods for reflection-like operations on managed objects. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT(Static) ManagedTypeUtility
	{
	public:
		/**
		 * Retrieves information about the object's type. Object must be a primitive type, scene object, component, resource, resource reference type, or
		 * a custom type marked with SerializeObject attribute in order for type information to be present. Type information can also be retrieved for
		 * arrays, lists or dictionaries of the above types.
		 */
		B3D_SCRIPT_EXPORT()
		static SPtr<ManagedTypeInfo> GetTypeInfo(MonoReflectionType* objectType);

		/**
		 * Retrieves detailed information about a serializable object. Provided object's type must be marked with a SerializeObject attribute for this information
		 * to be available.
		 */
		B3D_SCRIPT_EXPORT()
		static SPtr<ManagedObjectInfo> GetSerializableObjectInfo(MonoReflectionType* objectType);

		/** Deduces the RTTI ID of the native object that is wrapped by the provided object type. Returns ~0u for types that do not wrap native types. */
		B3D_SCRIPT_EXPORT()
		static u32 GetRTTITypeId(MonoReflectionType* objectType);

		/** Creates a new instance of a serialized object of the provided type. */
		B3D_SCRIPT_EXPORT()
		static MonoObject* CreateSerializableObject(const SPtr<ManagedTypeInfoObject>& typeInfo);

		/** Creates a new instance of an array of the provided type and size. Size array must have an element for each rank of the array type. */
		B3D_SCRIPT_EXPORT()
		static MonoObject* CreateArray(const SPtr<ManagedTypeInfoArray>& typeInfo, const Vector<u32>& arraySizes);

		/** Creates a new instance of a list of the provided type with the provided initial capacity. */
		B3D_SCRIPT_EXPORT()
		static MonoObject* CreateList(const SPtr<ManagedTypeInfoList>& typeInfo, u32 size);

		/** Creates a new instance of a dictionary of the provided type. */
		B3D_SCRIPT_EXPORT()
		static MonoObject* CreateDictionary(const SPtr<ManagedTypeInfoDictionary>& typeInfo);

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
} // namespace b3d
