//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Image/BsColor.h"
#include "Image/BsPixelData.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for PixelData, for adding additional functionality for the script version of PixelData. */
	class BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData)) PixelDataEx
	{
	public:
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(PixelData))
		static SPtr<PixelData> Create(const PixelVolume& volume, PixelFormat format = PF_BGRA8);

		BS_SCRIPT_EXPORT(ExtensionConstructorForType(PixelData))
		static SPtr<PixelData> Create(u32 width, u32 height, u32 depth = 1, PixelFormat pixelFormat = PF_BGRA8);

		/**
		 * Returns a pixel at the specified location in the buffer.
		 *
		 * @param[in] x		X coordinate of the pixel.
		 * @param[in] y		Y coordinate of the pixel.
		 * @param[in] z		Z coordinate of the pixel.
		 * @return			Value of the pixel, or undefined value if coordinates are out of range.
		 */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData),n:GetPixel)
		static Color GetPixel(const SPtr<PixelData>& thisPtr, int x, int y, int z = 0);
		
		/**
		 * Sets a pixel at the specified location in the buffer.
		 *
		 * @param[in] value		Color of the pixel to set.
		 * @param[in] x			X coordinate of the pixel.
		 * @param[in] y			Y coordinate of the pixel.
		 * @param[in] z			Z coordinate of the pixel.
		 */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData),n:SetPixel)
		static void SetPixel(const SPtr<PixelData>& thisPtr, const Color& value, int x, int y, int z = 0);
		
		/**
		 * Returns values of all pixels.
		 *
		 * @return	All pixels in the buffer ordered consecutively. Pixels are stored as a succession of "depth" slices,
		 *			each containing "height" rows of "width" pixels.
		 */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData),n:GetPixels)
		static Vector<Color> GetPixels(const SPtr<PixelData>& thisPtr);
		
		/**
		 * Sets all pixels in the buffer.Caller must ensure that number of pixels match the extends of the buffer.
		 *
		 * @param value	All pixels in the buffer ordered consecutively. Pixels are stored as a succession of "depth" slices,
		 *				each containing "height" rows of "width" pixels.
		 */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData),n:SetPixels)
		static void SetPixels(const SPtr<PixelData>& thisPtr, const Vector<Color>& value);
		
		/**
		 * Returns all pixels in the buffer as raw bytes.
		 *
		 * @return	Raw pixel bytes. It is up to the caller to interpret the pixel format and account for potential
		 *			row and slice pitch values.
		 */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData),n:GetRawPixels)
		static Vector<char> GetRawPixels(const SPtr<PixelData>& thisPtr);
		
		/**
		 * Sets all pixels in the buffer as raw bytes.
		 *
		 * @param[in] value		Raw pixel bytes. It is up to the caller to interpret the pixel format and account for
		 *						potential row and slice pitch values.
		 */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PixelData),n:SetRawPixels)
		static void SetRawPixels(const SPtr<PixelData>& thisPtr, const Vector<char>& value);

		static bool CheckIsLocked(const SPtr<PixelData>& thisPtr);
	};

	/** @endcond */
	/** @} */
}
