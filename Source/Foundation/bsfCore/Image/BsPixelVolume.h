//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Utility-Core
	 *  @{
	 */

	/**	Represents a 3D region of pixels used for referencing pixel data. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(ExportAsStruct(true),n:PixelVolume) PixelVolume
	{
		u32 Left = 0;
		u32 Top = 0;
		u32 Right = 1;
		u32 Bottom = 1;
		u32 Front = 0;
		u32 Back = 1;

		PixelVolume() = default;

		PixelVolume(u32 left, u32 top, u32 right, u32 bottom):
			Left(left), Top(top), Right(right), Bottom(bottom), Front(0), Back(1)
		{
			assert(right >= left && bottom >= top && Back >= Front);
		}

		PixelVolume(u32 left, u32 top, u32 front, u32 right, u32 bottom, u32 back):
			Left(left), Top(top), Right(right), Bottom(bottom), Front(front), Back(back)
		{
			assert(right >= left && bottom >= top && back >= front);
		}
			
		/**	Return true if the other box is a part of this one. */
		bool Contains(const PixelVolume &volume) const
		{
			return (volume.Left >= Left && volume.Top >= Top && volume.Front >= Front &&
				volume.Right <= Right && volume.Bottom <= Bottom && volume.Back <= Back);
		}
			
		u32 GetWidth() const { return Right-Left; }
		u32 GetHeight() const { return Bottom-Top; }
		u32 GetDepth() const { return Back-Front; }
	};

	/** @} */
}
