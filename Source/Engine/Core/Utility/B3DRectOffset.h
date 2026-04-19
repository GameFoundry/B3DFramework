//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
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
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for b3d::RectOffset. */
template<>
struct hash<b3d::RectOffset>
{
	size_t operator()(const b3d::RectOffset& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.Left);
		b3d::B3DCombineHash(hash, value.Right);
		b3d::B3DCombineHash(hash, value.Top);
		b3d::B3DCombineHash(hash, value.Bottom);

		return hash;
	}
};
} // namespace std

/** @endcond */
