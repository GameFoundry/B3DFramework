//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Allows access to an underlying managed list, or a cached version of that list that can be serialized/deserialized.
	 *
	 * @note
	 * This class can be in two states:
	 *	 - Linked - When the object has a link to a managed object. This is the default state when a new instance
	 *				of ManagedSerializableList is created. Any operations during this state will operate directly
	 *				on the linked managed object. A GC handle will be kept to the linked managed object. The handle can
	 *				be freed by transfering to serialized mode or by destroying this object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 *
	 * You can transfer an object in linked state to serialized state by calling serialize(). If an object is in serialized
	 * state you can call deserialize() to populated a managed object from the cached data.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableList : public IReflectable
	{
	private:
		struct ConstructPrivately
		{};

	public:
		ManagedSerializableList(const ConstructPrivately& dummy, const SPtr<ManagedTypeInfoList>& typeInfo, MonoObject* managedInstance);
		ManagedSerializableList(const ConstructPrivately& dummy);
		~ManagedSerializableList();

		/** Returns the internal managed instance of the list. This will return null if the object is in serialized mode. */
		MonoObject* GetManagedInstance() const;

		/**	Returns the type information for the internal list. */
		SPtr<ManagedTypeInfoList> GetTypeInfo() const { return mListTypeInfo; }

		/** Changes the size of the list. Operates on managed object if in linked state, or on cached data otherwise. */
		void Resize(u32 newSize);

		/**
		 * Sets a new element value at the specified list index. Operates on managed object if in linked state, or on cached
		 * data otherwise.
		 *
		 * @param[in]	arrayIdx	Index at which to set the value.
		 * @param[in]	val			Wrapper around the value to store in the list. Must be of the list element type.
		 */
		void SetFieldData(u32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val);

		/**
		 * Returns the element value at the specified list index. Operates on managed object if in linked state, or on
		 * cached data otherwise.
		 *
		 * @param[in]	arrayIdx	Index at which to retrieve the value.
		 * @return					A wrapper around the element value in the list.
		 */
		SPtr<ManagedSerializableFieldData> GetFieldData(u32 arrayIdx);

		/** Returns the size of the list. Operates on managed object if in linked state, or on cached data otherwise. */
		u32 GetLength() const { return mNumElements; }

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
		 * Creates a managed serializable list that references an existing managed list. Created object will be in linked
		 * mode.
		 *
		 * @param[in]	managedInstance	Constructed managed instance of the list to link with. Its type must correspond with
		 *								the provided type info.
		 * @param[in]	typeInfo		Type information for the list and its elements.
		 */
		static SPtr<ManagedSerializableList> CreateFromExisting(MonoObject* managedInstance, const SPtr<ManagedTypeInfoList>& typeInfo);

		/**
		 * Creates a managed serializable list that creates and references a brand new managed list instance.
		 *
		 * @param[in]	typeInfo	Type of the list to create.
		 * @param[in]	size		Initial size of the list.
		 */
		static SPtr<ManagedSerializableList> CreateNew(const SPtr<ManagedTypeInfoList>& typeInfo, u32 size);

		/**
		 * Creates a managed list instance.
		 *
		 * @param[in]	typeInfo	Type of the list to create.
		 * @param[in]	size		Initial size of the list.
		 */
		static MonoObject* CreateManagedInstance(const SPtr<ManagedTypeInfoList>& typeInfo, u32 size);

	protected:
		/**
		 * Retrieves needed Mono types and methods. Should be called before performing any operations with the managed
		 * object.
		 */
		void InitMonoObjects(MonoClass* listClass);

		/**	Returns the size of the list. Operates on the internal managed object. */
		u32 GetLengthInternal() const;

		/**
		 * Sets a new element value at the specified array index. Operates on the provided managed instance.
		 *
		 * @param[in]	obj			Managed instance in which to set the data in.
		 * @param[in]	arrayIdx	Index at which to set the value.
		 * @param[in]	val			Wrapper around the value to store in the array. Must be of the array element type.
		 */
		void SetFieldData(MonoObject* obj, u32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val);

		/** Appends data to the end of the list. Operates on the internal managed object. */
		void AddFieldDataInternal(const SPtr<ManagedSerializableFieldData>& val);

		uint32_t mGCHandle = 0;

		MonoMethod* mAddMethod = nullptr;
		MonoMethod* mAddRangeMethod = nullptr;
		MonoMethod* mClearMethod = nullptr;
		MonoMethod* mCopyToMethod = nullptr;
		MonoProperty* mItemProp = nullptr;
		MonoProperty* mCountProp = nullptr;

		SPtr<ManagedTypeInfoList> mListTypeInfo;
		Vector<SPtr<ManagedSerializableFieldData>> mCachedEntries;
		u32 mNumElements = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static SPtr<ManagedSerializableList> CreateEmpty();

	public:
		friend class ManagedSerializableListRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
