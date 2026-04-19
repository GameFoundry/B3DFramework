//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DMaterialEx.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;
void MaterialEx::SetTexture(const HMaterial& thisPtr, const String& name, const TResourceHandle<Texture>& value, u32 mipLevel, u32 numMipLevels, u32 arraySlice, u32 numArraySlices)
{
	thisPtr->SetTexture(name, value, TextureSurface(mipLevel, numMipLevels, arraySlice, numArraySlices));
}

HTexture MaterialEx::GetTexture(const HMaterial& thisPtr, const String& name)
{
	return thisPtr->GetTexture(name);
}

void MaterialEx::SetSpriteImage(const HMaterial& thisPtr, const String& name, const HSpriteImage& value)
{
	thisPtr->SetSpriteImage(name, value);
}

HSpriteImage MaterialEx::GetSpriteImage(const HMaterial& thisPtr, const String& name)
{
	return thisPtr->GetSpriteImage(name);
}
