//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"

namespace b3d::ecs
{
	/** @addtogroup ECS-Internal
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/** Provides information about a backing type for an Entity. */
	template<typename>
	struct TEntityTypeTraits;

	template<>
	struct TEntityTypeTraits<u32>
	{
		using StorageType = u32;
		using IdentifierType = u32;
		using VersionType = u16;

		static constexpr u32 kIdentifierBitCount = 20;
		static constexpr u32 kVersionBitCount  = 12;
	};

	template<>
	struct TEntityTypeTraits<u64>
	{
		using StorageType = u64;
		using IdentifierType = u64;
		using VersionType = u32;

		static constexpr u32 kIdentifierBitCount = 32;
		static constexpr u32 kVersionBitCount  = 32;
	};

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	/**
	 * Represents a single entity in the ECS system. Each entity can have zero or multiple components associated with it. Each entity has a unique
	 * identifier and a version field. Version field gets incremented if an entity is destroyed and then its identifier gets re-used.
	 *
	 * @tparam	Type	Backing type for the entity, normally 32-bit or 64-bit unsigned integer. Use a larger integer if you require larger number of entities.
	 */
	template<typename Type>
	struct TEntity
	{
		using Traits = TEntityTypeTraits<Type>;
		using StorageType = typename Traits::StorageType;
		using IdentifierType = typename Traits::IdentifierType;
		using VersionType = typename Traits::VersionType;

		/** Masks the bits for the identifier portion of the entity. Identifier is guaranteed to start at the lowest bit. */
		static constexpr StorageType kIdentifierMask = (1 << Traits::kIdentifierBitCount) - 1;

		/** Masks the bits for the version portion of the entity. Version is guaranteed to start after the highest identifier bit. */
		static constexpr StorageType kVersionMask = (1 << Traits::kVersionBitCount) - 1;

		constexpr TEntity() = default;
		constexpr TEntity(IdentifierType identifier, VersionType version)
		{
			if constexpr(kVersionMask == 0u)
				IdentifierAndVersion = (Type)(identifier & kIdentifierMask);
			else
				IdentifierAndVersion = ((Type)(identifier & kIdentifierMask)) | (((Type)(version & kVersionMask)) << Traits::kIdentifierBitCount);
		}

		bool operator==(TEntity other) const
		{
			return IdentifierAndVersion == other.IdentifierAndVersion;
		}

		bool operator!=(TEntity other) const
		{
			return IdentifierAndVersion != other.IdentifierAndVersion;
		}

		bool operator<(TEntity other) const
		{
			return Identifier < other.Identifier;
		}

		/** Returns the unique identifier of the entity. */
		constexpr IdentifierType GetIdentifier() const
		{
			return (IdentifierType)(IdentifierAndVersion) & kIdentifierMask;
		}

		/** Returns the current version of the entity. This will be incremented each time an entity is destroyed and its identifier gets re-used. */
		constexpr VersionType GetVersion() const
		{
			if constexpr(kVersionMask == 0u)
				return VersionType{};

			return (typename Traits::VersionType)(IdentifierAndVersion >> Traits::kIdentifierBitCount) & kVersionMask;
		}

		/** Returns the entity with the same identifier, but with the version incremented by one. */
		constexpr TEntity GetAsNextVersion() const
		{
			return TEntity(GetIdentifier(), GetVersion() + 1);
		}

		/** Returns a hash value for this entity. */
		u64 GenerateHash() const
		{
			return (u64)IdentifierAndVersion;
		}
		
		union
		{
			struct
			{
				StorageType Identifier : Traits::kIdentifierBitCount;
				StorageType Version : Traits::kVersionBitCount;
			};

			StorageType IdentifierAndVersion;
		};
	};

#define B3D_ECS_64BIT_ENTITIES 0
#define B3D_ECS_SPARSE_SET_PAGE_SIZE 4096

#if B3D_ECS_64BIT_ENTITIES
	using Entity = TEntity<u64>;
#else
	using Entity = TEntity<u32>;
#endif

	/**
	 * Helper structure that represents a null Entity. Null entity is any entity with the highest allowed identifier value.
	 * Version is ignored when comparing against a null entity.
	 */
	struct NullEntity
	{
		template<typename Type>
		constexpr operator TEntity<Type>() const
		{
			return TEntity<Type>(TEntity<Type>::kIdentifierMask, TEntity<Type>::kVersionMask);
		}

		constexpr bool operator==(NullEntity) const
		{
			return true;
		}

		constexpr bool operator!=(NullEntity) const
		{
			return false;
		}

		template<typename Type>
		constexpr bool operator==(TEntity<Type> value) const
		{
			return value.GetIdentifier() == ((TEntity<Type>)*this).GetIdentifier();
		}

		template<typename Type>
		constexpr bool operator!=(TEntity<Type> value) const
		{
			return !(value == *this);
		}
	};

	template<typename Type>
	constexpr bool operator==(TEntity<Type> lhs, NullEntity rhs)
	{
		return rhs.operator==(lhs);
	}

	template<typename Type>
	constexpr bool operator!=(TEntity<Type> lhs, NullEntity rhs)
	{
		return !(rhs == lhs);
	}

	/**
	 * Helper structure that represents an invalid (usually deleted) Entity. Invalid entity is any entity with the highest allowed version value.
	 * Identifier is ignored when comparing against an invalid entity.
	 */
	struct InvalidEntity
	{
		template<typename Type>
		constexpr operator TEntity<Type>() const
		{
			return TEntity<Type>(TEntity<Type>::kIdentifierMask, TEntity<Type>::kVersionMask);
		}

		constexpr bool operator==(InvalidEntity) const
		{
			return true;
		}

		constexpr bool operator!=(InvalidEntity) const
		{
			return false;
		}

		template<typename Type>
		constexpr bool operator==(TEntity<Type> value) const
		{
			if constexpr(TEntity<Type>::kVersionMask == 0u)
				return false;

			return value.GetVersion() == ((TEntity<Type>)*this).GetVersion();
		}

		template<typename Type>
		constexpr bool operator!=(Type value) const
		{
			return !(value == *this);
		}
	};

	template<typename Type>
	constexpr bool operator==(Type lhs, InvalidEntity rhs)
	{
		return rhs.operator==(lhs);
	}

	template<typename Type>
	constexpr bool operator!=(Type lhs, InvalidEntity rhs)
	{
		return !(rhs == lhs);
	}

	/** @copydoc NullEntity */
	inline constexpr NullEntity kNullEntity;

	/** @copydoc InvalidEntity */
	inline constexpr InvalidEntity kInvalidEntity;

	/** @} */
} // namespace b3d::ecs
