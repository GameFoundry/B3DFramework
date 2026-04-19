//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DPixelDataEx.h"

using namespace b3d;
SPtr<PixelData> PixelDataEx::Create(const PixelVolume& volume, PixelFormat format)
{
	SPtr<PixelData> pixelData = B3DMakeShared<PixelData>(volume, format);
	pixelData->AllocateInternalBuffer();

	return pixelData;
}

SPtr<PixelData> PixelDataEx::Create(u32 width, u32 height, u32 depth, PixelFormat format)
{
	SPtr<PixelData> pixelData = B3DMakeShared<PixelData>(width, height, depth, format);
	pixelData->AllocateInternalBuffer();

	return pixelData;
}

Color PixelDataEx::GetPixel(const SPtr<PixelData>& thisPtr, int x, int y, int z)
{
	if(!CheckIsLocked(thisPtr))
		return thisPtr->GetColorAt(x, y, z);
	else
		return Color();
}

void PixelDataEx::SetPixel(const SPtr<PixelData>& thisPtr, const Color& value, int x, int y, int z)
{
	if(!CheckIsLocked(thisPtr))
		thisPtr->SetColorAt(value, x, y, z);
}

Vector<Color> PixelDataEx::GetPixels(const SPtr<PixelData>& thisPtr)
{
	if(!CheckIsLocked(thisPtr))
		return Vector<Color>();

	return thisPtr->GetColors();
}

void PixelDataEx::SetPixels(const SPtr<PixelData>& thisPtr, const Vector<Color>& value)
{
	if(!CheckIsLocked(thisPtr))
		return;

	thisPtr->SetColors(value);
}

Vector<char> PixelDataEx::GetRawPixels(const SPtr<PixelData>& thisPtr)
{
	if(!CheckIsLocked(thisPtr))
		return Vector<char>();

	Vector<char> output(thisPtr->GetSize());
	memcpy(output.data(), thisPtr->GetData(), thisPtr->GetSize());

	return output;
}

void PixelDataEx::SetRawPixels(const SPtr<PixelData>& thisPtr, const Vector<char>& value)
{
	if(!CheckIsLocked(thisPtr))
		return;

	u32 arrayLen = (u32)value.size();
	if(thisPtr->GetSize() != arrayLen)
	{
		B3D_LOG(Warning, LogTexture, "Unable to set colors, invalid array size.");
		return;
	}

	u8* data = thisPtr->GetData();
	memcpy(data, value.data(), thisPtr->GetSize());
}

bool PixelDataEx::CheckIsLocked(const SPtr<PixelData>& thisPtr)
{
	if(thisPtr->IsLocked())
	{
		B3D_LOG(Warning, LogTexture, "Attempting to access a locked pixel data buffer.");
		return true;
	}

	return false;
}
