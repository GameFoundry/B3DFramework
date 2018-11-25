//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <string>
#include <algorithm>
#include <unordered_map>

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsRTTIPlainField.h"
#include "Reflection/BsRTTIReflectableField.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIManagedDataBlockField.h"
#include "Reflection/BsIReflectable.h"
#include "Serialization/BsBinaryDiff.h"

namespace bs
{
	/** @addtogroup RTTI
	 *  @{
	 */

	 /**
	  * Starts definitions for member fields within a RTTI type. Follow this with calls to BS_RTTI_MEMBER* calls, and finish by
	  * calling BS_END_RTTI_MEMBERS.
	  */
#define BS_BEGIN_RTTI_MEMBERS																	\
	struct META_FirstEntry {};																	\
	void META_InitPrevEntry(META_FirstEntry typeId) { }											\
																								\
	typedef META_FirstEntry

	  /**
	   * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
	   * The type of the member must be a valid plain type. Each field must specify a unique ID for @p id.
	   */
#define BS_RTTI_MEMBER_PLAIN(name, id)															\
	META_Entry_##name;																			\
																								\
	decltype(OwnerType::name)& get##name(OwnerType* obj) { return obj->name; }					\
	void set##name(OwnerType* obj, decltype(OwnerType::name)& val) { obj->name = val; }			\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addPlainField(#name, id, &MyType::get##name, &MyType::set##name);						\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

	   /** Same as BS_RTTI_MEMBER_PLAIN, but allows you to specify separate names for the field name and the member variable. */
#define BS_RTTI_MEMBER_PLAIN_NAMED(name, field, id)												\
	META_Entry_##name;																			\
																								\
	decltype(OwnerType::field)& get##name(OwnerType* obj) { return obj->field; }				\
	void set##name(OwnerType* obj, decltype(OwnerType::field)& val) { obj->field = val; }		\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addPlainField(#name, id, &MyType::get##name, &MyType::set##name);						\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be an array of valid plain types. Each field must specify a unique ID for @p id.
 */
#define BS_RTTI_MEMBER_PLAIN_ARRAY(name, id)													\
	META_Entry_##name;																			\
																								\
	std::common_type<decltype(OwnerType::name)>::type::value_type& get##name(OwnerType* obj, UINT32 idx) { return obj->name[idx]; }					\
	void set##name(OwnerType* obj, UINT32 idx, std::common_type<decltype(OwnerType::name)>::type::value_type& val) { obj->name[idx] = val; }		\
	UINT32 getSize##name(OwnerType* obj) { return (UINT32)obj->name.size(); }																		\
	void setSize##name(OwnerType* obj, UINT32 val) { obj->name.resize(val); }																		\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addPlainArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name);						\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

