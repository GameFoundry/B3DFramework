//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <string>
#include <algorithm>
#include <unordered_map>

#include "BsRTTIIteratorField.h"
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsRTTIPlainField.h"
#include "Reflection/BsRTTIReflectableField.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIManagedDataBlockField.h"
#include "Reflection/BsIReflectable.h"
#include "Reflection/BsRTTITypeMacros.h"
#include "Serialization/BsBinaryDelta.h"

namespace bs
{
	/**
	 * @addtogroup RTTI
	 * @{
	 */

	/** Contains serializable meta-data about a RTTIType. */
	struct RTTISchema : IReflectable
	{
		u32 TypeId = 0;

		SPtr<RTTISchema> BaseTypeSchema;
		Vector<RTTIFieldSchema> FieldSchemas;

		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** List of operations that may be performed on a RTTIType object, to be used with RTTITypeOnOperationStarted/Ended notifies. */
	enum class RTTIOperationType
	{
		Unknown = 0,

		////////////////////////////////
		// Flags that added to type
		////////////////////////////////

		/** Operation performs a read on the RTTI fields. Exclusive with WriteBit. */
		ReadBit = 1 << 0,

		/** Operation performs a write on the RTTI fields. Exclusive with ReadBit. */
		WriteBit = 1 << 1,

		/**
		 * Set when performing writes on an existing object (i.e. object was not created by the RTTI operation itself).
		 * Only relevant for operations that write to fields.
		 */
		PreExistingObjectBit = 1 << 2,

		/** First bit at which operation types start. Not to be used externally. */
		TypeBitStart = 1 << 30,

		////////////////////////////////
		// Operation types
		////////////////////////////////

		/**
		 * Serializing an object to a binary stream or some other format that may be deserialized later. If the object is part of a
		 * class hierarchy, the start notify will first be called on the most-derived child, followed by its parent, and so on.
		 * End notify will be called in the reverse order.
		 */
		Serialization = (TypeBitStart + 0) | ReadBit,
		/**
		 * Deserializing a brand new object from a binary stream or some other format. If the object is part of a class hierarchy,
		 * the start notify will first be called on the base class, followed by its child, and so on. End notify will be called in
		 * the reverse order.
		 */
		Deserialization = (TypeBitStart + 1) | WriteBit,
		DeltaGenerate = (TypeBitStart + 2) | ReadBit, /**< Generating a delta between two objects. */
		DeltaRead = (TypeBitStart + 3) | ReadBit, /**< Decodes objects encoded in a delta to object instances.  */
		/**
		 * Some or all fields on a pre-existing object will be updated with new data. Object initialization should be skipped as the
		 * object is pre-existing, but some kind of refresh/update might be required. If the object is part of a class hierarchy,
		 * start notify will be fired on the entire class hierarchy, starting with base class and followed by each child, after which
		 * field data will be applied, and finally end notify will be called on the entire class hierarchy in reverse order (from
		 * most-derived child to base).
		 */
		DeltaApply = (TypeBitStart + 4) | WriteBit | PreExistingObjectBit,
		/**
		 * Some or all fields on a pre-existing object will be updated with new data. Object initialization should be skipped as the
		 * object is pre-existing, but some kind of refresh/update might be required. If the object is part of a class hierarchy,
		 * start notify will be fired on the exact class whose fields are being updated, followed by the end notify after all fields
		 * have been updated. This process will start with the base class, followed by its child class, and so on.
		 */
		Patch = (TypeBitStart + 5) | WriteBit | PreExistingObjectBit,
		/**
		 * Searching the RTTI fields for references to certain IReflectable object instances or types. If the object is part of
		 * a class hierarchy, the start notify will first be called on the most-derived child, followed by its parent, and so on.
		 * End notify will be called in the reverse order.
		 */
		GatherReferences = (TypeBitStart + 6) | ReadBit,
	};

