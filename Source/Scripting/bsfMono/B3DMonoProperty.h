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
	 * Encapsulates information about a single Mono (managed) property belonging to some managed class. This object
	 * also allows you to set or retrieve values to/from specific instances containing the property.
	 */
	class B3D_MONO_EXPORT MonoProperty
	{
	public:
		/**	Returns the name of the property. */
		const String& GetName() const { return mName; }

		/**
		 * Returns a boxed value contained in the property in the specified instance.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @return					A boxed value of the property.
		 */
		MonoObject* Get(MonoObject* instance) const;

		/**
		 * Sets a value of the property in the specified instance.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		void Set(MonoObject* instance, void* value) const;

		/**
		 * Returns a boxed value contained in the property in the specified instance. Used for properties with indexers.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	index		Index of the value to retrieve.
		 * @return					A boxed value of the property.
		 */
		MonoObject* GetIndexed(MonoObject* instance, u32 index) const;

		/**
		 * Sets a value of the property in the specified instance. Used for properties with indexers.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	index		Index of the value to set.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		void SetIndexed(MonoObject* instance, u32 index, void* value) const;

		/** Checks does the property contains indexed data, or just a single value. */
		bool IsIndexed() const;

		/**	Returns the data type the property holds. */
		MonoClass* GetReturnType() const;

		/**	Checks if property has an attribute of the specified type. */
		bool HasAttribute(MonoClass* monoClass);

		/**
		 * Returns an instance of an attribute of the specified type. Returns null if the property doesn't have such an
		 * attribute.
		 */
		MonoObject* GetAttribute(MonoClass* monoClass);

		/**
		 * Returns property visibility in the class. If getter/setter methods have different visibility, the more
		 * restrictive one is returned.
		 */
		MonoMemberVisibility GetVisibility();

	private:
		friend class MonoClass;

		MonoProperty(::MonoProperty* monoProp);

		/**
		 * Some property data is not initialized by default on creation (with the assumption it will never be accessed).
		 * This method will initialize that data.
		 */
		void InitializeDeferred() const;

		String mName;
		::MonoProperty* mProperty;
		::MonoMethod* mGetMethod;
		::MonoMethod* mSetMethod;

		mutable MonoClass* mReturnType;
		mutable bool mIsIndexed;
		mutable bool mIsFullyInitialized;
	};

	/** @} */
} // namespace b3d
