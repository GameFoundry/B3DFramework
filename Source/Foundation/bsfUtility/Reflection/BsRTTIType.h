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
#include "Serialization/BsBinaryDiff.h"
#include "Serialization/BsBinaryCompare.h"

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
	using __TRTTIIterator##name##Type = TRTTIIterator<decltype(OwnerType::field), container>;                                                                       \
	using __TRTTIIteratorDeleter##name##Type = TRTTIIteratorDeleter<decltype(OwnerType::field), container>;                                                         \
                                                                                                                                                                    \
	UPtr<__TRTTIIterator##name##Type, DefaultAllocatorTag, __TRTTIIteratorDeleter##name##Type> GetIterator##name(OwnerType& object, FrameAllocator& allocator)      \
	{                                                                                                                                                               \
		return CreateRTTIIterator<decltype(OwnerType::field), container>(allocator, object.field);                                                                  \
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
#define B3D_RTTI_MEMBER_CONTAINER(name, id) B3D_RTTI_MEMBER_CONTAINER_FULL(name, name, id, bs::RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_CONTAINER, but allows you to specify separate names for the field name and the member variable. */
#define B3D_RTTI_MEMBER_CONTAINER_NAMED(name, field, id) B3D_RTTI_MEMBER_CONTAINER_FULL(name, field, id, RTTIFieldInfo::DEFAULT)

/** Same as B3D_RTTI_MEMBER_ITERATOR, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_MEMBER_CONTAINER_INFO(name, id, info) B3D_RTTI_MEMBER_CONTAINER_FULL(name, name, id, info)
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

	/** Contains serializable meta-data about a RTTIType. */
	struct RTTISchema : IReflectable
	{
		u32 TypeId = 0;

		SPtr<RTTISchema> BaseTypeSchema;
		Vector<RTTIFieldSchema> FieldSchemas;

		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

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

		/**
		 * Called by the serializers when serialization for this object has started. Use this to do any preprocessing on
		 * data you might need during serialization itself.
		 */
		virtual void OnSerializationStarted(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Called by the serializers when serialization for this object has ended. After serialization has ended you can
		 * be sure that the type has been fully serialized, and you may clean up any temporary data.
		 */
		virtual void OnSerializationEnded(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Called by the serializers when deserialization for this object has started. Use this to do any preprocessing
		 * on data you might need during deserialization itself.
		 */
		virtual void OnDeserializationStarted(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Called by the serializers when deserialization for this object has ended. At this point you can be sure the
		 * instance has been fully deserialized and you may safely use it.
		 *
		 * One exception being are fields you marked with RTTI_Flag_WeakRef, as they might be resolved only after
		 * deserialization has fully completed for all objects.
		 */
		virtual void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) {}

		/**
		 * Returns a handler that determines how are "diffs" generated and applied when it comes to objects of this RTTI
		 * type. A "diff" is a list of differences between two objects that may be saved, viewed or applied to another
		 * object to transform it.
		 */
		virtual IDiff& GetDiffHandler() const
		{
			static BinaryDiff diffHandler;
			return diffHandler;
		}

		/** Returns a handler that determines how are IReflectable objects compared for equality. */
		virtual ICompare& GetCompareHandler() const
		{
			static BinaryCompare compareHandler;
			return compareHandler;
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
	};

	template <typename Type, typename BaseType, typename MyRTTIType>
	InitRTTIOnStart<Type, BaseType> RTTIType<Type, BaseType, MyRTTIType>::initOnStart;

	/** Extendable class to be used by the user to provide extra information to RTTIType objects during serialization. */
	struct B3D_UTILITY_EXPORT SerializationContext : IReflectable
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
