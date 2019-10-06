//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Handles creation and applying of managed diffs. A diff contains differences between two objects of identical types.
	 * If the initial state of an object is known the recorded differences can be saved and applied to the original state to
	 * restore the modified object.
	 *
	 * Differences are recorded per primitive field in an object. Complex objects are recursed. Special handling is
	 * implemented to properly generate diffs for arrays, lists and dictionaries.
	 *
	 * All primitive types supported by managed serialization are supported (see ScriptPrimitiveType).
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableDiff : public IReflectable
	{
	public:
		/**	A base class for all modifications recorded in a diff. */
		struct Modification : IReflectable
		{
			virtual ~Modification() = default;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModificationRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/**
		 * Contains a modification of a specific field in an object along with information about the field and its parent
		 * object.
		 */
		struct ModifiedField : IReflectable
		{
			ModifiedField() = default;
			ModifiedField(const SPtr<ManagedSerializableTypeInfo>& parentType,
				const SPtr<ManagedSerializableMemberInfo>& fieldType, const SPtr<Modification>& modification);

			SPtr<ManagedSerializableTypeInfo> parentType; /**< Type of the parent object the field belongs to. */
			SPtr<ManagedSerializableMemberInfo> fieldType; /**< Data type of the field. */
			SPtr<Modification> modification; /**< Recorded modification(s) on the field. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedFieldRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/**	Represents a single modified array or list entry. */
		struct ModifiedArrayEntry : IReflectable
		{
			ModifiedArrayEntry() = default;
			ModifiedArrayEntry(UINT32 idx, const SPtr<Modification>& modification);

			UINT32 idx; /**< Index of the array/list entry that is modified. */
			SPtr<Modification> modification; /**< Recorded modification(s) on the entry. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedArrayEntryRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/**	Represents a single modified dictionary entry. */
		struct ModifiedDictionaryEntry : IReflectable
		{
			ModifiedDictionaryEntry() = default;
			ModifiedDictionaryEntry(const SPtr<ManagedSerializableFieldData>& key, const SPtr<Modification>& modification);

			SPtr<ManagedSerializableFieldData> key; /**< Serialized value of the key for the modified entry. */
			SPtr<Modification> modification; /**< Recorded modification(s) on the dictionary entry value. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedArrayEntryRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/**
		 * Contains data about all modifications in a single complex object (aside from arrays, list, dictionaries which are
		 * handled specially).
		 */
		struct ModifiedObject : Modification
		{
			static SPtr<ModifiedObject> create();

			Vector<ModifiedField> entries; /**< A list of entries containing each modified field in the object. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedObjectRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/**	Contains data about all modifications in an array or a list. */
		struct ModifiedArray : Modification
		{
			static SPtr<ModifiedArray> create();

			Vector<ModifiedArrayEntry> entries; /**< A list of all modified array/list entries along with their indices. */
			Vector<UINT32> origSizes; /**< Original size of the array/list (one size per dimension). */
			Vector<UINT32> newSizes; /**< New size of the array/list (one size per dimension). */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedArrayRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/**	Contains data about all modifications in a dictionary. */
		struct ModifiedDictionary : Modification
		{
			static SPtr<ModifiedDictionary> create();

			/** A list of modified entries in the dictionary. */
			Vector<ModifiedDictionaryEntry> entries;
			/** A list of keys for entries that were removed from the dictionary. */
			Vector<SPtr<ManagedSerializableFieldData>> removed;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedDictionaryRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

		/** Contains data about modification of a primitive field (field's new value). */
		struct ModifiedEntry : Modification
		{
			ModifiedEntry() = default;
			ModifiedEntry(const SPtr<ManagedSerializableFieldData>& value);

			static SPtr<ModifiedEntry> create(const SPtr<ManagedSerializableFieldData>& value);

			SPtr<ManagedSerializableFieldData> value;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedEntryRTTI;
			static RTTITypeBase* getRTTIStatic();
			RTTITypeBase* getRTTI() const override;
		};

	public:
		ManagedSerializableDiff();
		~ManagedSerializableDiff() = default;

		/**
		 * Generates a new managed diff object by comparing two objects of the same type. Callers must ensure both objects
		 * are not null and of identical types.
		 *
		 * @param[in]	oldObj	Original object. This is the object you can apply the diff to to convert it to @p newObj.
		 * @param[in]	newObj	New modified object. Any values in this object that differ from the original object will be
		 *						recorded in the diff.
		 * @return				Returns null if objects are identical.
		 */
		static SPtr<ManagedSerializableDiff> create(const ManagedSerializableObject* oldObj, const ManagedSerializableObject* newObj);

		/**
		 * Applies the diff data stored in this object to the specified object, modifying all fields in the object to
		 * correspond to the stored diff data.
		 */
		void apply(const SPtr<ManagedSerializableObject>& obj);

	private:
		/**
		 * Recursively generates a diff between all fields of the specified objects. Returns null if objects are identical.
		 */
		SPtr<ModifiedObject> generateDiff(const ManagedSerializableObject* oldObj, const ManagedSerializableObject* newObj);

		/**
		 * Generates a diff between two fields. Fields can be of any type and the system will generate the diff
		 * appropriately. Diff is generated recursively on all complex objects as well. Returns null if fields contain
		 * identical data.
		 */
		SPtr<Modification> generateDiff(const SPtr<ManagedSerializableFieldData>& oldData, const SPtr<ManagedSerializableFieldData>& newData,
			UINT32 fieldTypeId);

		/**
		 * Applies an object modification to a managed object. Modifications are applied recursively.
		 *
		 * @param[in]	mod Object modification to apply.
		 * @param[in]	obj	Object to apply the modification to.
		 * @return		New field data in the case modification needed the object to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> applyDiff(const SPtr<ModifiedObject>& mod, const SPtr<ManagedSerializableObject>& obj);

		/**
		 * Applies an array modification to a managed array. Modifications are applied recursively.
		 *
		 * @param[in]	mod Array modification to apply.
		 * @param[in]	obj	Array to apply the modification to.
		 * @return		New field data in the case modification needed the array to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> applyDiff(const SPtr<ModifiedArray>& mod, const SPtr<ManagedSerializableArray>& obj);

		/**
		 * Applies an list modification to a managed list. Modifications are applied recursively.
		 *
		 * @param[in]	mod List modification to apply.
		 * @param[in]	obj	List to apply the modification to.
		 * @return		New field data in the case modification needed the list to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> applyDiff(const SPtr<ModifiedArray>& mod, const SPtr<ManagedSerializableList>& obj);

		/**
		 * Applies an dictionary modification to a managed dictionary. Modifications are applied recursively.
		 *
		 * @param[in]	mod Dictionary modification to apply.
		 * @param[in]	obj	Dictionary to apply the modification to.
		 * @return	New field data in the case modification needed the dictionary to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> applyDiff(const SPtr<ModifiedDictionary>& mod, const SPtr<ManagedSerializableDictionary>& obj);

		/**
		 * Applies a modification to a single field. Field type is determined and the modification is applied to the
		 * specific field type as needed. Modifications are applied recursively.
		 *
		 * @param[in]	mod			Modification to apply.
		 * @param[in]	fieldType	Type of the field we're applying the modification to.
		 * @param[in]	origData	Original data of the field.
		 * @return					New field data in the case modification needed the field data to be re-created instead
		 *							of just modified.
		 */
		SPtr<ManagedSerializableFieldData> applyDiff(const SPtr<Modification>& mod, const SPtr<ManagedSerializableTypeInfo>& fieldType,
			const SPtr<ManagedSerializableFieldData>& origData);

		SPtr<ModifiedObject> mModificationRoot;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableDiffRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
