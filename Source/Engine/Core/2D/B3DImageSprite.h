//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "2D/B3DSprite.h"
#include "Math/B3DVector2.h"
#include "Image/B3DColor.h"

namespace b3d
{
	/** @addtogroup 2D
	 *  @{
	 */

	/**	Information used for initializing or updating an image sprite. */
	struct ImageSpriteInformation : SpriteInformation
	{
		ImageSpriteInformation() = default;

		SpriteAnchor Anchor = SA_TopLeft; /**< Determines where in the provided bounds will the sprite be placed. */
		Vector2 UvScale = Vector2(1.0f, 1.0f); /**< Scale applied to UV width/height used for rendering the sprite. */
		Vector2 UvOffset = Vector2(0.0f, 0.0f); /**< Offset applied to UV coordinates when rendering the sprite. */

		HSpriteImage Image; /**< Image to overlay on the sprite. */
		/**
		 * Time (since application start) at which the sprite texture's 0th frame is played. Used if the sprite texture
		 * has sprite sheet animation defined.
		 */
		float AnimationStartTime = 0.0f;

		/**
		 * Borders (in texels) that allow you to control how is the texture scaled. If borders are 0 the texture will be
		 * scaled uniformly. If they are not null only the area inside the borders will be scaled and the outside are will
		 * remain the original size as in the texture. This allows you to implement "Scale9Grid" functionality.
		 */
		u32 BorderLeft = 0;
		u32 BorderRight = 0;
		u32 BorderTop = 0;
		u32 BorderBottom = 0;
	};

	/**	A sprite consisting of a single image represented by a sprite texture. */
	class B3D_EXPORT ImageSprite : public Sprite
	{
	public:
		ImageSprite() = default;
		~ImageSprite();

		/**
		 * Recreates internal sprite data according the specified description structure.
		 *
		 * @param	information		Describes the geometry and material of the sprite.
		 * @param	groupId			Group identifier that forces different materials to be used for different groups (for
		 *							example you don't want the sprites to share the same material if they use different world
		 *							transform matrices).
		 */
		void Update(const ImageSpriteInformation& information, u64 groupId);

		/**
		 * Calculates the required UV scale in order for a texture of size @p sourceSize to be placed on the surface
		 * of @p destinationSize size, while respecting the chosen scale mode.
		 */
		static Vector2 GetTextureUvScale(Size2I sourceSize, Size2I destinationSize, TextureScaleMode scaleMode);

	private:
		/**	Clears internal geometry buffers. */
		void ClearMesh();

		TShared<SpriteImageAllocation> mSpriteImageAllocations;
	};

	/** @} */
} // namespace b3d
