//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Debug/BsDebug.h"
#include "ECS/BsEntity.h"
#include "ECS/BsUtility.h"
#include "ECS/BsComponentStorage.h"
#include "ECS/BsTagStorage.h"
#include "ECS/BsEntityStorage.h"
#include "ECS/BsView.h"
#include "ECS/BsGroup.h"

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	class Registry
	{
	public:
		const SparseSet* TryGetStorage(TypeId typeId) const
		{
			if(auto found = mComponentStorage.find(typeId); found != mComponentStorage.end())
				return found->second.get();

			return nullptr;
		}

		SparseSet* TryGetStorage(TypeId typeId)
		{
			return const_cast<SparseSet*>(std::as_const(*this).TryGetStorage(typeId));
		}

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

		template<typename Type>
		TStorageType<Type>* TryGetStorage()
		{
			return const_cast<TStorageType<Type>*>(std::as_const(*this).TryGetStorage<Type>());
		}

		bool RemoveStorage(TypeId typeId)
		{
			return mComponentStorage.erase(typeId) > 0;
		}

		template<typename Type>
		bool RemoveStorage()
		{
			const TypeId typeId = B3DGetRuntimeTypeId<Type>();
			return RemoveStorage(typeId);
		}

		template<typename... Type>
		void ClearStorage()
		{
			if(sizeof...(Type) == 0u)
				return;

			(GetOrCreateStorage<Type>().Clear(), ...);
		}

		void Clear()
		{
			for(auto& entry : mComponentStorage)
				entry.second->Clear();
			
			mEntityStorage.Clear();
		}

		bool IsEntityValid(Entity entity) const
		{
			if(auto found = mEntityStorage.Find(entity); found != mEntityStorage.End())
				return found.Index() < mEntityStorage.GetFirstFreeElementPackedIndex();

			return false;
		}

		bool HasEntityAnyComponents(Entity entity) const
		{
			for(auto& entry : mComponentStorage)
			{
				if(entry.second->Contains(entity))
					return true;
			}

			return false;
		}

		Entity::VersionType GetEntityVersion(Entity entity) const
		{
			return mEntityStorage.GetVersion(entity);
		}

		Entity CreateEntity()
		{
			return mEntityStorage.Create();
		}

		Entity CreateEntity(Entity hint)
		{
			return mEntityStorage.Create(hint);
		}

		Entity::VersionType DestroyEntity(Entity entity) // TODO - Inconsistent naming erase vs. delete
		{
			for(auto& entry : mComponentStorage)
				entry.second->EraseIfValid(entity);

			mEntityStorage.EraseIfValid(entity);
			return mEntityStorage.GetVersion(entity);
		}

		Entity::VersionType DestroyEntity(Entity entity, Entity::VersionType newVersion)
		{
			DestroyEntity(entity);

			Entity destroyedEntity(entity.GetIdentifier(), newVersion);
			if(destroyedEntity == kInvalidEntity)
				destroyedEntity = destroyedEntity.GetAsNextVersion();

			mEntityStorage.UpdateVersion(destroyedEntity);
			return destroyedEntity.GetVersion();
		}

		template<typename It>
		void DestroyEntities(It first, It last)
		{
			// Note: Deleting from the end would be more efficient. Perhaps in the future.
			const auto from = mEntityStorage.Begin();
			const auto to = mEntityStorage.SortAs(first, last);

			for(auto& entry : mComponentStorage)
				entry.second->EraseIfValid(from, to);

			mEntityStorage.Erase(from, to);
		}

		template<typename Type, typename... Arguments>
		Type& AddComponent(Entity entity, Arguments&&... arguments)
		{
			return GetOrCreateStorage<Type>().Add(entity, std::forward<Arguments>(arguments)...);
		}

		template<typename Type, typename It>
		void AddComponents(It first, It last, const Type& component = {})
		{
			B3D_ASSERT(std::all_of(first, last, [this](Entity entity) { return IsEntityValid(entity); }));
			GetOrCreateStorage<Type>().Add(std::move(first), std::move(last), component);
		}

		template<typename Type, typename... Arguments>
		Type& AddOrReplaceComponent(Entity entity, Arguments&&... arguments)
		{
			auto storage = GetOrCreateStorage<Type>();
			if(storage.Contains(entity))
			{
				Type& component = storage.Get(entity);
				component = Type{std::forward<Arguments>(arguments)...};
				return component;
			}

			return storage.template Add<Type>(entity, std::forward<Arguments>(arguments)...);
		}

		template<typename FirstComponentType, typename... OtherComponentType>
		u64 RemoveComponents(Entity entity)
		{
			return (GetOrCreateStorage<FirstComponentType>().EraseIfValid(entity) + ... + GetOrCreateStorage<OtherComponentType>(entity));
		}

		template<typename FirstComponentType, typename... OtherComponentType, typename It>
		u64 RemoveComponents(It first, It last)
		{
			u64 count = 0;
			auto relevantComponentStorageTuple = std::forward_as_tuple(GetOrCreateStorage<FirstComponentType>(), GetOrCreateStorage<OtherComponentType>()...);
			for(; first != last; ++first)
				count = std::apply([entity = *first](auto&... storage) { return (storage.EraseIfValid(entity) + ... + 0u); }, relevantComponentStorageTuple);

			return count;
		}

		template<typename... ComponentType>
		decltype(auto) GetComponents(Entity entity) const
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		template<typename... ComponentType>
		decltype(auto) GetComponents(Entity entity)
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

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

		template<typename... ComponentType>
		decltype(auto) TryGetComponents(Entity entity)
		{
			if constexpr(sizeof...(ComponentType) == 1u)
				return (TryGetStorage<std::remove_const_t<ComponentType>>()->Get(entity), ...);
			else
				return std::forward_as_tuple(GetComponents<ComponentType>(entity)...);
		}

		template<typename Type, typename... Arguments>
		Type& GetOrAddComponent(Entity entity, Arguments&&... arguments)
		{
			auto& storage = GetOrCreateStorage<Type>();
			return storage.Contains(entity) ? storage.Get(entity) : storage.Add(entity, std::forward<Arguments>(arguments)...);
		}

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

		template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
		TGroup<TOwnedTypes<OwnedTypes...>, TIncludedTypes<IncludedTypes...>, TExcludedTypes<ExcludedTypes...>>
		GetOrCreateGroup(TIncludedTypes<IncludedTypes...> = TIncludedTypes<IncludedTypes...>{}, TExcludedTypes<ExcludedTypes...> = TExcludedTypes<ExcludedTypes...>{})
		{
			using GroupType = TGroup<TOwnedTypes<OwnedTypes...>, TIncludedTypes<IncludedTypes...>, TExcludedTypes<ExcludedTypes...>>;
			using InternalsType = typename GroupType::GroupInternalsType;

			if(auto found = mGroupStorage.find(GroupType::TypeId()); found != mGroupStorage.end())
				return GroupType(*std::static_pointer_cast<InternalsType>(found.second));

			SPtr<InternalsType> internals;
			if constexpr(sizeof...(OwnedTypes) == 0)
				internals = B3DMakeShared<InternalsType>(std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<IncludedTypes>>()...), std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<ExcludedTypes>>()...));
			else
			{
				// Ensure no other group owns any of types the new group is meant to own
				if(!B3D_ENSURE(std::all_of(mGroupStorage.begin(), mGroupStorage.end(), [](const auto& entry) -> bool { return !(entry.second->OwnsType(B3DGetRuntimeTypeId<OwnedTypes>()) || ...); })))
					return GroupType();

				internals = B3DMakeShared<InternalsType>(std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<OwnedTypes>>()...), std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<IncludedTypes>>()...), std::forward_as_tuple(GetOrCreateStorage<std::remove_const_t<ExcludedTypes>>()...));
			}

			mGroupStorage[GroupType::TypeId()] = internals;
			return GroupType(*internals);
		}

		template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
		TGroup<TOwnedTypes<OwnedTypes...>, TIncludedTypes<IncludedTypes...>, TExcludedTypes<ExcludedTypes...>>
		GetGroup(TIncludedTypes<IncludedTypes...> = TIncludedTypes<IncludedTypes...>{}, TExcludedTypes<ExcludedTypes...> = TExcludedTypes<ExcludedTypes...>{})
		{
			using GroupType = TGroup<TOwnedTypes<OwnedTypes...>, TIncludedTypes<IncludedTypes...>, TExcludedTypes<ExcludedTypes...>>;
			using InternalsType = typename GroupType::GroupInternalsType;

			if(auto found = mGroupStorage.find(GroupType::TypeId()); found != mGroupStorage.end())
				return GroupType(*std::static_pointer_cast<InternalsType>(found.second));

			return GroupType();
		}

		template<typename Type, typename ComparisonFunction = std::less<>>
		void Sort()
		{
			GetOrCreateStorage<Type>().template Sort<ComparisonFunction>();
		}

		template<typename TypeToSort, typename TypeToSortAs>
		void SortAs()
		{
			const SparseSet& sortAsStorage = GetOrCreateStorage<TypeToSortAs>();
			GetOrCreateStorage<TypeToSort>().SortAs(sortAsStorage.Begin(), sortAsStorage.End());
		}

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

		template<typename... ComponentType>
		bool HasAnyOf(Entity entity) const
		{
			return (HasAllOf<ComponentType>(entity) || ...);
		}

	private:
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

		EntitySparseSet mEntityStorage;
		UnorderedMap<TypeId, SPtr<SparseSet>> mComponentStorage;
		UnorderedMap<TypeId, SPtr<GroupInternals>> mGroupStorage;
	};

	/** @} */
} // namespace b3d::ecs
