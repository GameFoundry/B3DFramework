//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"

namespace b3d
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**
	 * Encapsulates information about a single Mono (managed) field belonging to some managed class. This object also
	 * allows you to set or retrieve values to/from specific instances containing the field.
	 */
	class B3D_MONO_EXPORT MonoField
	{
	public:
		/**	Returns the name of the field. */
		const String& GetName() const { return mName; }

		/**	Returns the class representing the type of data the field holds. */
		MonoClass* GetType();

		/**
		 * Retrieves value currently set in the field on the specified object instance. If field is static object instance
		 * can be null.
		 *
		 * @note
		 * Value will be a pointer to raw data type for value types (for example int, float), and a MonoObject* for
		 * reference types.
		 */
		void Get(MonoObject* instance, void* outValue);

		/**
		 * Retrieves value currently set in the field on the specified object instance. If field is static object instance
		 * can be null. If returned value is a value type it will be boxed.
		 */
		MonoObject* GetBoxed(MonoObject* instance);

		/**
		 * Sets a value for the field on the specified object instance. If field is static object instance can be null.
		 *
		 * @note
		 * Value should be a pointer to raw data type for value types (for example int, float), and a MonoObject* for
		 * reference types.
		 */
		void Set(MonoObject* instance, void* value);

		/**	Checks if field has an attribute of the specified type. */
		bool HasAttribute(MonoClass* monoClass);

		/**
		 * Returns an instance of an attribute of the specified type. Returns null if the field doesn't have such an
		 * attribute.
		 */
		MonoObject* GetAttribute(MonoClass* monoClass);

		/**	Returns field visibility in the class. */
		MonoMemberVisibility GetVisibility();

		/**	Query if the field is static. */
		bool IsStatic();

	private:
		friend class MonoClass;

		MonoField(MonoClassField* field);

		String mName;
		MonoClassField* mField;
		MonoClass* mFieldType;
	};

	/** @} */
} // namespace b3d