	using RTTIOperationTypeFlags = Flags<RTTIOperationType>;
	B3D_FLAGS_OPERATORS(RTTIOperationType);

	/** @} */

	/** @addtogroup Internal-Utility
	 *  @{
	 */
	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	struct RTTIOperationContext;

	/**
	 * Provides an interface for accessing fields of a certain class.
	 * Data can be easily accessed by getter and setter methods.
	 *
	 * Supported data types:
	 *	- Plain types - All types defined in BsRTTIField.h, mostly native types and POD (plain old data) structs. Data is parsed byte by byte.
	 *	                No pointers to plain types are supported. Data is passed around by value.
	 *	- Reflectable types - Any class deriving from IReflectable. Data is parsed based on fields in its RTTI class. Can be pointer or value type.
	 *	- Arrays of both plain and reflectable types are supported
	 *	- Data blocks - A managed or unmanaged block of data. See ManagedDataBlock.
	 */
	class B3D_UTILITY_EXPORT RTTITypeBase
	{
	public:
		RTTITypeBase() = default;
		virtual ~RTTITypeBase();

		/** Returns RTTI type information for all classes that derive from the class that owns this RTTI type. */
		virtual Vector<RTTITypeBase*>& GetDerivedClasses() const = 0;

		/**
		 * Returns RTTI type information for the class that owns this RTTI type. If the class has not base type, null is
		 * returned instead.
		 */
		virtual RTTITypeBase* GetBaseClass() const = 0;

		/** Returns true if current RTTI class is derived from @p base. (Or if it is the same type as base) */
		virtual bool IsDerivedFrom(const RTTITypeBase* base) const = 0;

		/** Creates a new instance of the class owning this RTTI type. */
		virtual SPtr<IReflectable> NewRttiObject() = 0;

		/** Returns the name of the class owning this RTTI type. */
		virtual const String& GetRttiName() = 0;

		/** Returns an RTTI id that uniquely represents each class in the RTTI system. */
		virtual u32 GetRttiId() const = 0;

		/** Called before any operation that is iterating over the type's fields starts. */
		virtual void NotifyOperationStarted(IReflectable& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) = 0;

		/** Called after any operation that is iterating over the type's fields ends. */
		virtual void NotifyOperationEnded(IReflectable& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) = 0;

		/**
		 * Called by the serializers when deserialization for this object has ended. At this point you can be sure the
		 * instance has been fully deserialized and you may safely use it.
		 *
		 * One exception being are fields you marked with RTTI_Flag_WeakRef, as they might be resolved only after
		 * deserialization has fully completed for all objects.
		 */
		virtual void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) {}

		/**
		 * Returns a handler that determines how are deltas generated and applied when it comes to objects of this RTTI
		 * type. A delta is a list of differences between two objects that may be saved, viewed or applied to another
		 * object.
		 */
		virtual IDeltaHandler& GetDeltaHandler() const
		{
			static BinaryDeltaHandler deltaHandler;
			return deltaHandler;
		}

		/** Returns the total number of fields in this RTTI type. */
		u32 GetFieldCount() const { return (u32)mFields.size(); }

		/** Returns a field based on the field index. Use getNumFields() to get total number of fields available. */
		RTTIField* GetField(u32 idx) { return mFields.at(idx); }

		/**
		 * Tries to find a field with the specified name. Throws an exception if it can't.
		 *
		 * @param	name	The name of the field.
		 */
		RTTIField* FindField(const String& name);

		/**
		 * Tries to find a field with the specified unique ID. Doesn't throw an exception if it can't find the field
		 * (Unlike findField(const String&)).
		 *
		 * @param	uniqueFieldId	Unique identifier for the field.
		 *
		 * @return	nullptr if it can't find the field.
		 */
		RTTIField* FindField(int uniqueFieldId);

		/** Returns a set of serializable meta-data describing the RTTI type. */
		const SPtr<RTTISchema>& GetSchema() const { return mSchema; }

		/** @name Internal
		 *  @{
		 */

