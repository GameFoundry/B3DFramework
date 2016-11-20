//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsIReflectable.h"
#include "BsManagedSerializableField.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Allows access to an underlying managed object, or a cached version of that object that can be
	 * serialized/deserialized.
	 *
	 * @note
	 * This class can be in two states:
	 *	 - Linked - When the object has a link to a managed object. This is the default state when a new instance
	 *				of ManagedSerializableObject is created. Any operations during this state will operate directly
	 *				on the linked managed object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 * You can transfer between these states by calling serialize(linked->serialized) & deserialize (serialized->linked).
	 *	
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableObject : public IReflectable
	{
	private:
		struct ConstructPrivately {};

		/**	Generates a hash value for field key data identifying a single field in the object. */
		struct BS_SCR_BE_EXPORT Hash
		{
			inline size_t operator()(const ManagedSerializableFieldKey& x) const;
		};

		/**	Compares two field key objects. */
		struct BS_SCR_BE_EXPORT Equals
		{
			inline bool operator()(const ManagedSerializableFieldKey& a, const ManagedSerializableFieldKey& b) const;
		};

	public:
		ManagedSerializableObject(const ConstructPrivately& dummy, SPtr<ManagedSerializableObjectInfo> objInfo, MonoObject* managedInstance);
		ManagedSerializableObject(const ConstructPrivately& dummy);

		/**
		 * Returns the internal managed instance of the object. This will return null if the object is in serialized mode.
		 */
		MonoObject* getManagedInstance() const { return mManagedInstance; }

		/**	Returns the type information for the internal object. */
		SPtr<ManagedSerializableObjectInfo> getObjectInfo() const { return mObjInfo; }

		/**
		 * Sets a new value of the specified field. Operates on managed object if in linked state, or on cached data
		 * otherwise.
		 *
		 * @param[in]	fieldInfo	Object describing the field to which to set the value. This field must belong to the
		 *							type this object is initialized with.
		 * @param[in]	val			Wrapper around the value to store in the field.
		 */
		void setFieldData(const SPtr<ManagedSerializableMemberInfo>& fieldInfo, const SPtr<ManagedSerializableFieldData>& val);

		/**
		 * Returns the value of the specified field. Operates on managed object if in linked state, or on cached data
		 * otherwise.
		 *
		 * @param[in]	fieldInfo	Object describing the field to which to set the value. This field must belong to the
		 *							type this object is initialized with.
		 * @return					A wrapper around the value of the field.
		 */
		SPtr<ManagedSerializableFieldData> getFieldData(const SPtr<ManagedSerializableMemberInfo>& fieldInfo) const;

		/**
		 * Serializes the internal managed object into a set of cached data that can be saved in memory/disk and can be
		 * deserialized later. Does nothing if object is already is serialized mode. When in serialized mode the reference
		 * to the managed instance will be lost.
		 */
		void serialize();

		/**
		 * Deserializes a set of cached data into a managed object. This action may fail in case the cached data contains a
		 * type that no longer exists. You may check if it completely successfully if getManagedInstance() returns non-null
		 * after.
		 *
		 * This action transfers the object into linked mode. All further operations will operate directly on the managed
		 * instance and the cached data will be cleared. If you call this method on an already linked object the old object
		 * will be replaced and initialized with empty data (since cached data does not exist).
		 */
		void deserialize();

		/**
		 * Deserializes a set of cached data into an existing managed object. Caller must ensure the provided object is of
		 * proper type.
		 *
		 * This action transfers the object into linked mode. All further operations will operate directly on the managed
		 * instance and the cached data will be cleared. If you call this method on an already linked object the old object
		 * will be replaced and initialized with empty data (since cached data does not exist).
		 *
		 * @param[in]	instance	Existing managed instance of the same type this serializable object represents.
		 * @param[in]	objInfo		Serializable object info for the managed object type.
		 */
		void deserialize(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo);

		/**
		 * Creates a managed serializable object that references an existing managed object. Created object will be in
		 * linked mode.
		 *
		 * @param[in]	managedInstance		Constructed managed instance of the object to link with.
		 */
		static SPtr<ManagedSerializableObject> createFromExisting(MonoObject* managedInstance);

		/**
		 * Creates a managed serializable object that creates and references a brand new managed object instance.
		 *
		 * @param[in]	type	Type of the object to create.
		 */
		static SPtr<ManagedSerializableObject> createNew(const SPtr<ManagedSerializableTypeInfoObject>& type);

		/**
		 * Creates a managed object instance.
		 *
		 * @param[in]	type	Type of the object to create.
		 */
		static MonoObject* createManagedInstance(const SPtr<ManagedSerializableTypeInfoObject>& type);
	protected:
		MonoObject* mManagedInstance;

		SPtr<ManagedSerializableObjectInfo> mObjInfo;
		UnorderedMap<ManagedSerializableFieldKey, SPtr<ManagedSerializableFieldData>, Hash, Equals> mCachedData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static SPtr<ManagedSerializableObject> createEmpty();

	public:
		friend class ManagedSerializableObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}