 /** 
  * Same as BS_RTTI_MEMBER_PLAIN_ARRAY, but allows you to specify separate names for the field name and the member variable.
  */
#define BS_RTTI_MEMBER_PLAIN_ARRAY_NAMED(name, field, id)													\
	META_Entry_##name;																			\
																								\
	std::common_type<decltype(OwnerType::field)>::type::value_type& get##name(OwnerType* obj, UINT32 idx) { return obj->field[idx]; }					\
	void set##name(OwnerType* obj, UINT32 idx, std::common_type<decltype(OwnerType::field)>::type::value_type& val) { obj->field[idx] = val; }		\
	UINT32 getSize##name(OwnerType* obj) { return (UINT32)obj->field.size(); }																		\
	void setSize##name(OwnerType* obj, UINT32 val) { obj->field.resize(val); }																		\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addPlainArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name);						\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class. 
 * The type of the member must be a valid reflectable (non-pointer) type. Each field must specify a unique ID for @p id.
 */
#define BS_RTTI_MEMBER_REFL(name, id)															\
	META_Entry_##name;																			\
																								\
	decltype(OwnerType::name)& get##name(OwnerType* obj) { return obj->name; }					\
	void set##name(OwnerType* obj, decltype(OwnerType::name)& val) { obj->name = val; }			\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectableField(#name, id, &MyType::get##name, &MyType::set##name);					\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/** Same as BS_RTTI_MEMBER_REFL, but allows you to specify separate names for the field name and the member variable. */
#define BS_RTTI_MEMBER_REFL_NAMED(name, field, id)												\
	META_Entry_##name;																			\
																								\
	decltype(OwnerType::field)& get##name(OwnerType* obj) { return obj->field; }				\
	void set##name(OwnerType* obj, decltype(OwnerType::field)& val) { obj->field = val; }		\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectableField(#name, id, &MyType::get##name, &MyType::set##name);					\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class. 
 * The type of the member must be an array of valid reflectable (non-pointer) types. Each field must specify a unique ID for
 * @p id.
 */
#define BS_RTTI_MEMBER_REFL_ARRAY(name, id)														\
	META_Entry_##name;																			\
																								\
	std::common_type<decltype(OwnerType::name)>::type::value_type& get##name(OwnerType* obj, UINT32 idx) { return obj->name[idx]; }					\
	void set##name(OwnerType* obj, UINT32 idx, std::common_type<decltype(OwnerType::name)>::type::value_type& val) { obj->name[idx] = val; }		\
	UINT32 getSize##name(OwnerType* obj) { return (UINT32)obj->name.size(); }					\
	void setSize##name(OwnerType* obj, UINT32 val) { obj->name.resize(val); }					\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectableArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name);				\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name


/** 
 * Same as BS_RTTI_MEMBER_REFL_ARRAY, but allows you to specify separate names for the field name and the member variable. 
 */
#define BS_RTTI_MEMBER_REFL_ARRAY_NAMED(name, field, id)										\
	META_Entry_##name;																			\
																								\
	std::common_type<decltype(OwnerType::field)>::type::value_type& get##name(OwnerType* obj, UINT32 idx) { return obj->field[idx]; }				\
	void set##name(OwnerType* obj, UINT32 idx, std::common_type<decltype(OwnerType::field)>::type::value_type& val) { obj->field[idx] = val; }		\
	UINT32 getSize##name(OwnerType* obj) { return (UINT32)obj->field.size(); }					\
	void setSize##name(OwnerType* obj, UINT32 val) { obj->field.resize(val); }					\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectableArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name);				\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class. 
 * The type of the member must be a valid reflectable pointer type. Each field must specify a unique ID for @p id.
 */
#define BS_RTTI_MEMBER_REFLPTR(name, id)														\
	META_Entry_##name;																			\
																								\
	decltype(OwnerType::name) get##name(OwnerType* obj) { return obj->name; }					\
	void set##name(OwnerType* obj, decltype(OwnerType::name) val) { obj->name = val; }			\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectablePtrField(#name, id, &MyType::get##name, &MyType::set##name);				\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/** Same as BS_RTTI_MEMBER_REFLPTR, but allows you to specify separate names for the field name and the member variable. */
#define BS_RTTI_MEMBER_REFLPTR_NAMED(name, field, id)											\
	META_Entry_##name;																			\
																								\
	decltype(OwnerType::field) get##name(OwnerType* obj) { return obj->field; }					\
	void set##name(OwnerType* obj, decltype(OwnerType::field) val) { obj->field = val; }		\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectablePtrField(#name, id, &MyType::get##name, &MyType::set##name);				\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class. 
 * The type of the member must be a valid reflectable pointer type. Each field must specify a unique ID for @p id.
 */
#define BS_RTTI_MEMBER_REFLPTR_ARRAY(name, id)													\
	META_Entry_##name;																			\
																								\
	std::common_type<decltype(OwnerType::name)>::type::value_type get##name(OwnerType* obj, UINT32 idx) { return obj->name[idx]; }				\
	void set##name(OwnerType* obj, UINT32 idx, std::common_type<decltype(OwnerType::name)>::type::value_type val) { obj->name[idx] = val; }		\
	UINT32 getSize##name(OwnerType* obj) { return (UINT32)obj->name.size(); }					\
	void setSize##name(OwnerType* obj, UINT32 val) { obj->name.resize(val); }					\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectablePtrArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name);			\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

