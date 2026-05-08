//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Serialization/B3DManagedSerializableField.h"

namespace b3d
{
	/** @addtogroup bsfScript
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
	 *				on the linked managed object. A GC handle will be kept to the linked managed object. The handle can
	 *				be freed by transfering to serialized mode or by destroying this object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 *
	 * You can transfer an object in linked state to serialized state by calling serialize(). If an object is in serialized
	 * state you can call deserialize() to populated a managed object from the cached data.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableObject : public IReflectable
	{
	private:
		struct ConstructPrivately
		{};

		/**	Generates a hash value for field key data identifying a single field in the object. */
		struct B3D_SCRIPT_INTEROP_EXPORT Hash
		{
			size_t operator()(const ManagedSerializableFieldKey& x) const;
		};

		/**	Compares two field key objects. */
		struct B3D_SCRIPT_INTEROP_EXPORT Equals
		{
			bool operator()(const ManagedSerializableFieldKey& a, const ManagedSerializableFieldKey& b) const;
		};

	public:
		ManagedSerializableObject(const ConstructPrivately& dummy, TShared<ManagedObjectInfo> objInfo, MonoObject* managedInstance);
		ManagedSerializableObject(const ConstructPrivately& dummy);
		~ManagedSerializableObject();

		/**
		 * Returns the internal managed instance of the object. This will return null if the object is in serialized mode.
		 */
		MonoObject* GetManagedInstance() const;

		/**	Returns the type information for the internal object. */
		TShared<ManagedObjectInfo> GetObjectInfo() const { return mObjInfo; }

		/**
		 * Sets a new value of the specified field. Operates on managed object if in linked state, or on cached data
		 * otherwise.
		 *
		 * @param[in]	fieldInfo	Object describing the field to which to set the value. This field must belong to the
		 *							type this object is initialized with.
		 * @param[in]	val			Wrapper around the value to store in the field.
		 */
		void SetFieldData(const TShared<ManagedMemberInfo>& fieldInfo, const TShared<ManagedSerializableFieldData>& val);

		/**
		 * Returns the value of the specified field. Operates on managed object if in linked state, or on cached data
		 * otherwise.
		 *
		 * @param[in]	fieldInfo	Object describing the field to which to set the value. This field must belong to the
		 *							type this object is initialized with.
		 * @return					A wrapper around the value of the field.
		 */
		TShared<ManagedSerializableFieldData> GetFieldData(const TShared<ManagedMemberInfo>& fieldInfo) const;

		/**
		 * Serializes the internal managed object into a set of cached data that can be saved in memory/disk and can be
		 * deserialized later. The internal managed object will be freed (if no other references to it). Calling serialize()
		 * again will have no result.
		 */
		void Serialize();

		/**
		 * Deserializes a set of cached data into a managed object. This action may fail in case the cached data contains a
		 * type that no longer exists in which case null is returned.
		 *
		 * @return		Newly created object initialized with the cached data.
		 */
		MonoObject* Deserialize();

		/**
		 * Deserializes a set of cached data into an existing managed object. Caller must ensure the provided object is of
		 * proper type.
		 *
		 * @param[in]	instance	Existing managed instance of the same type this serializable object represents.
		 * @param[in]	objInfo		Serializable object info for the managed object type.
		 */
		void Deserialize(MonoObject* instance, const TShared<ManagedObjectInfo>& objInfo);

		/** Checks if this object has the same contents as the provided object. */
		bool Equals(ManagedSerializableObject& other, RTTIOperationContext* context = nullptr);

		/**
		 * Creates a managed serializable object that references an existing managed object. Created object will be in
		 * linked mode.
		 *
		 * @param[in]	managedInstance		Constructed managed instance of the object to link with.
		 */
		static TShared<ManagedSerializableObject> CreateFromExisting(MonoObject* managedInstance);

		/**
		 * Creates a managed serializable object that creates and references a brand new managed object instance.
		 *
		 * @param[in]	type	Type of the object to create.
		 */
		static TShared<ManagedSerializableObject> CreateNew(const TShared<ManagedTypeInfoObject>& type);

		/**
		 * Creates a managed object instance.
		 *
		 * @param[in]	type	Type of the object to create.
		 */
		static MonoObject* CreateManagedInstance(const TShared<ManagedTypeInfoObject>& type);

	protected:
		uint32_t mGCHandle = 0;
		TShared<ManagedObjectInfo> mObjInfo;
		UnorderedMap<ManagedSerializableFieldKey, TShared<ManagedSerializableFieldData>, Hash, struct Equals> mCachedData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static TShared<ManagedSerializableObject> CreateEmpty();

	public:
		friend class ManagedSerializableObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
