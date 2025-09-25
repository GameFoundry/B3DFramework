//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DSpriteMaterials.h"
#include "Resources/B3DBuiltinResources.h"
#include "Material/B3DMaterial.h"

using namespace b3d;

u32 GetMaterialId(SpriteMaterialTransparency transparency, bool animated)
{
	switch(transparency)
	{
	default:
	case SpriteMaterialTransparency::Opaque: return animated ? 3 : 0;
	case SpriteMaterialTransparency::Alpha: return animated ? 4 : 1;
	case SpriteMaterialTransparency::Premultiplied: return animated ? 5 : 2;
	}
}

ShaderVariationParameters GetMaterialVariation(SpriteMaterialTransparency transparency, bool animated)
{
	return ShaderVariationParameters(TInlineArray<ShaderVariationParameter, 4>({ ShaderVariationParameter("TRANSPARENCY", (i32)transparency),
																	ShaderVariationParameter("ANIMATED", animated) }));
}

SpriteImageMaterial::SpriteImageMaterial(SpriteMaterialTransparency transparency, bool animated)
	: SpriteMaterial(
		  GetMaterialId(transparency, animated),
		  BuiltinResources::Instance().CreateSpriteImageMaterial(),
		  GetMaterialVariation(transparency, animated),
		  !animated)
{}

SpriteTextMaterial::SpriteTextMaterial()
	: SpriteMaterial(6, BuiltinResources::Instance().CreateSpriteTextMaterial())
{}

SpriteLineMaterial::SpriteLineMaterial()
	: SpriteMaterial(7, BuiltinResources::Instance().CreateSpriteLineMaterial())
{}
