//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsMaterialEx.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	void MaterialEx::setTexture(const HMaterial& thisPtr, const String& name, const ResourceHandle<Texture>& value, 
		UINT32 mipLevel, UINT32 numMipLevels, UINT32 arraySlice, UINT32 numArraySlices)
	{
		thisPtr->setTexture(name, value, TextureSurface(mipLevel, numMipLevels, arraySlice, numArraySlices));
	}

	HTexture MaterialEx::getTexture(const HMaterial& thisPtr, const String& name)
	{
		return thisPtr->getTexture(name);
	}

	void MaterialEx::setSpriteTexture(const HMaterial& thisPtr, const String& name, const HSpriteTexture& value)
	{
		thisPtr->setSpriteTexture(name, value);
	}

	HSpriteTexture MaterialEx::getSpriteTexture(const HMaterial& thisPtr, const String& name)
	{
		return thisPtr->getSpriteTexture(name);
	}
}
