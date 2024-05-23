//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Starts definitions for member fields within a RTTI type. Follow this with calls to B3D_RTTI_MEMBER* calls, and finish by
	 * calling B3D_RTTI_END_MEMBERS.
	 */
#define B3D_RTTI_BEGIN_MEMBERS                       \
	struct META_FirstEntry                          \
	{};                                             \
	void META_InitPrevEntry(META_FirstEntry typeId) \
	{}                                              \
                                                    \
	typedef META_FirstEntry

	/**
	 * Same as B3D_RTTI_MEMBER_PLAIN, but allows you to specify separate names for the field name and the member variable,
	 * as well as an optional info structure further describing the field.
	 */
#define B3D_RTTI_MEMBER_PLAIN_FULL(name, field, id, info)                        \
	META_Entry_##name;                                                          \
                                                                                \
	decltype(OwnerType::field)& get##name(OwnerType* obj)                       \
	{                                                                           \
		return obj->field;                                                      \
	}                                                                           \
	void set##name(OwnerType* obj, decltype(OwnerType::field)& val)             \
	{                                                                           \
		obj->field = val;                                                       \
	}                                                                           \
                                                                                \
	struct META_NextEntry_##name                                                \
	{};                                                                         \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                       \
	{                                                                           \
		AddPlainField(#name, id, &MyType::get##name, &MyType::set##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                \
	}                                                                           \
                                                                                \
	typedef META_NextEntry_##name

	/**
	 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
	 * The type of the member must be a valid plain type. Each field must specify a unique ID for @p id.
	 * An optional @p RTTIFieldInfo structure can be provided to provide further information about the field.
	 */
#define B3D_RTTI_MEMBER_PLAIN(name, id) B3D_RTTI_MEMBER_PLAIN_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

	/** Same as B3D_RTTI_MEMBER_PLAIN, but allows you to specify separate names for the field name and the member variable. */
#define B3D_RTTI_MEMBER_PLAIN_NAMED(name, field, id) B3D_RTTI_MEMBER_PLAIN_FULL(name, field, id, bs::RTTIFieldInfo::DEFAULT)

	/** Same as B3D_RTTI_MEMBER_PLAIN, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_PLAIN_INFO(name, id, info) B3D_RTTI_MEMBER_PLAIN_FULL(name, name, id, info)

	/**
	 * Same as B3D_RTTI_MEMBER_PLAIN_ARRAY, but allows you to specify separate names for the field name and the member
	 * variable, as well as an optional info structure further describing the field.
	 */
#define B3D_RTTI_MEMBER_PLAIN_ARRAY_FULL(name, field, id, info)                                                                       \
	META_Entry_##name;                                                                                                               \
                                                                                                                                     \
	std::common_type<decltype(OwnerType::field)>::type::value_type& get##name(OwnerType* obj, ::bs::u32 idx)                         \
	{                                                                                                                                \
		return obj->field[idx];                                                                                                      \
	}                                                                                                                                \
	void set##name(OwnerType* obj, ::bs::u32 idx, std::common_type<decltype(OwnerType::field)>::type::value_type& val)               \
	{                                                                                                                                \
		obj->field[idx] = val;                                                                                                       \
	}                                                                                                                                \
	::bs::u32 getSize##name(OwnerType* obj)                                                                                          \
	{                                                                                                                                \
		return (::bs::u32)obj->field.size();                                                                                         \
	}                                                                                                                                \
	void setSize##name(OwnerType* obj, ::bs::u32 val)                                                                                \
	{                                                                                                                                \
		obj->field.resize(val);                                                                                                      \
	}                                                                                                                                \
                                                                                                                                     \
	struct META_NextEntry_##name                                                                                                     \
	{};                                                                                                                              \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                                            \
	{                                                                                                                                \
		AddPlainArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                                                                     \
	}                                                                                                                                \
                                                                                                                                     \
	typedef META_NextEntry_##name

	/**
	 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
	 * The type of the member must be an array of valid plain types. Each field must specify a unique ID for @p id.
	 */
#define B3D_RTTI_MEMBER_PLAIN_ARRAY(name, id) B3D_RTTI_MEMBER_PLAIN_ARRAY_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

	/**
	 * Same as B3D_RTTI_MEMBER_PLAIN_ARRAY, but allows you to specify separate names for the field name and the member variable.
	 */
#define B3D_RTTI_MEMBER_PLAIN_ARRAY_NAMED(name, field, id) B3D_RTTI_MEMBER_PLAIN_ARRAY_FULL(name, field, id, bs::RTTIFieldInfo::DEFAULT)

	/**
	 * Same as B3D_RTTI_MEMBER_PLAIN_ARRAY, but allows you to specify an info structure that further describes the field.
	 */
#define B3D_RTTI_MEMBER_PLAIN_ARRAY_INFO(name, id, info) B3D_RTTI_MEMBER_PLAIN_ARRAY_FULL(name, name, id, info)

	/**
	 * Same as B3D_RTTI_MEMBER_PLAIN_MAP, but allows you to specify separate names for the field name and the member
	 * variable, as well as an optional info structure further describing the field.
	 */
#define B3D_RTTI_MEMBER_PLAIN_MAP_FULL(name, field, id, key, info)                                                                   \
	META_Entry_##name;                                                                                                               \
                                                                                                                                     \
	std::common_type<decltype(OwnerType::field)>::type::mapped_type& Get##name(OwnerType* object, ::bs::u32 index)                   \
	{                                                                                                                                \
		auto iterator = object->field.begin();                                                                                       \
		for(u32 i = 0; i < index; i++)                                                                                               \
		{                                                                                                                            \
			++iterator;                                                                                                              \
		}                                                                                                                            \
		return iterator->second;                                                                                                     \
	}                                                                                                                                \
	void Set##name(OwnerType* object, ::bs::u32 index, std::common_type<decltype(OwnerType::field)>::type::mapped_type& value)       \
	{                                                                                                                                \
		object->field[value.key] = value;                                                                                            \
	}                                                                                                                                \
	::bs::u32 GetSize##name(OwnerType* object)                                                                                       \
	{                                                                                                                                \
		return (::bs::u32)object->field.size();                                                                                      \
	}                                                                                                                                \
	void SetSize##name(OwnerType* object, ::bs::u32 size)                                                                            \
	{ /* Do nothing*/                                                                                                                \
	}                                                                                                                                \
                                                                                                                                     \
	struct META_NextEntry_##name                                                                                                     \
	{                                                                                                                                \
	};                                                                                                                               \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                                            \
	{                                                                                                                                \
		AddPlainArrayField(#name, id, &MyType::Get##name, &MyType::GetSize##name, &MyType::Set##name, &MyType::SetSize##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                                                                     \
	}                                                                                                                                \
                                                                                                                                     \
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be a map of valid plain types. The mapped value must also contain the key as one of its fields, provided as @p key.
 * Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER_PLAIN_MAP(name, id, key) B3D_RTTI_MEMBER_PLAIN_MAP_FULL(name, name, id, key, bs::RTTIFieldInfo::DEFAULT)

/**
 * Same as B3D_RTTI_MEMBER_PLAIN_MAP, but allows you to specify separate names for the field name and the member variable.
 */
#define B3D_RTTI_MEMBER_PLAIN_MAP_NAMED(name, field, id, key) B3D_RTTI_MEMBER_PLAIN_MAP_FULL(name, field, id, key, bs::RTTIFieldInfo::DEFAULT)

/**
 * Same as B3D, but allows you to specify an info structure that further describes the field.
 */
#define B3D_RTTI_MEMBER_PLAIN_MAP_INFO(name, id, key, info) B3D_RTTI_MEMBER_PLAIN_MAP_FULL(name, name, id, key, info)


/**
 * Same as B3D_RTTI_MEMBER_REFL, but allows you to specify separate names for the field name and the member variable,
 * as well as an optional info structure further describing the field.
 */
#define B3D_RTTI_MEMBER_REFL_FULL(name, field, id, info)                               \
	META_Entry_##name;                                                                \
                                                                                      \
	decltype(OwnerType::field)& get##name(OwnerType* obj)                             \
	{                                                                                 \
		return obj->field;                                                            \
	}                                                                                 \
	void set##name(OwnerType* obj, decltype(OwnerType::field)& val)                   \
	{                                                                                 \
		obj->field = val;                                                             \
	}                                                                                 \
                                                                                      \
	struct META_NextEntry_##name                                                      \
	{};                                                                               \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                             \
	{                                                                                 \
		AddReflectableField(#name, id, &MyType::get##name, &MyType::set##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                      \
	}                                                                                 \
                                                                                      \
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be a valid reflectable (non-pointer) type. Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER_REFL(name, id) B3D_RTTI_MEMBER_REFL_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_REFL, but allows you to specify separate names for the field name and the member variable. */
#define B3D_RTTI_MEMBER_REFL_NAMED(name, field, id) B3D_RTTI_MEMBER_REFL_FULL(name, field, id, bs::RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_REFL, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_REFL_INFO(name, id, info) B3D_RTTI_MEMBER_REFL_FULL(name, name, id, info)

/**
 * Same as B3D_RTTI_MEMBER_REFL_ARRAY, but allows you to specify separate names for the field name and the member variable,
 * as well as an optional info structure further describing the field.
 */
#define B3D_RTTI_MEMBER_REFL_ARRAY_FULL(name, field, id, info)                                                                              \
	META_Entry_##name;                                                                                                                     \
                                                                                                                                           \
	std::common_type<decltype(OwnerType::field)>::type::value_type& get##name(OwnerType* obj, ::bs::u32 idx)                               \
	{                                                                                                                                      \
		return obj->field[idx];                                                                                                            \
	}                                                                                                                                      \
	void set##name(OwnerType* obj, ::bs::u32 idx, std::common_type<decltype(OwnerType::field)>::type::value_type& val)                     \
	{                                                                                                                                      \
		obj->field[idx] = val;                                                                                                             \
	}                                                                                                                                      \
	::bs::u32 getSize##name(OwnerType* obj)                                                                                                \
	{                                                                                                                                      \
		return (::bs::u32)obj->field.size();                                                                                               \
	}                                                                                                                                      \
	void setSize##name(OwnerType* obj, ::bs::u32 val)                                                                                      \
	{                                                                                                                                      \
		obj->field.resize(val);                                                                                                            \
	}                                                                                                                                      \
                                                                                                                                           \
	struct META_NextEntry_##name                                                                                                           \
	{};                                                                                                                                    \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                                                  \
	{                                                                                                                                      \
		AddReflectableArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                                                                           \
	}                                                                                                                                      \
                                                                                                                                           \
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be an array of valid reflectable (non-pointer) types. Each field must specify a unique ID for
 * @p id.
 */
#define B3D_RTTI_MEMBER_REFL_ARRAY(name, id) B3D_RTTI_MEMBER_REFL_ARRAY_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

/**
 * Same as B3D_RTTI_MEMBER_REFL_ARRAY, but allows you to specify separate names for the field name and the member variable.
 */
#define B3D_RTTI_MEMBER_REFL_ARRAY_NAMED(name, field, id) B3D_RTTI_MEMBER_REFL_ARRAY_FULL(name, field, id, bs::RTTIFieldInfo::DEFAULT)

/**
 * Same as B3D_RTTI_MEMBER_REFL_ARRAY, but allows you to specify an info structure that further describes the field.
 */
#define B3D_RTTI_MEMBER_REFL_ARRAY_INFO(name, id, info) B3D_RTTI_MEMBER_REFL_ARRAY_FULL(name, name, id, info)

/**
 * Same as B3D_RTTI_MEMBER_REFLPTR, but allows you to specify separate names for the field name and the member variable,
 * as well as an optional info structure further describing the field.
 */
#define B3D_RTTI_MEMBER_REFLPTR_FULL(name, field, id, info)                               \
	META_Entry_##name;                                                                   \
                                                                                         \
	decltype(OwnerType::field) get##name(OwnerType* obj)                                 \
	{                                                                                    \
		return obj->field;                                                               \
	}                                                                                    \
	void set##name(OwnerType* obj, decltype(OwnerType::field) val)                       \
	{                                                                                    \
		obj->field = val;                                                                \
	}                                                                                    \
                                                                                         \
	struct META_NextEntry_##name                                                         \
	{};                                                                                  \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                \
	{                                                                                    \
		AddReflectablePtrField(#name, id, &MyType::get##name, &MyType::set##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                         \
	}                                                                                    \
                                                                                         \
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be a valid reflectable pointer type. Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER_REFLPTR(name, id) B3D_RTTI_MEMBER_REFLPTR_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_REFLPTR, but allows you to specify separate names for the field name and the member variable. */
#define B3D_RTTI_MEMBER_REFLPTR_NAMED(name, field, id) B3D_RTTI_MEMBER_REFLPTR_FULL(name, field, id, RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_REFLPTR, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_REFLPTR_INFO(name, id, info) B3D_RTTI_MEMBER_REFLPTR_FULL(name, name, id, info)

	/**
	 * Same as B3D_RTTI_MEMBER_REFLPTR_ARRAY, but allows you to specify separate names for the field name and the member
	 * variable, as well as an optional info structure further describing the field.
	 */
#define B3D_RTTI_MEMBER_REFLPTR_ARRAY_FULL(name, field, id, info)                                                                              \
	META_Entry_##name;                                                                                                                        \
                                                                                                                                              \
	std::common_type<decltype(OwnerType::field)>::type::value_type get##name(OwnerType* obj, ::bs::u32 idx)                                   \
	{                                                                                                                                         \
		return obj->field[idx];                                                                                                               \
	}                                                                                                                                         \
	void set##name(OwnerType* obj, ::bs::u32 idx, std::common_type<decltype(OwnerType::field)>::type::value_type val)                         \
	{                                                                                                                                         \
		obj->field[idx] = val;                                                                                                                \
	}                                                                                                                                         \
	::bs::u32 getSize##name(OwnerType* obj)                                                                                                   \
	{                                                                                                                                         \
		return (::bs::u32)obj->field.size();                                                                                                  \
	}                                                                                                                                         \
	void setSize##name(OwnerType* obj, ::bs::u32 val)                                                                                         \
	{                                                                                                                                         \
		obj->field.resize(val);                                                                                                               \
	}                                                                                                                                         \
                                                                                                                                              \
	struct META_NextEntry_##name                                                                                                              \
	{};                                                                                                                                       \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                                                     \
	{                                                                                                                                         \
		AddReflectablePtrArrayField(#name, id, &MyType::get##name, &MyType::getSize##name, &MyType::set##name, &MyType::setSize##name, info); \
		META_InitPrevEntry(META_Entry_##name());                                                                                              \
	}                                                                                                                                         \
                                                                                                                                              \
	typedef META_NextEntry_##name

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be a valid reflectable pointer type. Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER_REFLPTR_ARRAY(name, id) B3D_RTTI_MEMBER_REFLPTR_ARRAY_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

	/**
	 * Same as B3D_RTTI_MEMBER_REFLPTR_ARRAY, but allows you to specify separate names for the field name and the member
	 * variable.
	 */
#define B3D_RTTI_MEMBER_REFLPTR_ARRAY_NAMED(name, field, id) B3D_RTTI_MEMBER_REFLPTR_ARRAY_FULL(name, field, id, bs::RTTIFieldInfo::DEFAULT)

	/** Same as B3D_RTTI_MEMBER_REFLPTR_ARRAY, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_REFLPTR_ARRAY_INFO(name, id, info) B3D_RTTI_MEMBER_REFLPTR_ARRAY_FULL(name, name, id, info)

/** Common code for implementing both B3D_RTTI_MEMBER_FULL and B3D_RTTI_MEMBER_CONTAINER_FULL. */
#define B3D_RTTI_MEMBER_IMPL(name, field, id, info, container)                                                                                                      \
	META_Entry_##name;                                                                                                                                              \
                                                                                                                                                                    \
	using __TRTTIIterator##name##Type = TRTTIIterator<std::remove_reference_t<decltype(OwnerType::field)>, container>;                                              \
	using __TRTTIIteratorDeleter##name##Type = TRTTIIteratorDeleter<std::remove_reference_t<decltype(OwnerType::field)>, container>;                                \
                                                                                                                                                                    \
	UPtr<__TRTTIIterator##name##Type, DefaultAllocatorTag, __TRTTIIteratorDeleter##name##Type> GetIterator##name(OwnerType& object, FrameAllocator& allocator)      \
	{                                                                                                                                                               \
		return CreateRTTIIterator<std::remove_reference_t<decltype(OwnerType::field)>, container>(allocator, object.field);                                         \
	}                                                                                                                                                               \
	const __TRTTIIterator##name##Type::ElementType& GetValue##name(OwnerType& object, FrameAllocator& allocator, __TRTTIIterator##name##Type& iterator)             \
	{                                                                                                                                                               \
		return *iterator;                                                                                                                                           \
	}                                                                                                                                                               \
	void SetValue##name(OwnerType& object, FrameAllocator& allocator, __TRTTIIterator##name##Type& iterator, const __TRTTIIterator##name##Type::ElementType& value) \
	{                                                                                                                                                               \
		iterator = value;                                                                                                                                           \
	}                                                                                                                                                               \
                                                                                                                                                                    \
	struct META_NextEntry_##name                                                                                                                                    \
	{};                                                                                                                                                             \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                                                                           \
	{                                                                                                                                                               \
		AddField(#name, id, &MyType::GetIterator##name, &MyType::GetValue##name, &MyType::SetValue##name, info);                                                    \
                                                                                                                                                                    \
		META_InitPrevEntry(META_Entry_##name());                                                                                                                    \
	}                                                                                                                                                               \
                                                                                                                                                                    \
	typedef META_NextEntry_##name
/**
 * Same as B3D_RTTI_MEMBER, but allows you to specify separate names for the field name and the member variable,
 * as well as an optional info structure further describing the field.
 */
#define B3D_RTTI_MEMBER_FULL(name, field, id, info) B3D_RTTI_MEMBER_IMPL(name, field, id, info, false)

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be a valid container type (e.g. vector or map). The container is allowed to contain
 * plain, reflectable and reflectable pointer types alike. Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER(name, id) B3D_RTTI_MEMBER_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER, but allows you to specify separate names for the field name and the member variable. */
#define B3D_RTTI_MEMBER_NAMED(name, field, id) B3D_RTTI_MEMBER_FULL(name, field, id, RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_INFO(name, id, info) B3D_RTTI_MEMBER_FULL(name, name, id, info)
/**
 * Same as B3D_RTTI_MEMBER_CONTAINER, but allows you to specify separate names for the field name and the member variable,
 * as well as an optional info structure further describing the field.
 */
#define B3D_RTTI_MEMBER_CONTAINER_FULL(name, field, id, info) B3D_RTTI_MEMBER_IMPL(name, field, id, info, true)

/**
 * Registers a new member field in the RTTI type. The field references the @p name member in the owner class.
 * The type of the member must be a valid container type (e.g. vector or map). The container is allowed to contain
 * plain, reflectable and reflectable pointer types alike. Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER_CONTAINER(name, id) B3D_RTTI_MEMBER_CONTAINER_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_CONTAINER, but allows you to specify separate names for the field name and the member variable. */
#define B3D_RTTI_MEMBER_CONTAINER_NAMED(name, field, id) B3D_RTTI_MEMBER_CONTAINER_FULL(name, field, id, RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_ITERATOR, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_CONTAINER_INFO(name, id, info) B3D_RTTI_MEMBER_CONTAINER_FULL(name, name, id, info)

/** Common code for implementing both B3D_RTTI_GENERATED_MEMBER_FULL and B3D_RTTI_GENERATED_MEMBER_CONTAINER_FULL. */
#define B3D_RTTI_GENERATED_MEMBER_IMPL(name, field, id, info, container)                                                                                            \
	META_Entry_##name;                                                                                                                                              \
                                                                                                                                                                    \
	using __TRTTIIterator##name##Type = TRTTIIterator<std::remove_reference_t<decltype(MyType::field)>, container>;                                                 \
	using __TRTTIIteratorDeleter##name##Type = TRTTIIteratorDeleter<std::remove_reference_t<decltype(MyType::field)>, container>;                                   \
                                                                                                                                                                    \
	UPtr<__TRTTIIterator##name##Type, DefaultAllocatorTag, __TRTTIIteratorDeleter##name##Type> GetIterator##name(OwnerType& object, FrameAllocator& allocator)      \
	{                                                                                                                                                               \
		return CreateRTTIIterator<std::remove_reference_t<decltype(field)>, container>(allocator, field);                                                           \
	}                                                                                                                                                               \
	const __TRTTIIterator##name##Type::ElementType& GetValue##name(OwnerType& object, FrameAllocator& allocator, __TRTTIIterator##name##Type& iterator)             \
	{                                                                                                                                                               \
		return *iterator;                                                                                                                                           \
	}                                                                                                                                                               \
	void SetValue##name(OwnerType& object, FrameAllocator& allocator, __TRTTIIterator##name##Type& iterator, const __TRTTIIterator##name##Type::ElementType& value) \
	{                                                                                                                                                               \
		iterator = value;                                                                                                                                           \
	}                                                                                                                                                               \
                                                                                                                                                                    \
	struct META_NextEntry_##name                                                                                                                                    \
	{};                                                                                                                                                             \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                                                                           \
	{                                                                                                                                                               \
		AddField(#name, id, &MyType::GetIterator##name, &MyType::GetValue##name, &MyType::SetValue##name, info);                                                    \
                                                                                                                                                                    \
		META_InitPrevEntry(META_Entry_##name());                                                                                                                    \
	}                                                                                                                                                               \
                                                                                                                                                                    \
	typedef META_NextEntry_##name

/**
 * Same as B3D_RTTI_MEMBER, but the field is looked up on the RTTIType class itself. These fields should be manually
 * populated after RTTI operation starts, and manually applied before it ends.
 */
#define B3D_RTTI_GENERATED_MEMBER(name, id) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, bs::RTTIFieldInfo::DEFAULT, false)

/** Same as B3D_RTTI_GENERATED_MEMBER, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_GENERATED_MEMBER_INFO(name, id, info) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, info, false)

/**
 * Same as B3D_RTTI_MEMBER_CONTAINER, but the field is looked up on the RTTIType class itself. These fields should be manually
 * populated after RTTI operation starts, and manually applied before it ends.
 */
#define B3D_RTTI_GENERATED_MEMBER_CONTAINER(name, id) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, bs::RTTIFieldInfo::DEFAULT, true)

/** Same as B3D_RTTI_GENERATED_MEMBER_CONTAINER, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_GENERATED_MEMBER_CONTAINER_INFO(name, id, info) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, info, true)

/** Ends definitions for member fields with a RTTI type. Must follow B3D_RTTI_BEGIN_MEMBERS. */
#define B3D_RTTI_END_MEMBERS                                  \
	META_LastEntry;                                          \
                                                             \
	struct META_InitAllMembers                               \
	{                                                        \
		META_InitAllMembers(MyType* owner)                   \
		{                                                    \
			static bool sMembersInitialized = false;         \
			if(!sMembersInitialized)                         \
			{                                                \
				owner->META_InitPrevEntry(META_LastEntry()); \
				sMembersInitialized = true;                  \
			}                                                \
		}                                                    \
	};                                                       \
                                                             \
	META_InitAllMembers mInitMembers{ this };

	/** @} */
} // namespace bs
