//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsColor.h"
#include "BsPixelData.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Extension class for PixelData, for adding additional functionality for the script version of PixelData. */
	class BS_SCRIPT_EXPORT(e:PixelData) PixelDataEx
	{
	public:
		BS_SCRIPT_EXPORT(ec:PixelData)
		static SPtr<PixelData> create(const PixelVolume& volume, PixelFormat format = PF_B8G8R8A8);

		BS_SCRIPT_EXPORT(ec:PixelData)
		static SPtr<PixelData> create(UINT32 width, UINT32 height, UINT32 depth = 1, PixelFormat pixelFormat = PF_B8G8R8A8);

		/**
		 * Returns a pixel at the specified location in the buffer.
		 * 
		 * @param[in] x		X coordinate of the pixel.
		 * @param[in] y		Y coordinate of the pixel.
		 * @param[in] z		Z coordinate of the pixel.
		 * @return			Value of the pixel, or undefined value if coordinates are out of range.
		 */
		BS_SCRIPT_EXPORT(e:PixelData,n:GetPixel)
		static Color getPixel(const SPtr<PixelData>& thisPtr, int x, int y, int z = 0);
		
		/**
		 * Sets a pixel at the specified location in the buffer.
		 * 
		 * @param[in] value		Color of the pixel to set.
		 * @param[in] x			X coordinate of the pixel.
		 * @param[in] y			Y coordinate of the pixel.
		 * @param[in] z			Z coordinate of the pixel.
		 */
		BS_SCRIPT_EXPORT(e:PixelData,n:SetPixel)
		static void setPixel(const SPtr<PixelData>& thisPtr, const Color& value, int x, int y, int z = 0);
		
		/**
		 * Returns values of all pixels. 
		 *
		 * @return	All pixels in the buffer ordered consecutively. Pixels are stored as a succession of "depth" slices, 
		 *			each containing "height" rows of "width" pixels.
		 */
		BS_SCRIPT_EXPORT(e:PixelData,n:GetPixels)
		static Vector<Color> getPixels(const SPtr<PixelData>& thisPtr);
		
		/**
		 * Sets all pixels in the buffer.Caller must ensure that number of pixels match the extends of the buffer.
		 *
		 * @param value	All pixels in the buffer ordered consecutively. Pixels are stored as a succession of "depth" slices, 
		 *				each containing "height" rows of "width" pixels.
		 */
		BS_SCRIPT_EXPORT(e:PixelData,n:SetPixels)
		static void setPixels(const SPtr<PixelData>& thisPtr, const Vector<Color>& value);
		
		/**
		 * Returns all pixels in the buffer as raw bytes.
		 *
		 * @return	Raw pixel bytes. It is up to the caller to interpret the pixel format and account for potential 
         *			row and slice pitch values.
		 */
		BS_SCRIPT_EXPORT(e:PixelData,n:GetRawPixels)
		static Vector<char> getRawPixels(const SPtr<PixelData>& thisPtr);
		
		/**
		 * Sets all pixels in the buffer as raw bytes.
		 *
		 * @param[in] value		Raw pixel bytes. It is up to the caller to interpret the pixel format and account for 
		 *						potential row and slice pitch values.
		 */
		BS_SCRIPT_EXPORT(e:PixelData,n:SetRawPixels)
		static void setRawPixels(const SPtr<PixelData>& thisPtr, const Vector<char>& value);

		static bool checkIsLocked(const SPtr<PixelData>& thisPtr);
	};

	/** @} */
}