 /**
  * Same as BS_RTTI_MEMBER_REFLPTR_ARRAY, but allows you to specify separate names for the field name and the member 
  * variable. 
  */
#define BS_RTTI_MEMBER_REFLPTR_ARRAY_NAMED(name, field, id)										\
	META_Entry_##name;																			\
																								\
	std::common_type<decltype(OwnerType::field)>::type::value_type get##name(OwnerType* obj, UINT32 idx) { return obj->field[idx]; }				\
	void set##name(OwnerType* obj, UINT32 idx, std::common_type<decltype(OwnerType::field)>::type::value_type val) { obj->field[idx] = val; }		\
	UINT32 getSize##name(OwnerType* obj) { return (UINT32)obj->field.size(); }					\
	void setSize##name(OwnerType* obj, UINT32 val) { obj->field.resize(val); }					\
																								\
	struct META_NextEntry_##name{};																\
	void META_InitPrevEntry(META_NextEntry_##name typeId)										\
	{																							\
		addReflectablePtrArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name);			\
		META_InitPrevEntry(META_Entry_##name());												\
	}																							\
																								\
	typedef META_NextEntry_##name

/** Ends definitions for member fields with a RTTI type. Must follow BS_BEGIN_RTTI_MEMBERS. */
#define BS_END_RTTI_MEMBERS																		\
	META_LastEntry;																				\
																								\
	struct META_InitAllMembers																	\
	{																							\
		META_InitAllMembers(MyType* owner)														\
		{																						\
			static bool sMembersInitialized = false;											\
			if(!sMembersInitialized)															\
			{																					\
				owner->META_InitPrevEntry(META_LastEntry());									\
				sMembersInitialized = true;														\
			}																					\
		}																						\
	};																							\
																								\
	META_InitAllMembers mInitMembers{this};

	/** @} */

	/** @addtogroup Internal-Utility
	 *  @{
	 */
	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	struct SerializationContext;

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
	class BS_UTILITY_EXPORT RTTITypeBase
	{
	public:
		RTTITypeBase() = default;
		virtual ~RTTITypeBase();

		/** Returns RTTI type information for all classes that derive from the class that owns this RTTI type. */
		virtual Vector<RTTITypeBase*>& getDerivedClasses() = 0;

		/**
		 * Returns RTTI type information for the class that owns this RTTI type. If the class has not base type, null is 
		 * returned instead.
		 */
		virtual RTTITypeBase* getBaseClass() = 0;

		/** Returns true if current RTTI class is derived from @p base. (Or if it is the same type as base) */
		virtual bool isDerivedFrom(RTTITypeBase* base) = 0;

		/** Creates a new instance of the class owning this RTTI type. */
		virtual SPtr<IReflectable> newRTTIObject() = 0;

		/** Returns the name of the class owning this RTTI type. */
		virtual const String& getRTTIName() = 0;

		/** Returns an RTTI id that uniquely represents each class in the RTTI system. */
		virtual UINT32 getRTTIId() = 0;

		/**
		 * Called by the serializers when serialization for this object has started. Use this to do any preprocessing on 
		 * data you might need during serialization itself.
		 */
		virtual void onSerializationStarted(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Called by the serializers when serialization for this object has ended. After serialization has ended you can 
		 * be sure that the type has been fully serialized, and you may clean up any temporary data.
		 */
		virtual void onSerializationEnded(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Called by the serializers when deserialization for this object has started. Use this to do any preprocessing 
		 * on data you might need during deserialization itself.
		 */
		virtual void onDeserializationStarted(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Called by the serializers when deserialization for this object has ended. At this point you can be sure the 
		 * instance has been fully deserialized and you may safely use it.
		 *
		 * One exception being are fields you marked with RTTI_Flag_WeakRef, as they might be resolved only after 
		 * deserialization has fully completed for all objects.
		 */
		virtual void onDeserializationEnded(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Returns a handler that determines how are "diffs" generated and applied when it comes to objects of this RTTI 
		 * type. A "diff" is a list of differences between two objects that may be saved, viewed or applied to another 
		 * object to transform it.
		 */
		virtual IDiff& getDiffHandler() const
		{
			static BinaryDiff diffHandler;
			return diffHandler;
		}

		/** Returns the total number of fields in this RTTI type. */
		UINT32 getNumFields() const { return (UINT32)mFields.size(); }

		/** Returns a field based on the field index. Use getNumFields() to get total number of fields available. */
		RTTIField* getField(UINT32 idx) { return mFields.at(idx); }

		/**
		 * Tries to find a field with the specified name. Throws an exception if it can't.
		 *
		 * @param	name	The name of the field.
		 */
		RTTIField* findField(const String& name);

		/**
		 * Tries to find a field with the specified unique ID. Doesn't throw an exception if it can't find the field 
		 * (Unlike findField(const String&)). 
		 *
		 * @param	uniqueFieldId	Unique identifier for the field.
		 *
		 * @return	nullptr if it can't find the field.
		 */
		RTTIField* findField(int uniqueFieldId);

		/** @name Internal 
		 *  @{
		 */

		/** Called by the RTTI system when a class is first found in order to form child/parent class hierarchy. */
		virtual void _registerDerivedClass(RTTITypeBase* derivedClass) = 0;

		/** 
		 * Constucts a cloned version of the underlying class. The cloned version will not have any field information and
		 * should instead be used for passing to various RTTIField methods during serialization/deserialization. This
		 * allows each object instance to have a unique places to store temporary instance-specific data.
		 */
		virtual RTTITypeBase* _clone(FrameAlloc& alloc) = 0;

		/** @} */

	protected:
		/**
		 * Tries to add a new field to the fields array, and throws an exception if a field with the same name or id 
		 * already exists.
		 *
		 * @param[in]	field	Field, must be non-null.
		 */
		void addNewField(RTTIField* field);

	private:
		Vector<RTTIField*> mFields;
	};

	/** Used for initializing a certain type as soon as the program is loaded. */
	template<typename Type, typename BaseType>
	struct InitRTTIOnStart
	{
	public:
		InitRTTIOnStart()
		{
			IReflectable::_registerRTTIType(Type::getRTTIStatic());
			BaseType::getRTTIStatic()->_registerDerivedClass(Type::getRTTIStatic());
		}

		void makeSureIAmInstantiated() { }
	};

	/** Specialization for root class of RTTI hierarchy - IReflectable */
	template<typename Type>
	struct InitRTTIOnStart<Type, IReflectable>
	{
	public:
		InitRTTIOnStart()
		{
			IReflectable::_registerRTTIType(Type::getRTTIStatic());
		}

		void makeSureIAmInstantiated() { }
	};

	/**
	 * Template that returns RTTI type of the specified type, unless the specified type is IReflectable in which case it 
	 * returns a null.
	 */
	template<typename Type>
	struct GetRTTIType
	{
		RTTITypeBase* operator()() { return Type::getRTTIStatic(); }
	};

	/** Specialization for root class of RTTI hierarchy - IReflectable. */
	template<>
	struct GetRTTIType<IReflectable>
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
			initOnStart.makeSureIAmInstantiated();
		}
		virtual ~RTTIType() = default;

		/** Returns a singleton of this RTTI type. */
		static MyRTTIType* instance()
		{
			static MyRTTIType inst;
			return &inst;
		}

		/** @copydoc RTTITypeBase::getDerivedClasses */
		Vector<RTTITypeBase*>& getDerivedClasses() override
		{
			static Vector<RTTITypeBase*> mRTTIDerivedClasses;
			return mRTTIDerivedClasses;
		}

		/** @copydoc RTTITypeBase::getBaseClass */
		RTTITypeBase* getBaseClass() override
		{
			return GetRTTIType<BaseType>()();
		}

		/** @copydoc RTTITypeBase::isDerivedFrom */
		bool isDerivedFrom(RTTITypeBase* base) override
		{
			assert(base != nullptr);

			Stack<RTTITypeBase*> todo;
			todo.push(base);

			while (!todo.empty())
			{
				RTTITypeBase* currentType = todo.top();
				todo.pop();

				if (currentType->getRTTIId() == getRTTIId())
					return true;

				for (const auto& item : currentType->getDerivedClasses())
					todo.push(item);
			}

			return false;
		}

		/** @copydoc RTTITypeBase::_registerDerivedClass */
		void _registerDerivedClass(RTTITypeBase* derivedClass) override
		{
			getDerivedClasses().push_back(derivedClass);
		}

		/** @copydoc RTTITypeBase::_clone */
		RTTITypeBase* _clone(FrameAlloc& alloc) override
		{
			return alloc.construct<MyRTTIType>();
		}

	protected:
		typedef Type OwnerType;
		typedef MyRTTIType MyType;

		/** Registers a field referencing a plain type. */
		template<class InterfaceType, class ObjectType, class DataType>
		void addPlainField(const String& name, UINT32 uniqueId, 
			DataType& (InterfaceType::*getter)(ObjectType*),
			void (InterfaceType::*setter)(ObjectType*, DataType&),
			UINT64 flags = 0)
		{
			static_assert((std::is_base_of<bs::RTTIType<Type, BaseType, MyRTTIType>, InterfaceType>::value), 
				"Class with the get/set methods must derive from bs::RTTIType.");

			static_assert(!(std::is_base_of<bs::IReflectable, DataType>::value), 
				"Data type derives from IReflectable but it is being added as a plain field.");

			auto newField = bs_new<RTTIPlainField<InterfaceType, DataType, ObjectType>>();
			newField->initSingle(name, uniqueId, getter, setter, flags);
			addNewField(newField);
		}

		/** Registers a field referencing an IReflectable type passed by value. */
		template<class InterfaceType, class ObjectType, class DataType>
		void addReflectableField(const String& name, UINT32 uniqueId, 
			DataType& (InterfaceType::*getter)(ObjectType*), 
			void (InterfaceType::*setter)(ObjectType*, DataType&), UINT64 flags = 0)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), 
				"Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = bs_new<RTTIReflectableField<InterfaceType, DataType, ObjectType>>();
			newField->initSingle(name, uniqueId, getter, setter, flags);
			addNewField(newField);
		}

		/** Registers a field referencing an IReflectable type passed by pointer. */
		template<class InterfaceType, class ObjectType, class DataType>
		void addReflectablePtrField(const String& name, UINT32 uniqueId, 
			SPtr<DataType> (InterfaceType::*getter)(ObjectType*), 
			void (InterfaceType::*setter)(ObjectType*, SPtr<DataType>), UINT64 flags = 0)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), 
				"Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = bs_new<RTTIReflectablePtrField<InterfaceType, DataType, ObjectType>>();
			newField->initSingle(name, uniqueId, getter, setter, flags);
			addNewField(newField);
		}

