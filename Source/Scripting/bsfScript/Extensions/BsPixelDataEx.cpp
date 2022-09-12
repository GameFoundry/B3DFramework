//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsPixelDataEx.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	SPtr<PixelData> PixelDataEx::Create(const PixelVolume& volume, PixelFormat format)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(volume, format);
		pixelData->AllocateInternalBuffer();

		return pixelData;
	}

	SPtr<PixelData> PixelDataEx::Create(UINT32 width, UINT32 height, UINT32 depth, PixelFormat format)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(width, height, depth, format);
		pixelData->AllocateInternalBuffer();

		return pixelData;
	}

	Color PixelDataEx::GetPixel(const SPtr<PixelData>& thisPtr, int x, int y, int z)
	{
		if (!checkIsLocked(thisPtr))
			return thisPtr->GetColorAt(x, y, z);
		else
			return Color();
	}

	void PixelDataEx::SetPixel(const SPtr<PixelData>& thisPtr, const Color& value, int x, int y, int z)
	{
		if (!checkIsLocked(thisPtr))
			thisPtr->SetColorAt(value, x, y, z);
	}

	Vector<Color> PixelDataEx::GetPixels(const SPtr<PixelData>& thisPtr)
	{
		if (!checkIsLocked(thisPtr))
			return Vector<Color>();

		return thisPtr->GetColors();
	}

	void PixelDataEx::SetPixels(const SPtr<PixelData>& thisPtr, const Vector<Color>& value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		thisPtr->SetColors(value);
	}

	Vector<char> PixelDataEx::GetRawPixels(const SPtr<PixelData>& thisPtr)
	{
		if (!checkIsLocked(thisPtr))
			return Vector<char>();

		Vector<char> Output(thisPtr->GetSize());
		memcpy(output.Data(), thisPtr->GetData(), thisPtr->getSize());

		return output;
	}

	void PixelDataEx::SetRawPixels(const SPtr<PixelData>& thisPtr, const Vector<char>& value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		UINT32 arrayLen = (UINT32)value.size();
		if (thisPtr->GetSize() != arrayLen)
		{
			BS_LOG(Warning, Texture, "Unable to set colors, invalid array size.");
			return;
		}

		UINT8* data = thisPtr->GetData();
		memcpy(data, value.Data(), thisPtr->GetSize());
	}

	bool PixelDataEx::CheckIsLocked(const SPtr<PixelData>& thisPtr)
	{
		if (thisPtr->IsLocked())
		{
			BS_LOG(Warning, Texture, "Attempting to access a locked pixel data buffer.");
			return true;
		}

		return false;
	}
}
