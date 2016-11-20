//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsSpriteMaterial.h"

namespace bs
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Contains materials used for sprite rendering. */
	class BS_EXPORT SpriteManager : public Module<SpriteManager>
	{
		/** Types of sprite materials accessible by default. */
		enum class BuiltinSpriteMaterialType
		{
			ImageTransparent,
			ImageOpaque,
			Text,
			Line,
			Count // Keep at end
		};

	public:
		SpriteManager();
		~SpriteManager();

		/** Returns the material used for rendering transparent image sprites. */
		SpriteMaterial* getImageTransparentMaterial() const 
			{ return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparent]); }

		/** Returns the material used for rendering opaque image sprites. */
		SpriteMaterial* getImageOpaqueMaterial() const
			{ return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaque]); }

		/** Returns the material used for rendering text sprites. */
		SpriteMaterial* getTextMaterial() const
			{ return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Text]); }

		/** Returns the material used for rendering antialiased lines. */
		SpriteMaterial* getLineMaterial() const
			{ return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Line]); }

		/** Returns a sprite material with the specified ID. Returns null if one cannot be found. */
		SpriteMaterial* getMaterial(UINT32 id) const;

		/** 
		 * Registers a new material in the sprite manager. Caller must ensure the material has a unique ID that doesn't
		 * already exist in the sprite manager, otherwise the call will be ignored.
		 *
		 * @return	Newly created material, or at existing one if one already exists.
		 */
		template <class T>
		SpriteMaterial* registerMaterial()
		{
			SpriteMaterial* newMaterial = bs_new<T>();
			
			UINT32 id = newMaterial->getId();
			auto iterFind = mMaterials.find(id);
			if (iterFind != mMaterials.end())
			{
				// Already exists
				LOGWRN("Attempting to register a sprite material that already exists, ignoring request.");
				bs_delete(newMaterial);
				return iterFind->second;
			}

			mMaterials[id] = newMaterial;
			return newMaterial;
		}
	private:
		UnorderedMap<UINT32, SpriteMaterial*> mMaterials;
		UINT32 builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::Count];
	};

	/** @} */
}