		/** Called by the RTTI system when a class is first found in order to form child/parent class hierarchy. */
		virtual void RegisterDerivedClassInternal(RTTITypeBase* derivedClass) = 0;

		/**
		 * Constucts a cloned version of the underlying class. The cloned version will not have any field information and
		 * should instead be used for passing to various RTTIField methods during serialization/deserialization. This
		 * allows each object instance to have a unique places to store temporary instance-specific data.
		 */
		virtual RTTITypeBase* CloneInternal(FrameAllocator& alloc) = 0;

		/** Initializes the type schema. Should be called once after construction. */
		void InitSchemaInternal();

		/** @} */

	protected:
		/**
		 * Tries to add a new field to the fields array, and throws an exception if a field with the same name or id
		 * already exists.
		 *
		 * @param[in]	field	Field, must be non-null.
		 */
		void AddNewField(RTTIField* field);

		SPtr<RTTISchema> mSchema;

	private:
		Vector<RTTIField*> mFields;
	};

	/** Used for initializing a certain type as soon as the program is loaded. */
	template <typename Type, typename BaseType>
	struct InitRTTIOnStart
	{
	public:
		InitRTTIOnStart()
		{
			RTTITypeBase* rttiType = Type::GetRttiStatic();
			rttiType->InitSchemaInternal();

			IReflectable::RegisterRTTITypeInternal(rttiType);
			BaseType::GetRttiStatic()->RegisterDerivedClassInternal(rttiType);
		}

		void MakeSureIAmInstantiated() {}
	};

	/** Specialization for root class of RTTI hierarchy - IReflectable */
	template <typename Type>
	struct InitRTTIOnStart<Type, IReflectable>
	{
	public:
		InitRTTIOnStart()
		{
			RTTITypeBase* rttiType = Type::GetRttiStatic();
			rttiType->InitSchemaInternal();

			IReflectable::RegisterRTTITypeInternal(rttiType);
		}

		void MakeSureIAmInstantiated() {}
	};

	/**
	 * Template that returns RTTI type of the specified type, unless the specified type is IReflectable in which case it
	 * returns a null.
	 */
	template <typename Type>
	struct GetRttiType
	{
		RTTITypeBase* operator()() { return Type::GetRttiStatic(); }
	};

	/** Specialization for root class of RTTI hierarchy - IReflectable. */
	template <>
	struct GetRttiType<IReflectable>
	{
		RTTITypeBase* operator()() { return nullptr; }
	};

	/** @} */
	/** @} */

	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Allows you to provide a run-time type information for a specific class, along with support for
	 * serialization/deserialization.
	 *
	 * Derive from this class and return the that class from IReflectable::getRTTI. This way you can separate serialization
	 * logic from the actual class you're serializing.
	 *
	 * This class will provide a way to register individual fields in the class, together with ways to read and write them,
	 * as well a providing information about class hierarchy, and run-time type checking.
	 */
	template <typename Type, typename BaseType, typename MyRTTIType>
	class RTTIType : public RTTITypeBase
	{
	protected:
		/************************************************************************/
		/* 						RTTI CLASS META DATA							*/
		/************************************************************************/

		static InitRTTIOnStart<Type, BaseType> initOnStart;

	public:
		RTTIType()
		{
			// Compiler will only generate code for stuff that is directly used, including static data members,
			// so we fool it here like we're using the class directly. Otherwise compiler won't generate the code for the member
			// and our type won't get initialized on start (Actual behavior is a bit more random)
			initOnStart.MakeSureIAmInstantiated();
		}

		virtual ~RTTIType() = default;

		/** Returns a singleton of this RTTI type. */
		static MyRTTIType* Instance()
		{
			static MyRTTIType inst;
			return &inst;
		}

		Vector<RTTITypeBase*>& GetDerivedClasses() const override
		{
			static Vector<RTTITypeBase*> mRTTIDerivedClasses;
			return mRTTIDerivedClasses;
		}

