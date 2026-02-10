//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DUtility.h"
#include "ECS/B3DComponentStorage.h"
#include "ECS/B3DTagStorage.h"
#include "ECS/B3DEntityStorage.h"
#include "ECS/B3DView.h"
#include "ECS/B3DGroup.h"

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/**
	 * Stores all entities and components in their respective storages. Allows creation & destruction of entities, as well as a way to
	 * add or remove components to/from those entities. Allows creation of views or groups that allow iteration over entities containing
	 * a certain subset of components. Provides other helper functionality such as sorting.
	 */
	class Registry
	{
	public:
		/** Tries to retrieve the storage that contains a component identified with @p typeId. Returns null if the storage has not been created. */
		const SparseSet* TryGetStorage(TypeId typeId) const
		{
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return found->second.get();

			return nullptr;
		}

		/** Tries to retrieve the storage that contains a component identified with @p typeId. Returns null if the storage has not been created. */
		SparseSet* TryGetStorage(TypeId typeId)
		{
			return const_cast<SparseSet*>(std::as_const(*this).TryGetStorage(typeId));
		}

		/** Tries to retrieve the storage that contains a component @p Type. Returns null if the storage has not been created. */
		template<typename Type>
		const TStorageType<Type>* TryGetStorage() const
		{
			if constexpr(std::is_same_v<Type, Entity>)
				return static_cast<const TStorageType<Type>*>(&mEntityStorage);

			const TypeId typeId = B3DGetRuntimeTypeId<Type>();
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return static_cast<const TStorageType<Type>*>(found->second.get());

			return nullptr;
		}

		/** Tries to retrieve the storage that contains a component of type @p Type. Returns null if the storage has not been created. */
		template<typename Type>
		TStorageType<Type>* TryGetStorage()
		{
			return const_cast<TStorageType<Type>*>(std::as_const(*this).TryGetStorage<Type>());
		}

		/** Retrieves storage that contains a component of type @p Type. If the storage doesn't already exist one is created. */
		template<typename Type>
		TStorageType<Type>& GetOrCreateStorage()
		{
			if constexpr(std::is_same_v<Type, Entity>)
				return static_cast<TStorageType<Type>&>(mEntityStorage);

			const TypeId typeId = B3DGetRuntimeTypeId<Type>();
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return static_cast<TStorageType<Type>&>(*found->second);

			SPtr<SparseSet> componentStorage;
			if constexpr(std::is_empty_v<Type>)
				componentStorage = B3DMakeShared<TTagSparseSet<Type>>();
			else
			{
				static constexpr bool isTypeMovable = std::is_move_constructible_v<Type> && std::is_move_assignable_v<Type>;
				componentStorage = B3DMakeShared<TComponentSparseSet<Type, !isTypeMovable>>();
			}

			mComponentStorage[typeId] = componentStorage;
			return static_cast<TStorageType<Type>&>(*componentStorage);
		}

		/** Destroys storage and all associated components of type @p typeId. */
		bool RemoveStorage(TypeId typeId)
		{
			return mComponentStorage.erase(typeId) > 0;
		}

		/** Destroys storage and all associated components of type @p Type. */
		template<typename Type>
		bool RemoveStorage()
		{
			const TypeId typeId = B3DGetRuntimeTypeId<Type>();
			return RemoveStorage(typeId);
		}

		/** Clears all components of type @p Type from their storage. */
		template<typename... Type>
		void ClearStorage()
		{
			if(sizeof...(Type) == 0u)
				return;

			(GetOrCreateStorage<Type>().Clear(), ...);
		}

		/** Clears all components and entities from the registry. */
		void Clear()
		{
			for(auto& entry : mComponentStorage)
				entry.second->Clear();
			
			mEntityStorage.Clear();
		}

		/** Returns true if the entity has been created and is still valid. */
		bool IsEntityValid(Entity entity) const
		{
			if(auto found = mEntityStorage.Find(entity); found != mEntityStorage.End())
				return found.Index() < mEntityStorage.GetFirstFreeElementPackedIndex();

			return false;
		}

		/** Returns true if entity has at least one associated component. */
		bool HasEntityAnyComponents(Entity entity) const
		{
			for(auto& entry : mComponentStorage)
			{
				if(entry.second->Contains(entity))
					return true;
			}

			return false;
		}

		/** Returns the current version of the entity. If an entity is destroyed and its identifier re-used, the version will be incremented. */
		Entity::VersionType GetEntityVersion(Entity entity) const
		{
			return mEntityStorage.GetVersion(entity);
		}

		/** Creates a brand new entity with a unique identifier. */
		Entity CreateEntity()
		{
			return mEntityStorage.Create();
		}

		/** Creates a new entity while attempting to re-use an invalid entity identifier as provided by @p hint. */
		Entity CreateEntity(Entity hint)
		{
			return mEntityStorage.Create(hint);
		}

		/** Removes an entity and all associated components from the set if it exists, otherwise does nothing. Returns the version of the entity that was removed. */
		Entity::VersionType EraseEntity(Entity entity)
		{
			for(auto& entry : mComponentStorage)
				entry.second->EraseIfValid(entity);

			mEntityStorage.EraseIfValid(entity);
			return mEntityStorage.GetVersion(entity);
		}

		/** Removes a range of entities and all their associated components. */
		template<typename It>
		void EraseEntities(It first, It last)
		{
			// Note: Deleting from the end would be more efficient. Perhaps in the future.
			const auto from = mEntityStorage.Begin();
			const auto to = mEntityStorage.SortAs(first, last);

			for(auto& entry : mComponentStorage)
				entry.second->EraseIfValid(from, to);

			mEntityStorage.Erase(from, to);
		}

		/** Constructs a component with the provided arguments and associates it with the provided entity. */
		template<typename Type, typename... Arguments>
		Type& AddComponent(Entity entity, Arguments&&... arguments)
		{
			return GetOrCreateStorage<Type>().Add(entity, std::forward<Arguments>(arguments)...);
		}

		/** Associates a component with a range of entities. */
		template<typename Type, typename It>
		void AddComponents(It first, It last, const Type& component = {})
		{
			B3D_ASSERT(std::all_of(first, last, [this](Entity entity) { return IsEntityValid(entity); }));
			GetOrCreateStorage<Type>().Add(std::move(first), std::move(last), component);
		}

		/** Constructs a component with the provided arguments and associates it with the provided entity. Replaces existing component if it already exists. */
		template<typename Type, typename... Arguments>
		Type& AddOrReplaceComponent(Entity entity, Arguments&&... arguments)
		{
			auto& storage = GetOrCreateStorage<Type>();
			if(storage.Contains(entity))
			{
				Type& component = storage.Get(entity);
				component = Type{std::forward<Arguments>(arguments)...};
				return component;
			}

			return storage.template Add<Type>(entity, std::forward<Arguments>(arguments)...);
		}

		/**
		 * Adds a tag to an entity. Tags are empty components that track only presence/absence.
		 * Does nothing if the tag is already present.
		 *
		 * @tparam TagType  Empty struct type representing the tag. Must satisfy std::is_empty_v.
		 */
		template<typename TagType>
		void AddTag(Entity entity)
		{
			static_assert(std::is_empty_v<TagType>, "AddTag only works with empty tag types. Use AddComponent for data components.");
			auto& storage = GetOrCreateStorage<TagType>();
			if(!storage.Contains(entity))
				storage.Add(entity);
		}

		/**
		 * Removes a tag from an entity if present. Convenience wrapper around RemoveComponents for tag types.
		 *
		 * @tparam TagType  Empty struct type representing the tag. Must satisfy std::is_empty_v.
		 */
		template<typename TagType>
		void RemoveTag(Entity entity)
		{
			static_assert(std::is_empty_v<TagType>, "RemoveTag only works with empty tag types. Use RemoveComponents for data components.");
			RemoveComponents<TagType>(entity);
		}

		/** Removes one or multiple components from an entity. */
		template<typename FirstComponentType, typename... OtherComponentType>
		u64 RemoveComponents(Entity entity)
		{
			return (GetOrCreateStorage<FirstComponentType>().EraseIfValid(entity) + ... + GetOrCreateStorage<OtherComponentType>().EraseIfValid(entity));
		}

		/** Removes one or multiple components from a range of entities. */
		template<typename FirstComponentType, typename... OtherComponentType, typename It>
		u64 RemoveComponents(It first, It last)
		{
			u64 count = 0;
			auto relevantComponentStorageTuple = std::forward_as_tuple(GetOrCreateStorage<FirstComponentType>(), GetOrCreateStorage<OtherComponentType>()...);
			for(; first != last; ++first)
				count = std::apply([entity = *first](auto&... storage) { return (storage.EraseIfValid(entity) + ... + 0u); }, relevantComponentStorageTuple);

			return count;
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. 
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<typename... ComponentType>
		decltype(auto) GetComponents(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. 
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing all the requested components, if multiple components are requested.
		 *								A single component if only one component is requested.
		 */
		template<typename... ComponentType>
		decltype(auto) GetComponents(Entity entity)
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. 
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing pointers to all the requested components, if multiple components are requested.
		 *								A pointer to a single component if only one component is requested.
		 *								Returns a null pointer if a component of specific type doesn't exist for the entity.
		 */
		template<typename... ComponentType>
		decltype(auto) TryGetComponents(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
			{
				const auto& storage = TryGetStorage<std::remove_const_t<ComponentType>...>();
				return (storage != nullptr && storage.Contains(entity) ? &storage.Get(entity) : nullptr);
			}
			else
				return std::forward_as_tuple(TryGetComponents<ComponentType>(entity)...);
		}

		/**
		 * Returns a tuple containing a subset of components for the specified entity. 
		 *
		 * @param	entity				Entity to retrieve the components for.
		 * @return						Tuple containing pointers to all the requested components, if multiple components are requested.
		 *								A pointer to a single component if only one component is requested.
		 *								Returns a null pointer if a component of specific type doesn't exist for the entity.
		 */
		template<typename... ComponentType>
		decltype(auto) TryGetComponents(Entity entity)
		{
			if constexpr(sizeof...(ComponentType) == 1u)
			{
				auto& storage = TryGetStorage<std::remove_const_t<ComponentType>...>();
				return (storage != nullptr && storage.Contains(entity) ? &storage.Get(entity) : nullptr);
			}
			else
				return std::forward_as_tuple(TryGetComponents<ComponentType>(entity)...);
		}

		/** Attempts to retrieve a component associated with the entity, or if one doesn't exist, constructs it using the provided arguments. */
		template<typename Type, typename... Arguments>
		Type& GetOrAddComponent(Entity entity, Arguments&&... arguments)
		{
			auto& storage = GetOrCreateStorage<Type>();
			return storage.Contains(entity) ? storage.Get(entity) : storage.Add(entity, std::forward<Arguments>(arguments)...);
		}

		/**
		 * Creates a view using the provided types as a filter. View will allow for iteration over all entities containing components that match the types in the included & excluded type filter.
		 * 
		 * @tparam FirstIncludedType		Type of the component that an entity must contain in order to be included in the view.
		 * @tparam OtherIncludedTypes		Additional types of components that an entity must contain in order to be included in the view.
		 * @tparam ExcludedTypes			Optional types of components that the entity must not contain in order to be included in the view.
		 */
		template<typename FirstIncludedType, typename... OtherIncludedTypes, typename... ExcludedTypes>
		TView<TIncludedTypes<TStorageType<const FirstIncludedType>, TStorageType<const OtherIncludedTypes>...>, TExcludedTypes<TStorageType<const ExcludedTypes>...>>
		CreateView(TExcludedTypes<ExcludedTypes...> = TExcludedTypes<ExcludedTypes...>{}) const
		{
			TView<TIncludedTypes<TStorageType<const FirstIncludedType>, TStorageType<const OtherIncludedTypes>...>, TExcludedTypes<TStorageType<const ExcludedTypes>...>> view;

			[&view](const auto*... storage)
			{
				((storage != nullptr ? view.SetStorage(*storage) : void()), ...);
			}(TryGetStorage<std::remove_const_t<FirstIncludedType>>(), TryGetStorage<std::remove_const_t<OtherIncludedTypes>>()..., TryGetStorage<std::remove_const_t<ExcludedTypes>>()...);

			return view;
		}

		/**
		 * Creates a view using the provided types as a filter. View will allow for iteration over all entities containing components that match the types in the included & excluded type filter.
		 * 
		 * @tparam FirstIncludedType		Type of the component that an entity must contain in order to be included in the view.
		 * @tparam OtherIncludedTypes		Additional types of components that an entity must contain in order to be included in the view.
		 * @tparam ExcludedTypes			Optional types of components that the entity must not contain in order to be included in the view.
		 */
		template<typename FirstIncludedType, typename... OtherIncludedTypes, typename... ExcludedTypes>
		TView<TIncludedTypes<TStorageType<FirstIncludedType>, TStorageType<OtherIncludedTypes>...>, TExcludedTypes<TStorageType<ExcludedTypes>...>>
		CreateView(TExcludedTypes<ExcludedTypes...> = TExcludedTypes<ExcludedTypes...>{}) 
		{
			TView<TIncludedTypes<TStorageType<FirstIncludedType>, TStorageType<OtherIncludedTypes>...>, TExcludedTypes<TStorageType<ExcludedTypes>...>> view;

			view.SetStorage(GetOrCreateStorage<std::remove_const_t<FirstIncludedType>>());
			(view.SetStorage(GetOrCreateStorage<std::remove_const_t<OtherIncludedTypes>>()), ...);
			(view.SetStorage(GetOrCreateStorage<std::remove_const_t<ExcludedTypes>>()), ...);

			return view;
		}

		/**
		 * Creates a group using the provided types as a filter. A group is similar to a view, as it will allow for iteration over all entities containing components that match the types
		 * in the owned, included & excluded type filter. However group will also ensure that all components in the owned type list are tightly packed, ensuring they can be quickly iterated
		 * over. Owned components may also be sorted using custom rules. Group therefore modifies the storage of owned components, and therefore a single component storage may only be owned
		 * by a single group.
		 *
		 * @tparam	OwnedTypes		Zero or more types that the group should own. If zero types are provided then a non-owning group is created, which is a special case that's
		 *							similar to a view, but ensures that entities in a group are tightly packed and can be sorted using a custom rule.
		 * @tparam	IncludedTypes	Optional types of components that an entity must contain in order to be included in the group. For non-owning groups, at least one included
		 *							type must be provided.
		 * @tparam	ExcludedTypes	Optional types of components that the entity must not contain in order to be included in the group.
		 */
		template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
		TGroup<TOwnedTypes<TStorageType<OwnedTypes>...>, TIncludedTypes<TStorageType<IncludedTypes>...>, TExcludedTypes<TStorageType<ExcludedTypes>...>>
		GetOrCreateGroup(TIncludedTypes<IncludedTypes...> = TIncludedTypes<IncludedTypes...>{}, TExcludedTypes<ExcludedTypes...> = TExcludedTypes<ExcludedTypes...>{})
		{
			using GroupType = TGroup<TOwnedTypes<TStorageType<OwnedTypes>...>, TIncludedTypes<TStorageType<IncludedTypes>...>, TExcludedTypes<TStorageType<ExcludedTypes>...>>;
			using InternalsType = typename GroupType::GroupInternalsType;

			if(auto found = mGroupStorage.find(GroupType::TypeId()); found != mGroupStorage.end())
				return GroupType(*std::static_pointer_cast<InternalsType>(found->second));

			SPtr<InternalsType> internals;
			if constexpr(sizeof...(OwnedTypes) == 0)
				internals = B3DMakeShared<InternalsType>(std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<IncludedTypes>>()...), std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<ExcludedTypes>>()...));
			else
			{
				// Ensure no other group owns any of types the new group is meant to own
				if(!B3D_ENSURE(std::all_of(mGroupStorage.begin(), mGroupStorage.end(), [](const auto& entry) -> bool { return !(entry.second->OwnsType(B3DGetTypeHash<OwnedTypes>()) || ...); })))
					return GroupType();

				internals = B3DMakeShared<InternalsType>(std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<OwnedTypes>>()..., GetOrCreateStorage<std::remove_const_t<IncludedTypes>>()...), std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<ExcludedTypes>>()...));
			}

			mGroupStorage[GroupType::TypeId()] = internals;
			return GroupType(*internals);
		}

		/** Retrieves an existing group that has the provided type filter, or an invalid group if one doesn't exist. See GetOrCreateGroup. */
		template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
		TGroup<TOwnedTypes<TStorageType<OwnedTypes>...>, TIncludedTypes<TStorageType<IncludedTypes>...>, TExcludedTypes<TStorageType<ExcludedTypes>...>>
		GetGroup(TIncludedTypes<IncludedTypes...> = TIncludedTypes<IncludedTypes...>{}, TExcludedTypes<ExcludedTypes...> = TExcludedTypes<ExcludedTypes...>{})
		{
			using GroupType = TGroup<TOwnedTypes<TStorageType<OwnedTypes>...>, TIncludedTypes<TStorageType<IncludedTypes>...>, TExcludedTypes<TStorageType<ExcludedTypes>...>>;
			using InternalsType = typename GroupType::GroupInternalsType;

			if(auto found = mGroupStorage.find(GroupType::TypeId()); found != mGroupStorage.end())
				return GroupType(*std::static_pointer_cast<InternalsType>(found.second));

			return GroupType();
		}

		/** Sorts the storage containing component of type @p Type, using the provided comparison function. */
		template<typename Type, typename ComparisonFunction = std::less<>>
		void Sort()
		{
			GetOrCreateStorage<Type>().template Sort<ComparisonFunction>();
		}

		/** Sorts storage of components @p TypeToSort in the same order as in the storage as @p TypeToSortAs. */
		template<typename TypeToSort, typename TypeToSortAs>
		void SortAs()
		{
			const SparseSet& sortAsStorage = GetOrCreateStorage<TypeToSortAs>();
			GetOrCreateStorage<TypeToSort>().SortAs(sortAsStorage.Begin(), sortAsStorage.End());
		}

		/** Shrinks the memory use of the set to accomodate the currently assigned entries for all storages, without any reserve for new entries. */
		template<typename... Type>
		void Shrink()
		{
			if constexpr(sizeof...(Type) == 0u)
			{
				for(auto&& storage : mComponentStorage)
					storage.second->Shrink();
			}
			else
			{
				(GetOrCreateStorage<Type>().Shrink(), ...);
			}
		}

		/** Returns true if the entity has all components of the provided types. */
		template<typename... ComponentType>
		bool HasAllOf(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
			{
				auto* storage = TryGetStorage<std::remove_const_t<ComponentType>...>();
				return storage != nullptr && storage->Contains(entity);
			}
			else
				return (HasAllOf<ComponentType>(entity) && ...);
		}

		/** Returns true if the entity has any components of the provided types. */
		template<typename... ComponentType>
		bool HasAnyOf(Entity entity) const
		{
			return (HasAllOf<ComponentType>(entity) || ...);
		}

	private:
		EntitySparseSet mEntityStorage;
		UnorderedMap<TypeId, SPtr<SparseSet>> mComponentStorage;
		UnorderedMap<TypeId, SPtr<GroupCommon>> mGroupStorage;
	};

	/** @} */
} // namespace b3d::ecs
