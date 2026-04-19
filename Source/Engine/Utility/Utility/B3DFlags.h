//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Wrapper around an enum that allows simple use of bitwise logic operations. */
	template <typename Enum, typename Storage = u32>
	class Flags
	{
	public:
		using InternalType = Storage;

		constexpr Flags() = default;

		constexpr Flags(Enum value)
		{
			mBits = static_cast<Storage>(value);
		}

		constexpr Flags(const Flags<Enum, Storage>& value)
		{
			mBits = value.mBits;
		}

		constexpr explicit Flags(Storage bits)
		{
			mBits = bits;
		}

		/** Checks whether all of the provided bits are set */
		bool IsSet(Enum value) const
		{
			return (mBits & static_cast<Storage>(value)) == static_cast<Storage>(value);
		}

		/** Checks whether any of the provided bits are set */
		bool IsSetAny(Enum value) const
		{
			return (mBits & static_cast<Storage>(value)) != 0;
		}

		/** Checks whether any of the provided bits are set */
		bool IsSetAny(const Flags<Enum, Storage>& value) const
		{
			return (mBits & value.mBits) != 0;
		}

		/** Checks whether all of the provided bits are set */
		bool IsSetAll(const Flags<Enum, Storage>& value) const
		{
			return (mBits & value.mBits) == value.mBits;
		}

		/** Activates all of the provided bits. */
		Flags<Enum, Storage>& Set(Enum value)
		{
			mBits |= static_cast<Storage>(value);

			return *this;
		}

		/** Deactivates all of the provided bits. */
		void Unset(Enum value)
		{
			mBits &= ~static_cast<Storage>(value);
		}

		bool operator==(Enum rhs) const
		{
			return mBits == static_cast<Storage>(rhs);
		}

		bool operator==(const Flags<Enum, Storage>& rhs) const
		{
			return mBits == rhs.mBits;
		}

		bool operator==(bool rhs) const
		{
			return ((bool)*this) == rhs;
		}

		bool operator!=(Enum rhs) const
		{
			return mBits != static_cast<Storage>(rhs);
		}

		bool operator!=(const Flags<Enum, Storage>& rhs) const
		{
			return mBits != rhs.mBits;
		}

		Flags<Enum, Storage>& operator=(Enum rhs)
		{
			mBits = static_cast<Storage>(rhs);

			return *this;
		}

		Flags<Enum, Storage>& operator=(const Flags<Enum, Storage>& rhs)
		{
			mBits = rhs.mBits;

			return *this;
		}

		Flags<Enum, Storage>& operator|=(Enum rhs)
		{
			mBits |= static_cast<Storage>(rhs);

			return *this;
		}

		Flags<Enum, Storage>& operator|=(const Flags<Enum, Storage>& rhs)
		{
			mBits |= rhs.mBits;

			return *this;
		}

		Flags<Enum, Storage> operator|(Enum rhs) const
		{
			Flags<Enum, Storage> out(*this);
			out |= rhs;

			return out;
		}

		Flags<Enum, Storage> operator|(const Flags<Enum, Storage>& rhs) const
		{
			Flags<Enum, Storage> out(*this);
			out |= rhs;

			return out;
		}

		Flags<Enum, Storage>& operator&=(Enum rhs)
		{
			mBits &= static_cast<Storage>(rhs);

			return *this;
		}

		Flags<Enum, Storage>& operator&=(const Flags<Enum, Storage>& rhs)
		{
			mBits &= rhs.mBits;

			return *this;
		}

		Flags<Enum, Storage> operator&(Enum rhs) const
		{
			Flags<Enum, Storage> out = *this;
			out.mBits &= static_cast<Storage>(rhs);

			return out;
		}

		Flags<Enum, Storage> operator&(const Flags<Enum, Storage>& rhs) const
		{
			Flags<Enum, Storage> out = *this;
			out.mBits &= rhs.mBits;

			return out;
		}

		Flags<Enum, Storage>& operator^=(Enum rhs)
		{
			mBits ^= static_cast<Storage>(rhs);

			return *this;
		}

		Flags<Enum, Storage>& operator^=(const Flags<Enum, Storage>& rhs)
		{
			mBits ^= rhs.mBits;

			return *this;
		}

		Flags<Enum, Storage> operator^(Enum rhs) const
		{
			Flags<Enum, Storage> out = *this;
			out.mBits ^= static_cast<Storage>(rhs);

			return out;
		}

		Flags<Enum, Storage> operator^(const Flags<Enum, Storage>& rhs) const
		{
			Flags<Enum, Storage> out = *this;
			out.mBits ^= rhs.mBits;

			return out;
		}

		Flags<Enum, Storage> operator~() const
		{
			Flags<Enum, Storage> out;
			out.mBits = (Storage)~mBits;

			return out;
		}

		operator bool() const
		{
			return mBits ? true : false;
		}

		operator u8() const
		{
			return static_cast<u8>(mBits);
		}

		operator u16() const
		{
			return static_cast<u16>(mBits);
		}

		operator u32() const
		{
			return static_cast<u32>(mBits);
		}

		friend Flags<Enum, Storage> operator&(Enum a, Flags<Enum, Storage>& b)
		{
			Flags<Enum, Storage> out;
			out.mBits = a & b.mBits;
			return out;
		}

	private:
		InternalType mBits{ 0 };
	};

/** Defines global operators for a Flags<Enum, Storage> implementation. */
#define B3D_FLAGS_OPERATORS(Enum) B3D_FLAGS_OPERATORS_EXT(Enum, u32)

/** Defines global operators for a Flags<Enum, Storage> implementation. */
#define B3D_FLAGS_OPERATORS_EXT(Enum, Storage)             \
	inline Flags<Enum, Storage> operator|(Enum a, Enum b) \
	{                                                     \
		Flags<Enum, Storage> r(a);                        \
		r |= b;                                           \
		return r;                                         \
	}                                                     \
	inline Flags<Enum, Storage> operator&(Enum a, Enum b) \
	{                                                     \
		Flags<Enum, Storage> r(a);                        \
		r &= b;                                           \
		return r;                                         \
	}                                                     \
	inline Flags<Enum, Storage> operator~(Enum a)         \
	{                                                     \
		return ~Flags<Enum, Storage>(a);                  \
	}

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for Flags<Enum, Storage>. */
	template <class Enum, class Storage>
	struct hash<b3d::Flags<Enum, Storage>>
	{
		size_t operator()(const b3d::Flags<Enum, Storage>& key) const
		{
			return (Storage)key;
		}
	};
} // namespace std

/** @endcond */
