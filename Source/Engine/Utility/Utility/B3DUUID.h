//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/B3DPlatformDefines.h"
#include "String/B3DString.h"
#include "Prerequisites/B3DTypes.h"
#include "Utility/B3DUtil.h"

namespace b3d
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/** Represents a universally unique identifier. */
	struct B3D_EXPORT UUID
	{
		/** Initializes an empty UUID. */
		constexpr UUID() = default;

		/** Initializes an UUID using framework's UUID representation. */
		constexpr UUID(u32 data1, u32 data2, u32 data3, u32 data4)
			: mData{ data1, data2, data3, data4 }
		{}

		/** Initializes an UUID using its string representation. */
		explicit UUID(const String& uuid);

		constexpr bool operator==(const UUID& rhs) const
		{
			return mData[0] == rhs.mData[0] && mData[1] == rhs.mData[1] && mData[2] == rhs.mData[2] && mData[3] == rhs.mData[3];
		}

		constexpr bool operator!=(const UUID& rhs) const
		{
			return !(*this == rhs);
		}

		constexpr bool operator<(const UUID& rhs) const
		{
			for(u32 elementIndex = 0; elementIndex < 4; elementIndex++)
			{
				if(mData[elementIndex] < rhs.mData[elementIndex])
					return true;
				else if(mData[elementIndex] > rhs.mData[elementIndex])
					return false;

				// Move to next element if equal
			}

			// They're equal
			return false;
		}

		/** Checks has the UUID been initialized to a valid value. */
		constexpr bool Empty() const
		{
			return mData[0] == 0 && mData[1] == 0 && mData[2] == 0 && mData[3] == 0;
		}

		/** Converts the UUID into its string representation. */
		String ToString() const;

		static const UUID kEmpty;

	private:
		friend struct std::hash<UUID>;

		u32 mData[4] = { 0, 0, 0, 0 };
	};

	/**
	 * Utility class for generating universally unique identifiers.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT UUIDGenerator
	{
	public:
		/**	Generate a new random universally unique identifier. */
		static UUID GenerateRandom();
	};

	/** @} */

} // namespace b3d

/** @cond STDLIB */
/** @addtogroup Utility
 *  @{
 */

namespace std
{
	/**	Hash value generator for UUID. */
	template <>
	struct hash<b3d::UUID>
	{
		size_t operator()(const b3d::UUID& value) const
		{
			size_t hash = 0;
			b3d::B3DCombineHash(hash, value.mData[0]);
			b3d::B3DCombineHash(hash, value.mData[1]);
			b3d::B3DCombineHash(hash, value.mData[2]);
			b3d::B3DCombineHash(hash, value.mData[3]);

			return hash;
		}
	};
} // namespace std

/** @} */
/** @endcond */
