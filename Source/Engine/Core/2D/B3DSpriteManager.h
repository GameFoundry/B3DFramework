//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "2D/B3DSpriteMaterial.h"

namespace b3d
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Contains materials used for sprite rendering. */
	class B3D_EXPORT SpriteManager : public Module<SpriteManager>
	{
		/** Types of sprite materials accessible by default. */
		enum class BuiltinSpriteMaterialType
		{
			ImageOpaque,
			ImageTransparentAlpha,
			ImageTransparentPremultiplied,
			ImageOpaqueAnimated,
			ImageTransparentAlphaAnimated,
			ImageTransparentPremultipliedAnimated,
			Text,
			Line,
			Count // Keep at end
		};

	public:
		SpriteManager();
		~SpriteManager();

		/**
		 * Returns the material used for rendering image sprites.
		 *
		 * @param	transparency	Transparency mode the material should support.
		 * @param	animation		True if the material should be able to perform sprite sheet animation.
		 * @return					Requested sprite material.
		 */
		SpriteMaterial* GetImageMaterial(SpriteMaterialTransparency transparency, bool animation = false) const
		{
			if(!animation)
			{
				switch(transparency)
				{
				default:
				case SpriteMaterialTransparency::Opaque:
					return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageOpaque]);
				case SpriteMaterialTransparency::Alpha:
					return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentAlpha]);
				case SpriteMaterialTransparency::Premultiplied:
					return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentPremultiplied]);
				}
			}
			else
			{
				switch(transparency)
				{
				default:
				case SpriteMaterialTransparency::Opaque:
					return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageOpaqueAnimated]);
				case SpriteMaterialTransparency::Alpha:
					return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentAlphaAnimated]);
				case SpriteMaterialTransparency::Premultiplied:
					return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::ImageTransparentPremultipliedAnimated]);
				}
			}
		}

		/** Returns the material used for rendering text sprites. */
		SpriteMaterial* GetTextMaterial() const
		{
			return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::Text]);
		}

		/** Returns the material used for rendering antialiased lines. */
		SpriteMaterial* GetLineMaterial() const
		{
			return GetMaterial(builtinMaterialIds[(u32)BuiltinSpriteMaterialType::Line]);
		}

		/** Returns a sprite material with the specified ID. Returns null if one cannot be found. */
		SpriteMaterial* GetMaterial(u32 id) const;

		/**
		 * Registers a new material in the sprite manager. Caller must ensure the material has a unique ID that doesn't
		 * already exist in the sprite manager, otherwise the call will be ignored.
		 *
		 * @return	Newly created material, or at existing one if one already exists.
		 */
		template <class T, class... Args>
		SpriteMaterial* RegisterMaterial(Args&&... args)
		{
			SpriteMaterial* newMaterial = B3DNew<T>(std::forward<Args>(args)...);

			u32 id = newMaterial->GetId();
			auto found = mMaterials.find(id);
			if(found != mMaterials.end())
			{
				// Already exists
				B3D_LOG(Warning, LogGeneric, "Attempting to register a sprite material that already exists, ignoring request.");
				B3DDelete(newMaterial);
				return found->second;
			}

			mMaterials[id] = newMaterial;
			return newMaterial;
		}

	private:
		UnorderedMap<u32, SpriteMaterial*> mMaterials;
		u32 builtinMaterialIds[(u32)BuiltinSpriteMaterialType::Count]{};
	};

	/** @} */
} // namespace b3d
