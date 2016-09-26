//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSpriteMaterials.h"
#include "BsBuiltinResources.h"
#include "BsGpuBuffer.h"
#include "BsMaterial.h"

namespace BansheeEngine
{
	SpriteImageTransparentMaterial::SpriteImageTransparentMaterial()
		:SpriteMaterial(0, BuiltinResources::instance().createSpriteImageMaterial())
	{ }

	SpriteImageOpaqueMaterial::SpriteImageOpaqueMaterial()
		: SpriteMaterial(1, BuiltinResources::instance().createSpriteNonAlphaImageMaterial())
	{ }

	SpriteTextMaterial::SpriteTextMaterial()
		: SpriteMaterial(2, BuiltinResources::instance().createSpriteTextMaterial())
	{ }

	SpriteLineMaterial::SpriteLineMaterial()
		: SpriteMaterial(3, BuiltinResources::instance().createSpriteLineMaterial())
	{ }
}