//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsMaterialEx.h"
#include "CoreObject/BsRenderThread.h"

using namespace bs;
void MaterialEx::SetTexture(const HMaterial& thisPtr, const String& name, const ResourceHandle<Texture>& value, u32 mipLevel, u32 numMipLevels, u32 arraySlice, u32 numArraySlices)
{
	thisPtr->SetTexture(name, value, TextureSurface(mipLevel, numMipLevels, arraySlice, numArraySlices));
}

HTexture MaterialEx::GetTexture(const HMaterial& thisPtr, const String& name)
{
	return thisPtr->GetTexture(name);
}

void MaterialEx::SetSpriteTexture(const HMaterial& thisPtr, const String& name, const HSpriteTexture& value)
{
	thisPtr->SetSpriteTexture(name, value);
}

HSpriteTexture MaterialEx::GetSpriteTexture(const HMaterial& thisPtr, const String& name)
{
	return thisPtr->GetSpriteTexture(name);
}
