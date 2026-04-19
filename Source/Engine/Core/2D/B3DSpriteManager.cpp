//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DSpriteManager.h"
#include "2D/B3DSpriteMaterials.h"

using namespace b3d;

SpriteManager::SpriteManager()
{
	SpriteMaterial* imageOpaqueMat = RegisterMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Opaque, false);
	SpriteMaterial* imageAlphaMat = RegisterMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Alpha, false);
	SpriteMaterial* imagePremultipliedMat = RegisterMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Premultiplied, false);
	SpriteMaterial* imageOpaqueAnimMat = RegisterMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Opaque, true);
	SpriteMaterial* imageAlphaAnimMat = RegisterMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Alpha, true);
	SpriteMaterial* imagePremultipliedAnimMat = RegisterMaterial<SpriteImageMaterial>(SpriteMaterialTransparency::Premultiplied, true);
	SpriteMaterial* textMat = RegisterMaterial<SpriteTextMaterial>();
	SpriteMaterial* lineMat = RegisterMaterial<SpriteLineMaterial>();

	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageOpaque] = imageOpaqueMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentAlpha] = imageAlphaMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentPremultiplied] = imagePremultipliedMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageOpaqueAnimated] = imageOpaqueAnimMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentAlphaAnimated] = imageAlphaAnimMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentPremultipliedAnimated] = imagePremultipliedAnimMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::Text] = textMat->GetId();
	builtinMaterialIds[(u32)BuiltinSpriteMaterialType::Line] = lineMat->GetId();
}

SpriteManager::~SpriteManager()
{
	for(auto& entry : mMaterials)
		B3DDelete(entry.second);
}

SpriteMaterial* SpriteManager::GetMaterial(u32 id) const
{
	auto found = mMaterials.find(id);
	if(found != mMaterials.end())
		return found->second;

	return nullptr;
}
