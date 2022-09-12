//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsSpriteManager.h"
#include "2D/BsSpriteMaterials.h"

namespace bs
{
	SpriteManager::SpriteManager()
	{
#ifndef BS_IS_ASSET_TOOL
		SpriteMaterial* imageOpaqueMat = registerMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Opaque, false);
		SpriteMaterial* imageAlphaMat = registerMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Alpha, false);
		SpriteMaterial* imagePremultipliedMat = registerMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Premultiplied, false);
		SpriteMaterial* imageOpaqueAnimMat = registerMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Opaque, true);
		SpriteMaterial* imageAlphaAnimMat = registerMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Alpha, true);
		SpriteMaterial* imagePremultipliedAnimMat = registerMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Premultiplied, true);
		SpriteMaterial* textMat = registerMaterial<SpriteTextMaterial>();
		SpriteMaterial* lineMat = registerMaterial<SpriteLineMaterial>();

		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaque] = imageOpaqueMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparentAlpha] = imageAlphaMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparentPremultiplied] = imagePremultipliedMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaqueAnimated] = imageOpaqueAnimMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparentAlphaAnimated] = imageAlphaAnimMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparentPremultipliedAnimated] = imagePremultipliedAnimMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Text] = textMat->GetId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Line] = lineMat->GetId();
#endif
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
