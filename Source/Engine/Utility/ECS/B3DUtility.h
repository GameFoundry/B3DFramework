//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "ECS/B3DEntity.h"
#include "Utility/B3DTypeList.h"

#include <iterator>

namespace b3d::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/** Moves a temporary value into the structure so may be passed around and accessed as if it was a pointer. */
	template<typename Type>
	struct TPointerToTemporary
	{
		constexpr TPointerToTemporary(Type&& value)
			: mValue{std::move(value)}
		{ }

		constexpr Type* operator->() noexcept { return &mValue; }
		constexpr Type& operator*() noexcept { return mValue; }

	private:
		Type mValue;
	};

	/** Contains a range between two iterators that can be iterated using a ranged for loop. */
	template <typename IteratorType>
	struct TIteratorRange
	{
		using value_type = typename std::iterator_traits<IteratorType>::value_type;
		using iterator = IteratorType;

		constexpr TIteratorRange() = default;
		constexpr TIteratorRange(IteratorType first, IteratorType last)
			: mFirst(std::move(first)), mLast(std::move(last))
		{ }

		constexpr IteratorType begin() const { return mFirst; }
		constexpr IteratorType end() const { return mLast; }

		constexpr IteratorType cbegin() const { return begin(); }
		constexpr IteratorType cend() const { return end(); }

	private:
		IteratorType mFirst;
		IteratorType mLast;
	};

	/** Performs iteration at multiple underlying iterators at once. Only supports forward iteration, one entry at a time. */
	template<typename BaseIteratorType, typename... OtherIteratorType>
	struct TMultiIteratorAdapter
	{
		using iterator_type = BaseIteratorType;
		using value_type = decltype(std::tuple_cat(std::make_tuple(*std::declval<BaseIteratorType>()), std::forward_as_tuple(*std::declval<OtherIteratorType>()...)));
		using difference_type = typename std::iterator_traits<BaseIteratorType>::difference_type;
		using pointer = TPointerToTemporary<value_type>;
		using reference = value_type;
		using iterator_category = std::input_iterator_tag;

		constexpr TMultiIteratorAdapter() = default;
		constexpr TMultiIteratorAdapter(BaseIteratorType firstIterator, OtherIteratorType... otherIterator)
			: mIterators(firstIterator, otherIterator...)
		{ }

		template <typename... OtherIteratorType2, typename = std::enable_if_t<(!std::is_same_v<OtherIteratorType, OtherIteratorType2> && ...) && (std::is_constructible_v<OtherIteratorType, OtherIteratorType2> && ...)>>
		constexpr TMultiIteratorAdapter(const TMultiIteratorAdapter<BaseIteratorType, OtherIteratorType2...>& other)
			: mIterators(other.mIterators)
		{}

		constexpr TMultiIteratorAdapter& operator++()
		{
			++std::get<BaseIteratorType>(mIterators);
			(++std::get<OtherIteratorType>(mIterators), ...);

			return *this;
		}

		constexpr pointer operator->() const noexcept
		{
			return operator*();
		}

		constexpr reference operator*() const noexcept
		{
			return { *std::get<BaseIteratorType>(mIterators), *std::get<OtherIteratorType>(mIterators)... };
		}

		constexpr iterator_type GetBaseIterator() const
		{
			return std::get<BaseIteratorType>(mIterators);
		}

		template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
		friend constexpr bool operator==(const TMultiIteratorAdapter<LeftIteratorTypes...>&, const TMultiIteratorAdapter<RightIteratorTypes...>&);

	private:
		std::tuple<BaseIteratorType, OtherIteratorType...> mIterators;
	};

	template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
	constexpr bool operator==(const TMultiIteratorAdapter<LeftIteratorTypes...>& lhs, const TMultiIteratorAdapter<RightIteratorTypes...>& rhs)
	{
		return lhs.GetBaseIterator() == rhs.GetBaseIterator();
	}

	template <typename... LeftIteratorTypes, typename... RightIteratorTypes>
	constexpr bool operator!=(const TMultiIteratorAdapter<LeftIteratorTypes...>& lhs, const TMultiIteratorAdapter<RightIteratorTypes...>& rhs)
	{
		return !(lhs == rhs);
	}


	/** Returns T as either 'T' or 'const T' depending if @p From is const or not. */
	template<typename T, typename From>
	struct TInheritConstFromHelper
	{
		using Type = std::remove_const_t<T>;
	};

	template<typename T, typename From>
	struct TInheritConstFromHelper<T, const From>
	{
		using Type = const T;
	};

	template<typename T, typename From>
	using TInheritConstFrom = typename TInheritConstFromHelper<T, From>::Type;

	/**
	 * Returns an entry from ECS storage as a tuple containing a single element. If the entity has no associated
	 * data (e.g. if storage contains tags), returns an empty tuple.
	 */
	template<typename StorageType>
	constexpr auto GetAsTuple(StorageType* storage, Entity entity)
	{
		if constexpr(std::is_void_v<decltype(std::declval<StorageType>().Get(entity))>)
			return std::tuple<>();
		else
			return std::forward_as_tuple(storage->Get(entity));
	}

	/** Helper type that can be used for providing a list of types to be included in view or group. */
	template<typename... Types>
	struct TIncludedTypes : TTypeList<Types...>
	{
		explicit constexpr TIncludedTypes() = default;
	};

	/** Helper type that can be used for providing a list of types to be excluded from view or group. */
	template<typename... Types>
	struct TExcludedTypes : TTypeList<Types...>
	{
		explicit constexpr TExcludedTypes() = default;
	};

	/** Helper type that can be used for providing a list of types owned by a group. */
	template<typename... Types>
	struct TOwnedTypes : TTypeList<Types...>
	{
		explicit constexpr TOwnedTypes() = default;
	};

	template<typename Function, typename... Arguments>
	struct TIsInvocableWithTupleArgumentsHelper : std::false_type { };

	template<typename Function, template<typename...> class Tuple, typename... Arguments>
	struct TIsInvocableWithTupleArgumentsHelper<Function, Tuple<Arguments...>> : std::is_invocable<Function, Arguments...> { };

	template<typename Function, template<typename...> class Tuple, typename... Arguments>
	struct TIsInvocableWithTupleArgumentsHelper<Function, const Tuple<Arguments...>> : std::is_invocable<Function, Arguments...> { };

	/** Checks if the specified @p Function can be invoked using std::apply(Function, std::tuple<Arguments...>). */
	template<typename Function, typename... Arguments>
	static constexpr bool TIsInvocableWithTupleArguments = TIsInvocableWithTupleArgumentsHelper<Function, Arguments...>::value;

	/** @} */
} // namespace b3d::ecs
