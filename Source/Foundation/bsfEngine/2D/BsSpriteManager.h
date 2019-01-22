//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "2D/BsSpriteMaterial.h"

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
			ImageTransparentAnimated,
			ImageOpaqueAnimated,
			Text,
			Line,
			Count // Keep at end
		};

	public:
		SpriteManager();
		~SpriteManager();

		/** Returns the material used for rendering image sprites. 
		 *
		 * @param[in]	transparent		True if the material should be able to render transparecy.
		 * @param[in]	animation		True if the material should be able to perform sprite sheet animation.
		 * @return						Requested sprite material.
		 */
		SpriteMaterial* getImageMaterial(bool transparent, bool animation = false) const
		{
			if(!animation)
			{
				if(transparent)
					return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparent]);

				return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaque]);
			}
			else
			{
				if(transparent)
					return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageTransparentAnimated]);

				return getMaterial(builtinMaterialIds[(UINT32)BuiltinSpriteMaterialType::ImageOpaqueAnimated]);
			}
		}

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
		template <class T, class... Args>
		SpriteMaterial* registerMaterial(Args &&...args)
		{
			SpriteMaterial* newMaterial = bs_new<T>(std::forward<Args>(args)...);
			
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