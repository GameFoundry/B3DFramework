//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DUtility.h"
#include "ECS/B3DSparseSet.h"

#include <iterator>

namespace b3d::ecs
{
	/** @addtogroup ECS-Internal
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/** Allows iteration over an array that is split over multiple pages of equal size. All allocations within a page are sequential. */
 	template<typename ContainerType, u32 PageSize>
	struct TPagedContainerIterator final
	{
 	private:
		using iterator_traits = std::iterator_traits<std::conditional_t<
			std::is_const_v<ContainerType>,
			const typename std::pointer_traits<typename std::remove_const_t<ContainerType>::value_type>::element_type*,
			typename std::pointer_traits<typename std::remove_const_t<ContainerType>::value_type>::element_type*>>;

 	public:
		using value_type = typename iterator_traits::value_type;
		using pointer = typename iterator_traits::pointer;
		using reference = typename iterator_traits::reference;
		using difference_type = typename iterator_traits::difference_type;
		using iterator_category = std::random_access_iterator_tag;

		TPagedContainerIterator() = default;
		TPagedContainerIterator(ContainerType& container, u64 index)
			: mContainer(&container), mIndex(index) { }

		TPagedContainerIterator& operator++()
		{
			++mIndex;
			return *this;
		}

		TPagedContainerIterator& operator--()
		{
			B3D_ENSURE(mIndex > 0);

			--mIndex;
			return *this;
		}

		TPagedContainerIterator& operator+=(u64 value)
		{
			mIndex += value;
			return *this;
		}

		TPagedContainerIterator& operator-=(u64 value)
		{
			B3D_ENSURE(mIndex >= value);

			mIndex -= value;
			return *this;
		}

		TPagedContainerIterator operator+(u64 value) const
		{
			TPagedContainerIterator copy = *this;
			return (copy += value);
		}

		TPagedContainerIterator operator-(u64 value) const
		{
			TPagedContainerIterator copy = *this;
			return (copy -= value);
		}

		reference operator[](u64 value) const
		{
			const u64 index = mIndex + value;
			return (*mContainer)[index / PageSize][index & (PageSize - 1)];
		}
		pointer operator->() const { return std::addressof(operator[](0)); }
		reference operator*() const { return operator[](0); }

		u64 Index() const { return mIndex; }

	private:
		ContainerType* mContainer = nullptr;
		u64 mIndex = 0;
	};