		RTTITypeBase* GetBaseClass() const override
		{
			return GetRttiType<BaseType>()();
		}

		bool IsDerivedFrom(const RTTITypeBase* base) const override
		{
			B3D_ASSERT(base != nullptr);

			TInlineArray<const RTTITypeBase*, 6> todo;
			todo.Add(base);

			while(!todo.Empty())
			{
				const RTTITypeBase* currentType = todo.Back();
				todo.Pop();

				if(currentType->GetRttiId() == GetRttiId())
					return true;

				for(const auto& item : currentType->GetDerivedClasses())
					todo.Add(item);
			}

			return false;
		}

		void RegisterDerivedClassInternal(RTTITypeBase* derivedClass) override
		{
			GetDerivedClasses().push_back(derivedClass);
		}

		RTTITypeBase* CloneInternal(FrameAllocator& alloc) override
		{
			return alloc.Construct<MyRTTIType>();
		}

	protected:
		typedef Type OwnerType;
		typedef MyRTTIType MyType;

		template<typename DataType, bool IsContainer>
		using UPtrRTTIIterator = UPtr<TRTTIIterator<DataType, IsContainer>, DefaultAllocatorTag, TRTTIIteratorDeleter<DataType, IsContainer>>;

		/** Registers a field referencing a plain type. */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddPlainField(const String& name, u32 uniqueId, DataType& (InterfaceType::*getter)(ObjectType*), void (InterfaceType::*setter)(ObjectType*, DataType&), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			static_assert((std::is_base_of<bs::RTTIType<Type, BaseType, MyRTTIType>, InterfaceType>::value), "Class with the get/set methods must derive from bs::RTTIType.");
			static_assert(!(std::is_base_of<bs::IReflectable, DataType>::value), "Data type derives from IReflectable but it is being added as a plain field.");
			static_assert(B3DHasRTTIPlainTypeSpecialization<DataType>::value, "Data type must provide a RTTIPlainType specialization.");

			auto newField = B3DNew<RTTIPlainField<InterfaceType, DataType, ObjectType>>();
			newField->InitSingle(name, uniqueId, getter, setter, info);
			AddNewField(newField);
		}

		/** Registers a field referencing an IReflectable type passed by value. */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddReflectableField(const String& name, u32 uniqueId, DataType& (InterfaceType::*getter)(ObjectType*), void (InterfaceType::*setter)(ObjectType*, DataType&), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), "Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = B3DNew<RTTIReflectableField<InterfaceType, DataType, ObjectType>>();
			newField->InitSingle(name, uniqueId, getter, setter, info);
			AddNewField(newField);
		}

