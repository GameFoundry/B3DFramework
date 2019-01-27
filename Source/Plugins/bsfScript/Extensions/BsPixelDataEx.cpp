//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsPixelDataEx.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	SPtr<PixelData> PixelDataEx::create(const PixelVolume& volume, PixelFormat format)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(volume, format);
		pixelData->allocateInternalBuffer();

		return pixelData;
	}

	SPtr<PixelData> PixelDataEx::create(UINT32 width, UINT32 height, UINT32 depth, PixelFormat format)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(width, height, depth, format);
		pixelData->allocateInternalBuffer();

		return pixelData;
	}

	Color PixelDataEx::getPixel(const SPtr<PixelData>& thisPtr, int x, int y, int z)
	{
		if (!checkIsLocked(thisPtr))
			return thisPtr->getColorAt(x, y, z);
		else
			return Color();
	}

	void PixelDataEx::setPixel(const SPtr<PixelData>& thisPtr, const Color& value, int x, int y, int z)
	{
		if (!checkIsLocked(thisPtr))
			thisPtr->setColorAt(value, x, y, z);
	}

	Vector<Color> PixelDataEx::getPixels(const SPtr<PixelData>& thisPtr)
	{
		if (!checkIsLocked(thisPtr))
			return Vector<Color>();

		return thisPtr->getColors();
	}

	void PixelDataEx::setPixels(const SPtr<PixelData>& thisPtr, const Vector<Color>& value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		thisPtr->setColors(value);
	}

	Vector<char> PixelDataEx::getRawPixels(const SPtr<PixelData>& thisPtr)
	{
		if (!checkIsLocked(thisPtr))
			return Vector<char>();

		Vector<char> output(thisPtr->getSize());
		memcpy(output.data(), thisPtr->getData(), thisPtr->getSize());

		return output;
	}

	void PixelDataEx::setRawPixels(const SPtr<PixelData>& thisPtr, const Vector<char>& value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		UINT32 arrayLen = (UINT32)value.size();
		if (thisPtr->getSize() != arrayLen)
		{
			LOGERR("Unable to set colors, invalid array size.")
				return;
		}

		UINT8* data = thisPtr->getData();
		memcpy(data, value.data(), thisPtr->getSize());
	}

	bool PixelDataEx::checkIsLocked(const SPtr<PixelData>& thisPtr)
	{
		if (thisPtr->isLocked())
		{
			LOGWRN("Attempting to access a locked pixel data buffer.");
			return true;
		}

		return false;
	}
}