	template<typename ContainerType, u32 PageSize>
	i64 operator-(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return rhs.Index() - lhs.Index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator==(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return lhs.Index() == rhs.Index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator!=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename ContainerType, u32 PageSize>
	bool operator<(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return lhs.index() > rhs.index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator>(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return rhs < lhs;
	}

	template<typename ContainerType, u32 PageSize>
	bool operator<=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs > rhs);
	}

	template<typename ContainerType, u32 PageSize>
	bool operator>=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs < rhs);
	}

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	/**
	 * Storage used for storing components associated with entities.
	 *
	 * @tparam	ComponentType	Type of the component to store. Type must not be empty.
	 * @tparam	InPlaceDelete	If true, deletion of entities will not erase the entity, but rather just mark it as invalid. Space of invalid entities
	 *							can be re-used, but iterating such storages is slower. Additionally sorting operations are limited when invalid
	 *							entities are present in the storage.
	 * @tparam	PackedPageSize	Size of a single payload page. In order to reduce the impact of array resize, payload array is split into pages of this size.
	 */
	template<typename ComponentType, bool InPlaceDelete = false, u64 PackedPageSize = 1024>
	class TComponentSparseSet : public TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase>
	{
		using ComponentContainerType = TArray<TArrayView<ComponentType>>;

	public:
		using ElementType = ComponentType;
		using Super = TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase>;
		using Iterator = TPagedContainerIterator<ComponentContainerType, PackedPageSize>;
		using ConstIterator = TPagedContainerIterator<const ComponentContainerType, PackedPageSize>;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;
		using IteratorRange = TIteratorRange<TMultiIteratorAdapter<typename Super::Iterator, Iterator>>;
		using ConstIteratorRange = TIteratorRange<TMultiIteratorAdapter<typename Super::ConstIterator, ConstIterator>>;
		using ReverseIteratorRange = TIteratorRange<TMultiIteratorAdapter<typename Super::ReverseIterator, ReverseIterator>>;
		using ConstReverseIteratorRange = TIteratorRange<TMultiIteratorAdapter<typename Super::ConstReverseIterator, ConstReverseIterator>>;

		TComponentSparseSet()
			:TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase>(B3DGetTypeHash<ComponentType>())
		{ }

		~TComponentSparseSet() override
		{
			ShrinkComponentArray(0);
		}

		/** Adds an entity and associates a newly constructed component of type @p Type using the provided arguments. */
		template<typename... Arguments>
		ComponentType& Add(Entity entity, Arguments&&... arguments)
		{
			auto iterator = AddInternal(entity, false, std::forward<Arguments>(arguments)...);
			return GetComponentReference(iterator.Index());
		}

		/** Adds multiple entities and associates the same component value to them. */
		template<typename It>
		void Add(It first, It last, const ComponentType& value)
		{
			for(It it = first; it != last; ++it)
				AddInternal(*it, true, value);
		}

		/** Returns the component value for the specified entity. Caller must ensure storage contains the entity. */
		ComponentType& Get(Entity entity)
		{
			const u64 packedEntryIndex = Super::GetPackedIndex(entity);
			return GetComponentReference(packedEntryIndex);
		}

		/** Returns the component value for the specified entity. Caller must ensure storage contains the entity. */
		const ComponentType& Get(Entity entity) const
		{
			const u64 packedEntryIndex = Super::GetPackedIndex(entity);
			return GetComponentReference(packedEntryIndex);
		}

		void Clear() override
		{
			if constexpr(InPlaceDelete)
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					if(*it != kInvalidEntity)
					{
						Super::OnWillRemove(*it);

						ComponentType& component = GetComponentReference(this->GetPackedIndex(*it));
						component.~ComponentType();
					}
				}
			}
			else
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					Super::OnWillRemove(*it);

					ComponentType& component = GetComponentReference(this->GetPackedIndex(*it));
					component.~ComponentType();
				}
			}

			Super::ClearInternal();
		}

		void ClearInvalid() override
		{
			Super::template ClearInvalidInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload>();
		}

		void Swap(Entity lhs, Entity rhs) override
		{
			Super::template SwapInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload>(lhs, rhs);
		}

		/** @copydoc TSparseSet::SortAs */
		template<typename It>
		Iterator SortAs(It first, It last)
		{
			return Super::template SortAsInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload>(first, last);
		}

		/** @copydoc TSparseSet::SortN */
		template<typename ComparisonFunction = std::less<>>
		void SortN(u64 count, ComparisonFunction predicate = ComparisonFunction{})
		{
			return Super::template SortNInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload, ComparisonFunction>(count, std::move(predicate));
		}

		/** @copydoc TSparseSet::Sort */
		template<typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = ComparisonFunction{})
		{
			return Super::template SortInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload, ComparisonFunction>(std::move(predicate));
		}

		void Reserve(u64 capacity) override
		{
			if(capacity > 0)
			{
				Super::Reserve(capacity);
				GetOrCreateComponentPageFor(capacity - 1u);
			}
		}

		u64 Capacity() const override
		{
			return mComponents.Size() * PackedPageSize;
		}

		void Shrink() override
		{
			Super::Shrink();
			ShrinkComponentArray(Super::Size());
		}

		/** Allows easy iteration over all components using a range for loop. */
		IteratorRange Each() { return IteratorRange({ Super::Begin(), Begin() }, { Super::End(), End() }); }
		ConstIteratorRange Each() const { return ConstIteratorRange({ Super::Cbegin(), Cbegin() }, { Super::Cend(), Cend() }); }

		/** Allows easy iteration over all components using a range for loop, in reverse order. */
		ReverseIteratorRange ReverseEach() { return ReverseIteratorRange({ Super::Rbegin(), Rbegin() }, { Super::Rend(), Rend() }); }
		ConstReverseIteratorRange ReverseEach() const { return ConstReverseIteratorRange({ Super::Crbegin(), Crbegin() }, { Super::Crend(), Crend() }); }

		/** Iterator to the first component in the storage. */
		ConstIterator Cbegin() const { return ConstIterator(mComponents, 0); }
		ConstIterator Begin() const { return Cbegin(); }
		Iterator Begin() { return Iterator(mComponents, 0); }

		/** Iterator past the last component in the storage. */
		ConstIterator Cend() const { return ConstIterator(mComponents, Super::Size()); }
		ConstIterator End() const { return Cend(); }
		Iterator End() { return Iterator(mComponents, Super::Size()); }

		/** Iterator to the last component in the storage, for reverse iteration. */
		ConstReverseIterator Crbegin() const { return std::make_reverse_iterator(Cend()); }
		ConstReverseIterator Rbegin() const { return Crbegin(); }
		ReverseIterator Rbegin() { return std::make_reverse_iterator(End()); }

		/** Iterator to before the first component in the storage, for reverse iteration. */
		ConstReverseIterator Crend() const { return std::make_reverse_iterator(Cbegin()); }
		ConstReverseIterator Rend() const { return Crend(); }
		ReverseIterator Rend() { return std::make_reverse_iterator(Begin()); }

		// For std compatibility
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using reverse_iterator = ReverseIterator;
		using const_reverse_iterator = ConstReverseIterator;

		const_iterator cbegin() const { return Cbegin(); }
		const_iterator begin() const { return Begin(); }
		iterator begin() { return Begin(); }

		const_iterator cend() const { return Cend(); }
		const_iterator end() const { return End(); }
		iterator end() { return End(); }

		const_reverse_iterator crbegin() const { return Crbegin(); }
		const_reverse_iterator rbegin() const { return Rbegin(); }
		reverse_iterator rbegin() { return Rend(); }

		const_reverse_iterator crend() const { return Crend(); }
		const_reverse_iterator rend() const { return Rend(); }
		reverse_iterator rend() { return Rend(); }

	private:
		using UnderlyingIterator = typename Super::Iterator;

		/**
		 * Adds an entity and associates a newly constructed component of type @p Type using the provided arguments. If @p forceAddAtEnd is true
		 * ensures that all added entries are added to the end of the component array, rather than re-using invalid (deleted) entries (only
		 * relevant if in-place deletion is true).
		 */
		template<typename... Arguments>
		UnderlyingIterator AddInternal(Entity entity, bool forceAddAtEnd, Arguments&&... arguments)
		{
			auto iterator = Super::AddInternal(entity, forceAddAtEnd);
			ComponentType* component = GetOrCreateComponentPointer(iterator.Index());
			new(component) ComponentType(std::forward<Arguments>(arguments)...);

			Super::OnWasAdded(*iterator);
			return iterator;
		}

		void EraseInternal(Entity entity) override
		{
			const u64 packedIndex = Super::GetPackedIndex(entity);
			ComponentType& component = GetComponentReference(packedIndex);

			if constexpr(InPlaceDelete)
			{
				Super::EraseInternal(entity);
				component.~ComponentType();
			}
			else // SwapAndErase
			{
				ComponentType& otherComponent = GetComponentReference(Super::Size() - 1);
				component = std::move(otherComponent);
				otherComponent.~ComponentType();

				Super::EraseInternal(entity);
			}
		}

		/** Moves the component from @p fromPackedIndex to @p toPackedIndex. */
		void MoveOrSwapPayload(u64 fromPackedIndex, u64 toPackedIndex)
		{
			constexpr bool isMovable = std::is_move_constructible_v<ComponentType> && std::is_move_assignable_v<ComponentType>;
			B3D_ASSERT(isMovable);

			if constexpr(isMovable)
			{
				if constexpr(InPlaceDelete)
				{
					if(Super::operator[](toPackedIndex) == kInvalidEntity)
					{
						ComponentType& componentToMove = GetComponentReference(fromPackedIndex);

						ComponentType* destinationComponent = GetOrCreateComponentPointer(toPackedIndex);
						new(destinationComponent) ComponentType(std::move(componentToMove));

						componentToMove.~ComponentType();
					}
					else
						std::swap(GetComponentReference(fromPackedIndex), GetComponentReference(toPackedIndex));
				}
				else
					std::swap(GetComponentReference(fromPackedIndex), GetComponentReference(toPackedIndex));
			}
		}

		/** Destroys any excess pages, as long as the internal contain can fit @p newComponentCount. */
		void ShrinkComponentArray(u64 newComponentCount)
		{
			const u64 oldComponentCount = Super::Size();
			for(u64 packedComponentIndex = newComponentCount; packedComponentIndex < oldComponentCount; ++packedComponentIndex)
			{
				if constexpr(InPlaceDelete)
				{
					if(Super::mPackedEntities[packedComponentIndex] != kInvalidEntity)
						GetComponentReference(packedComponentIndex).~ComponentType();
				}
				else
					GetComponentReference(packedComponentIndex).~ComponentType();
			}

			const auto newPageCount = (newComponentCount + PackedPageSize - 1u) / PackedPageSize;
			const u64 oldPageCount = mComponents.Size();
			for(u64 page = newPageCount; page < oldPageCount; ++page)
				B3DFree(mComponents[page].Data());

			mComponents.Resize(newPageCount);
			mComponents.Shrink();
		}

		/**
		 * Returns a page in which to store the component with the specified index at. Internally allocates a page for
		 * the provided index if one doesn't exist.
		 */
		ComponentType* GetOrCreateComponentPageFor(u64 packedComponentIndex)
		{
			const u64 page = GetComponentPage(packedComponentIndex);

			if(page >= mComponents.Size())
				mComponents.Resize(page + 1);

			if(mComponents[page].IsEmpty())
				mComponents[page] = TArrayView(B3DAllocateMultiple<ComponentType>(PackedPageSize), PackedPageSize);

			return mComponents[page].Data();
		}

		/**
		 * Returns a pointer to store the component with the specified index at. Internally allocates a page for
		 * the provided index if one doesn't exist.
		 */
		ComponentType* GetOrCreateComponentPointer(u64 packedComponentIndex)
		{
			return GetOrCreateComponentPageFor(packedComponentIndex) + GetComponentIndexWithinPage(packedComponentIndex);
		}

		/**
		 * Returns a reference to store the component with the specified index at. Internally allocates a page for
		 * the provided index if one doesn't exist.
		 */
		ComponentType& GetComponentReference(u64 packedComponentIndex)
		{
			return mComponents[GetComponentPage(packedComponentIndex)][GetComponentIndexWithinPage(packedComponentIndex)];
		}

		/**
		 * Returns a reference to store the component with the specified index at. Internally allocates a page for
		 * the provided index if one doesn't exist.
		 */
		const ComponentType& GetComponentReference(u64 packedComponentIndex) const
		{
			return mComponents[GetComponentPage(packedComponentIndex)][GetComponentIndexWithinPage(packedComponentIndex)];
		}

		/** Converts a global packed component index, into an index that's local to the page. */
		static constexpr u64 GetComponentIndexWithinPage(u64 packedComponentIndex)
		{
			return packedComponentIndex & (PackedPageSize - 1);
		}

		/** Converts a global packed component index, into a page index. */
		static constexpr u64 GetComponentPage(u64 packedComponentIndex)
		{
			return packedComponentIndex / PackedPageSize;
		}

		ComponentContainerType mComponents;
	};

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::conjunction_v<std::is_move_constructible<std::remove_const_t<Type>>, std::is_move_assignable<std::remove_const_t<Type>>, std::negation<std::is_empty<Type>>>>>
	{
		using StorageType = TInheritConstFrom<TComponentSparseSet<std::remove_const_t<Type>>, Type>;
	};

	template<typename Type>
	struct StorageForType<Type, std::enable_if_t<std::conjunction_v<std::disjunction<std::negation<std::is_move_constructible<std::remove_const_t<Type>>>, std::negation<std::is_move_assignable<std::remove_const_t<Type>>>>, std::negation<std::is_empty<Type>>>>>
	{
		using StorageType = TInheritConstFrom<TComponentSparseSet<std::remove_const_t<Type>, true>, Type>;
	};

	/** @} */
} // namespace b3d