		/** Registers a field referencing an IReflectable type passed by pointer. */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddReflectablePtrField(const String& name, u32 uniqueId, SPtr<DataType> (InterfaceType::*getter)(ObjectType*), void (InterfaceType::*setter)(ObjectType*, SPtr<DataType>), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), "Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = B3DNew<RTTIReflectablePtrField<InterfaceType, DataType, ObjectType>>();
			newField->InitSingle(name, uniqueId, getter, setter, info);
			AddNewField(newField);
		}

		/** Registers a field referencing an array of plain types. */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddPlainArrayField(const String& name, u32 uniqueId, DataType& (InterfaceType::*getter)(ObjectType*, u32), u32 (InterfaceType::*getSize)(ObjectType*), void (InterfaceType::*setter)(ObjectType*, u32, DataType&), void (InterfaceType::*setSize)(ObjectType*, u32), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			static_assert((std::is_base_of<bs::RTTIType<Type, BaseType, MyRTTIType>, InterfaceType>::value), "Class with the get/set methods must derive from bs::RTTIType.");
			static_assert(!(std::is_base_of<bs::IReflectable, DataType>::value), "Data type derives from IReflectable but it is being added as a plain field.");
			static_assert(B3DHasRTTIPlainTypeSpecialization<DataType>::value, "Data type must provide a RTTIPlainType specialization.");

			auto newField = B3DNew<RTTIPlainField<InterfaceType, DataType, ObjectType>>();
			newField->InitArray(name, uniqueId, getter, getSize, setter, setSize, info);
			AddNewField(newField);
		}

		/** Registers a field referencing an array of IReflectable objects. */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddReflectableArrayField(const String& name, u32 uniqueId, DataType& (InterfaceType::*getter)(ObjectType*, u32), u32 (InterfaceType::*getSize)(ObjectType*), void (InterfaceType::*setter)(ObjectType*, u32, DataType&), void (InterfaceType::*setSize)(ObjectType*, u32), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), "Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = B3DNew<RTTIReflectableField<InterfaceType, DataType, ObjectType>>();
			newField->InitArray(name, uniqueId, getter, getSize, setter, setSize, info);
			AddNewField(newField);
		}

		/** Registers a field referencing an array of IReflectable pointers. */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddReflectablePtrArrayField(const String& name, u32 uniqueId, SPtr<DataType> (InterfaceType::*getter)(ObjectType*, u32), u32 (InterfaceType::*getSize)(ObjectType*), void (InterfaceType::*setter)(ObjectType*, u32, SPtr<DataType>), void (InterfaceType::*setSize)(ObjectType*, u32), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), "Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = B3DNew<RTTIReflectablePtrField<InterfaceType, DataType, ObjectType>>();
			newField->InitArray(name, uniqueId, getter, getSize, setter, setSize, info);
			AddNewField(newField);
		}

		/** Registers a field referencing a blob of memory. */
		template <class InterfaceType, class ObjectType>
		void AddDataBlockField(const String& name, u32 uniqueId, SPtr<DataStream> (InterfaceType::*getter)(ObjectType*, u32&), void (InterfaceType::*setter)(ObjectType*, const SPtr<DataStream>&, u32), const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			auto newField = B3DNew<RTTIManagedDataBlockField<InterfaceType, u8*, ObjectType>>();
			newField->InitSingle(name, uniqueId, getter, setter, info);
			AddNewField(newField);
		}

		/**
		 * Registers a field containing a type derived from IReflectable, shared pointer to IReflectable, or a type implementing
		 * RTTIPlainType<T> specialization. Containers (vectors, map) containing plain types are also supported, but if the container
		 * contains IReflectable types, then use AddContainerField.
		 */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddField(const String& name, u32 uniqueId,
			UPtr<TRTTIIterator<DataType, false>, DefaultAllocatorTag, TRTTIIteratorDeleter<DataType, false>> (InterfaceType::*getIteratorCallback)(ObjectType&, FrameAllocator&),
			const DataType& (InterfaceType::*getValueCallback)(ObjectType&, FrameAllocator&, TRTTIIterator<DataType, false>&),
			void (InterfaceType::*setValueCallback)(ObjectType&, FrameAllocator&, TRTTIIterator<DataType, false>&, const DataType&),
			const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			auto field = B3DNew<TRTTIIteratorField<InterfaceType, DataType, false, ObjectType>>(name, uniqueId, getIteratorCallback, getValueCallback, setValueCallback, info);
			AddNewField(field);
		}

		/**
		 * Registers a field referencing an iterable container (such as an array or a map). Unlike AddField(), this overload will allow
		 * enumeration of child IReflectable entries within the container.
		 */
		template <class InterfaceType, class ObjectType, class DataType>
		void AddField(const String& name, u32 uniqueId,
			UPtr<TRTTIIterator<DataType, true>, DefaultAllocatorTag, TRTTIIteratorDeleter<DataType, true>> (InterfaceType::*getIteratorCallback)(ObjectType&, FrameAllocator&),
			const typename DataType::value_type& (InterfaceType::*getValueCallback)(ObjectType&, FrameAllocator&, TRTTIIterator<DataType, true>&),
			void (InterfaceType::*setValueCallback)(ObjectType&, FrameAllocator&, TRTTIIterator<DataType, true>&, const typename DataType::value_type&),
			const RTTIFieldInfo& info = RTTIFieldInfo::DEFAULT)
		{
			auto field = B3DNew<TRTTIIteratorField<InterfaceType, DataType, true, ObjectType>>(name, uniqueId, getIteratorCallback, getValueCallback, setValueCallback, info);
			AddNewField(field);
		}

		/**
		 * Helper function to create a RTTI iterator that may be used for reading or writing to the provided data type. Iterator will be allocated
		 * using the provided frame allocator. Caller must ensure that allocator lives longer than the iterator. If @p IsContainer if false, you
		 * may provide a non-container type in @p value, in which case the iterator will act as faux single-value iterator.
		 */
		template<class DataType, bool IsContainer>
		static UPtrRTTIIterator<DataType, IsContainer> CreateRTTIIterator(FrameAllocator& allocator, DataType& value)
		{
			return UPtrRTTIIterator<DataType, IsContainer>(allocator.Construct<TRTTIIterator<DataType, IsContainer>>(value), TRTTIIteratorDeleter<DataType, IsContainer>(&allocator));
		}

		/** @copydoc NotifyOperationStarted(IReflectable&, RTTIOperationTypeFlags, RTTIOperationContext&) */
		virtual void OnOperationStarted(Type& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) {}

		/** @copydoc NotifyOperationEnded(IReflectable&, RTTIOperationTypeFlags, RTTIOperationContext&) */
		virtual void OnOperationEnded(Type& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) {}

		void NotifyOperationStarted(IReflectable& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) final
		{
			OnOperationStarted(static_cast<Type&>(object), operationType, context);
		}

		void NotifyOperationEnded(IReflectable& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) final
		{
			OnOperationEnded(static_cast<Type&>(object), operationType, context);
		}
	};

	template <typename Type, typename BaseType, typename MyRTTIType>
	InitRTTIOnStart<Type, BaseType> RTTIType<Type, BaseType, MyRTTIType>::initOnStart;

	/** Provides information about the operation being performed on RTTIType objects. */
	struct B3D_UTILITY_EXPORT RTTIOperationContext : IReflectable
	{
		u32 Flags = 0;

		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Returns true if the provided object can be safely cast into type T. */
	template <class T>
	bool B3DRTTIIsOfType(IReflectable* object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value), "Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object->GetTypeId() == T::GetRttiStatic()->GetRttiId();
	}

	/** Returns true if the provided object can be safely cast into type T. */
	template <class T>
	bool B3DRTTIIsOfType(SPtr<IReflectable> object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value), "Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object->GetTypeId() == T::GetRttiStatic()->GetRttiId();
	}

	/** Creates a new object just from its type ID. */
	B3D_UTILITY_EXPORT SPtr<IReflectable> B3DRTTICreate(u32 rttiId);

	/** Checks is the current object a subclass of some type. */
	template <class T>
	bool B3DRTTIIsSubclass(const IReflectable* object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value), "Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object && object->IsDerivedFrom(T::GetRttiStatic());
	}

	/** Checks is the current object a subclass of some type. */
	template <class T>
	bool B3DRTTIIsSubclass(const SPtr<IReflectable>& object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value), "Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object && object->IsDerivedFrom(T::GetRttiStatic());
	}

	/** Attempts to cast the object to the provided type, or returns null if cast is not valid. */
	template <class T>
	T* B3DRTTICast(const IReflectable* object)
	{
		if(B3DRTTIIsSubclass<T>(object))
			return (T*)object;

		return nullptr;
	}

	/** Attempts to cast the object to the provided type, or returns null if cast is not valid. */
	template <class T>
	SPtr<T> B3DRTTICast(const SPtr<IReflectable> object)
	{
		if(B3DRTTIIsSubclass<T>(object))
			return std::static_pointer_cast<T>(object);

		return nullptr;
	}

	/** @} */
} // namespace bs
