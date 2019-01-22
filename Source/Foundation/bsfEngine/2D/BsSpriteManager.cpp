//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsSpriteManager.h"
#include "2D/BsSpriteMaterials.h"

namespace bs
{
	SpriteManager::SpriteManager()
	{
		SpriteMaterial* imageTransparentMat = registerMaterial<SpriteImageMaterial>(true, false);
		SpriteMaterial* imageOpaqueMat = registerMaterial<SpriteImageMaterial>(false, false);
		SpriteMaterial* imageTransparentAnimMat = registerMaterial<SpriteImageMaterial>(true, true);
		SpriteMaterial* imageOpaqueAnimMat = registerMaterial<SpriteImageMaterial>(false, true);
		SpriteMaterial* textMat = registerMaterial<SpriteTextMaterial>();
		SpriteMaterial* lineMat = registerMaterial<SpriteLineMaterial>();

		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparent] = imageTransparentMat->getId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaque] = imageOpaqueMat->getId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparentAnimated] = imageTransparentAnimMat->getId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaqueAnimated] = imageOpaqueAnimMat->getId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Text] = textMat->getId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Line] = lineMat->getId();
	}

	SpriteManager::~SpriteManager()
	{
		for(auto& entry : mMaterials)
			bs_delete(entry.second);
	}

	SpriteMaterial* SpriteManager::getMaterial(UINT32 id) const
	{
		auto iterFind = mMaterials.find(id);
		if (iterFind != mMaterials.end())
			return iterFind->second;

		return nullptr;
	}
}