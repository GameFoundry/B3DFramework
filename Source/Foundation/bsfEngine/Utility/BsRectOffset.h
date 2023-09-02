//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/** Rectangle represented in the form of offsets from some parent rectangle. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Math)) RectOffset
	{
		RectOffset() = default;

		RectOffset(i32 left, i32 right, i32 top, i32 bottom)
			: Left(left), Right(right), Top(top), Bottom(bottom)
		{}

		bool operator==(const RectOffset& rhs) const
		{
			return Left == rhs.Left && Right == rhs.Right && Top == rhs.Top && Bottom == rhs.Bottom;
		}

		bool operator!=(const RectOffset& rhs) const
		{
			return !operator==(rhs);
		}

		i32 Left = 0;
		i32 Right = 0;
		i32 Top = 0;
		i32 Bottom = 0;
	};

	/** @} */
} // namespace bs

/** @cond STDLIB */

namespace std
{
/** Hash value generator for bs::RectOffset. */
template<>
struct hash<bs::RectOffset>
{
	size_t operator()(const bs::RectOffset& value) const
	{
		size_t hash = 0;
		bs::B3DCombineHash(hash, value.Left);
		bs::B3DCombineHash(hash, value.Right);
		bs::B3DCombineHash(hash, value.Top);
		bs::B3DCombineHash(hash, value.Bottom);

		return hash;
	}
};
} // namespace std

/** @endcond */
