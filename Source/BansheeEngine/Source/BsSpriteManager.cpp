//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSpriteManager.h"
#include "BsSpriteMaterials.h"

namespace bs
{
	SpriteManager::SpriteManager()
	{
		SpriteMaterial* imageTransparentMat = registerMaterial<SpriteImageTransparentMaterial>();
		SpriteMaterial* imageOpaqueMat = registerMaterial<SpriteImageOpaqueMaterial>();
		SpriteMaterial* textMat = registerMaterial<SpriteTextMaterial>();
		SpriteMaterial* lineMat = registerMaterial<SpriteLineMaterial>();

		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparent] = imageTransparentMat->getId();
		builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaque] = imageOpaqueMat->getId();
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