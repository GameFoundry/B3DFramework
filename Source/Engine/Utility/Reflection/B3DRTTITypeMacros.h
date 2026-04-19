//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
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
 * The type of the member must be a plain, reflectable and reflectable pointer type. Each field must specify
 * a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER(name, id) B3D_RTTI_MEMBER_FULL(name, name, id, b3d::RTTIFieldInfo::DEFAULT)

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
 * plain, reflectable and reflectable pointer types. Each field must specify a unique ID for @p id.
 */
#define B3D_RTTI_MEMBER_CONTAINER(name, id) B3D_RTTI_MEMBER_CONTAINER_FULL(name, name, id, b3d::RTTIFieldInfo::DEFAULT)

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
#define B3D_RTTI_GENERATED_MEMBER(name, id) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, b3d::RTTIFieldInfo::DEFAULT, false)

/** Same as B3D_RTTI_GENERATED_MEMBER, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_GENERATED_MEMBER_INFO(name, id, info) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, info, false)

/**
 * Same as B3D_RTTI_MEMBER_CONTAINER, but the field is looked up on the RTTIType class itself. These fields should be manually
 * populated after RTTI operation starts, and manually applied before it ends.
 */
#define B3D_RTTI_GENERATED_MEMBER_CONTAINER(name, id) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, b3d::RTTIFieldInfo::DEFAULT, true)

/** Same as B3D_RTTI_GENERATED_MEMBER_CONTAINER, but allows you to specify an info structure that further describes the field. */
#define B3D_RTTI_GENERATED_MEMBER_CONTAINER_INFO(name, id, info) B3D_RTTI_GENERATED_MEMBER_IMPL(name, name, id, info, true)

/**
 * Registers an ECS type as an RTTI field. Automatically detects whether the type is a
 * data component (fragment) or a TagGroup and serializes accordingly. For fragments, the
 * component data is serialized directly. For tag groups, tags are packed into a single
 * integer bitfield. Owner must implement IECSEntityOwner. Type is expected to be in ecs:: namespace.
 *
 * @param	name	ECS type name (without ecs:: prefix). Used as both the field name and type identifier.
 * @param	id		Unique field ID for serialization.
 */
#define B3D_RTTI_MEMBER_ECS(name, id)                                                                      \
	META_Entry_##name;                                                                                     \
                                                                                                           \
	struct META_NextEntry_##name                                                                           \
	{};                                                                                                    \
	void META_InitPrevEntry(META_NextEntry_##name typeId)                                                  \
	{                                                                                                      \
		auto field = B3DNew<TRTTIECSField<ecs::name, OwnerType>>(#name, id, b3d::RTTIFieldInfo::DEFAULT);  \
		AddNewField(field);                                                                                \
		META_InitPrevEntry(META_Entry_##name());                                                           \
	}                                                                                                      \
                                                                                                           \
	typedef META_NextEntry_##name

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
} // namespace b3d