		/** Registers a field referencing an array of plain types. */
		template<class InterfaceType, class ObjectType, class DataType>
		void addPlainArrayField(const String& name, UINT32 uniqueId, 
			DataType& (InterfaceType::*getter)(ObjectType*, UINT32),
			UINT32(InterfaceType::*getSize)(ObjectType*),
			void (InterfaceType::*setter)(ObjectType*, UINT32, DataType&),
			void(InterfaceType::*setSize)(ObjectType*, UINT32),
			UINT64 flags = 0)
		{
			static_assert((std::is_base_of<bs::RTTIType<Type, BaseType, MyRTTIType>, InterfaceType>::value), 
				"Class with the get/set methods must derive from bs::RTTIType.");

			static_assert(!(std::is_base_of<bs::IReflectable, DataType>::value), 
				"Data type derives from IReflectable but it is being added as a plain field.");

			auto newField = bs_new<RTTIPlainField<InterfaceType, DataType, ObjectType>>();
			newField->initArray(name, uniqueId, getter, getSize, setter, setSize, flags);
			addNewField(newField);
		}	

		/** Registers a field referencing an array of IReflectable objects. */
		template<class InterfaceType, class ObjectType, class DataType>
		void addReflectableArrayField(const String& name, UINT32 uniqueId, 
			DataType& (InterfaceType::*getter)(ObjectType*, UINT32), 
			UINT32 (InterfaceType::*getSize)(ObjectType*), 
			void (InterfaceType::*setter)(ObjectType*, UINT32, DataType&), 
			void(InterfaceType::*setSize)(ObjectType*, UINT32), UINT64 flags = 0)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), 
				"Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = bs_new<RTTIReflectableField<InterfaceType, DataType, ObjectType>>();
			newField->initArray(name, uniqueId, getter, getSize, setter, setSize, flags);
			addNewField(newField);
		}

		/** Registers a field referencing an array of IReflectable pointers. */
		template<class InterfaceType, class ObjectType, class DataType>
		void addReflectablePtrArrayField(const String& name, UINT32 uniqueId, 
			SPtr<DataType> (InterfaceType::*getter)(ObjectType*, UINT32), 
			UINT32 (InterfaceType::*getSize)(ObjectType*), 
			void (InterfaceType::*setter)(ObjectType*, UINT32, SPtr<DataType>), 
			void(InterfaceType::*setSize)(ObjectType*, UINT32), UINT64 flags = 0)
		{
			static_assert((std::is_base_of<bs::IReflectable, DataType>::value), 
				"Invalid data type for complex field. It needs to derive from bs::IReflectable.");

			auto newField = bs_new<RTTIReflectablePtrField<InterfaceType, DataType, ObjectType>>();
			newField->initArray(name, uniqueId, getter, getSize, setter, setSize, flags);
			addNewField(newField);
		}

		/** Registers a field referencing a blob of memory. */
		template<class InterfaceType, class ObjectType>
		void addDataBlockField(const String& name, UINT32 uniqueId, SPtr<DataStream> (InterfaceType::*getter)(ObjectType*, UINT32&), 
			void (InterfaceType::*setter)(ObjectType*, const SPtr<DataStream>&, UINT32), UINT64 flags = 0)
		{
			auto newField = bs_new<RTTIManagedDataBlockField<InterfaceType, UINT8*, ObjectType>>();
			newField->initSingle(name, uniqueId, getter, setter, flags);
			addNewField(newField);
		}	
	};

	template <typename Type, typename BaseType, typename MyRTTIType>
	InitRTTIOnStart<Type, BaseType> RTTIType<Type, BaseType, MyRTTIType>::initOnStart;

	/** Extendable class to be used by the user to provide extra information to RTTIType objects during serialization. */
	struct BS_UTILITY_EXPORT SerializationContext : IReflectable
	{
		UINT32 flags = 0;

		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Returns true if the provided object can be safely cast into type T. */
	template<class T>
	bool rtti_is_of_type(IReflectable* object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value), 
			"Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object->getTypeId() == T::getRTTIStatic()->getRTTIId();
	}

	/** Returns true if the provided object can be safely cast into type T. */
	template<class T>
	bool rtti_is_of_type(SPtr<IReflectable> object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value), 
			"Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object->getTypeId() == T::getRTTIStatic()->getRTTIId();
	}

	/** Creates a new object just from its type ID. */
	BS_UTILITY_EXPORT SPtr<IReflectable> rtti_create(UINT32 rttiId);

	/** Checks is the current object a subclass of some type. */
	template<class T>
	bool rtti_is_subclass(IReflectable* object)
	{
		static_assert((std::is_base_of<bs::IReflectable, T>::value),
			"Invalid data type for type checking. It needs to derive from bs::IReflectable.");

		return object->isDerivedFrom(T::getRTTIStatic());
	}

	/** Attempts to cast the object to the provided type, or returns null if cast is not valid. */
	template<class T>
	T* rtti_cast(IReflectable* object)
	{
		if(rtti_is_subclass<T>(object))
			return (T*)object;

		return nullptr;
	}

	/** @} */
}
