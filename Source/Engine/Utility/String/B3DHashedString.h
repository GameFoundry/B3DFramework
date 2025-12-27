//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	namespace internal
	{
		/** Parameters used for Fowler-Noll-Vo hash. */
		template<typename = u64>
		struct TFNVHashParameters;

		template<>
		struct TFNVHashParameters<u32>
		{
			static constexpr u32 kOffset = 2166136261;
			static constexpr u32 kPrime = 16777619;
		};

		template<>
		struct TFNVHashParameters<u64>
		{
			static constexpr u64 kOffset = 14695981039346656037ull;
			static constexpr u64 kPrime = 1099511628211ull;
		};
	} // namespace internal

	/** @addtogroup General
	 *  @{
	 */

	/** References a compile time string and calculates its hash at compile time. */
	template<typename CharType, typename HashType = u64>
	class THashedString
	{
		using HashParameters = internal::TFNVHashParameters<HashType>;

		// Helper to allow for an explicit constructor, to avoid creating a hashed string directly from const char*
		struct ConstWrapper
		{
			constexpr ConstWrapper(const CharType* string) noexcept
				: Characters{string} {}

			const CharType* Characters;
		};
	public:
		constexpr THashedString() noexcept
			: THashedString{nullptr, 0u} {}

		constexpr THashedString(const CharType* string, u64 length)
			: mCharacters(string), mLength(length), mHash(CalculateHash({string, length}))
		{ }

		template<std::size_t N>
		constexpr THashedString(const CharType (&string)[N])
			: mCharacters((const CharType*)string), mLength(N), mHash(CalculateHash({(const CharType*)string, N}))
		{ }

		explicit constexpr THashedString(ConstWrapper wrapper) noexcept
			: mCharacters(wrapper.Characters), mLength(std::basic_string_view<CharType>(wrapper.Characters).size()), mHash(CalculateHash({wrapper.Characters}))
		{}

		/** Returns the length of the string. */
		constexpr u64 GetLength() const { return mLength; }

		/** Returns the characters of the compile time string. */
		constexpr const CharType* GetCharacters() const { return mCharacters; }

		/** Returns the calculated hash. */
		constexpr HashType GetHash() const { return mHash; }

		constexpr operator const CharType*() const { return GetCharacters(); }
		constexpr operator HashType() const { return GetHash(); }

		/** Calculates compile time hash of a compile time string. */
		static constexpr HashType CalculateHash(std::basic_string_view<CharType> view)
		{
			HashType hash = HashParameters::kOffset;

			for(auto&& character: view)
				hash = (hash ^ static_cast<HashType>(character)) * HashParameters::kPrime;

			return hash;
		}

		/** Calculates compile time hash of a compile time string. */
		static constexpr HashType CalculateHash(const CharType* string, const u64 length) { return CalculateHash({string, length}); }

		/** Calculates compile time hash of a compile time string. */
		template<std::size_t N>
		static constexpr HashType CalculateHash(const CharType (&string)[N]) { return CalculateHash({string}); }

	private:
		const CharType* mCharacters;
		u64 mLength;
		HashType mHash;
	};

	template<typename CharType>
	constexpr bool operator==(const THashedString<CharType>& lhs, const THashedString<CharType>& rhs)
	{
		return lhs.GetHash() == rhs.GetHash();
	}

	template<typename CharType>
	constexpr bool operator!=(const THashedString<CharType>& lhs, const THashedString<CharType>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename CharType>
	constexpr bool operator<(const THashedString<CharType>& lhs, const THashedString<CharType>& rhs)
	{
		return lhs.value() < rhs.value();
	}

	template<typename CharType>
	constexpr bool operator<=(const THashedString<CharType>& lhs, const THashedString<CharType>& rhs)
	{
		return !(rhs < lhs);
	}

	template<typename CharType>
	constexpr bool operator>(const THashedString<CharType>& lhs, const THashedString<CharType>& rhs)
	{
		return rhs < lhs;
	}

	template<typename CharType>
	constexpr bool operator>=(const THashedString<CharType>& lhs, const THashedString<CharType>& rhs)
	{
		return !(lhs < rhs);
	}

	using HashedString = THashedString<char>;

	/** @} */
} // namespace b3d::ecs
