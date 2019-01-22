//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsSpriteMaterials.h"
#include "Resources/BsBuiltinResources.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Material/BsMaterial.h"

namespace bs
{
	UINT32 getMaterialId(bool transparent, bool animated)
	{
		if(!animated)
			return transparent ? 0 : 1;

		return transparent ? 2 : 3;
	}

	ShaderVariation getMaterialVariation(bool transparent, bool animated)
	{
		return ShaderVariation(SmallVector<ShaderVariation::Param, 4>({
			ShaderVariation::Param("TRANSPARENT", transparent),
			ShaderVariation::Param("ANIMATED", animated)
			}));
	}

	SpriteImageMaterial::SpriteImageMaterial(bool transparent, bool animated)
		:SpriteMaterial(
			getMaterialId(transparent, animated), 
			BuiltinResources::instance().createSpriteImageMaterial(),
			getMaterialVariation(transparent, animated), 
			!animated)
	{
		
	}

	SpriteTextMaterial::SpriteTextMaterial()
		: SpriteMaterial(4, BuiltinResources::instance().createSpriteTextMaterial())
	{ }

	SpriteLineMaterial::SpriteLineMaterial()
		: SpriteMaterial(5, BuiltinResources::instance().createSpriteLineMaterial())
	{ }
}