//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsSprite.h"
#include "Math/BsVector2.h"
#include "Image/BsColor.h"

namespace bs
{
	/** @addtogroup 2D
	 *  @{
	 */

	/**	Image sprite description structure used for initializing or updating an image sprite. */
	struct IMAGE_SPRITE_DESC
	{
		IMAGE_SPRITE_DESC() = default;

		UINT32 width = 0; /**< Width of the image in pixels. */
		UINT32 height = 0; /**< Height of the image in pixels. */
		SpriteAnchor anchor = SA_TopLeft; /**< Determines where in the provided bounds will the sprite be placed. */
		Vector2 uvScale = Vector2(1.0f, 1.0f); /**< Scale applied to UV width/height used for rendering the sprite. */
		Vector2 uvOffset = Vector2(0.0f, 0.0f); /**< Offset applied to UV coordinates when rendering the sprite. */
		bool transparent = true; /**< Should the sprite be rendered with transparency. */

		HSpriteTexture texture; /**< Texture to overlay on the sprite. */
		Color color; /**< Color tint to apply to the sprite. */
		/** 
		 * Time (since application start) at which the sprite texture's 0th frame is played. Used if the sprite texture
		 * has sprite sheet animation defined.
		 */
		float animationStartTime = 0.0f;

		/** 
		 * Borders (in texels) that allow you to control how is the texture scaled. If borders are 0 the texture will be 
		 * scaled uniformly. If they are not null only the area inside the borders will be scaled and the outside are will
		 * remain the original size as in the texture. This allows you to implement "Scale9Grid" functionality.
		 */
		UINT32 borderLeft = 0;
		UINT32 borderRight = 0;
		UINT32 borderTop = 0;
		UINT32 borderBottom = 0;
	};

	/**	A sprite consisting of a single image represented by a sprite texture. */
	class BS_EXPORT ImageSprite : public Sprite
	{
	public:
		ImageSprite() = default;
		~ImageSprite();

		/**
		 * Recreates internal sprite data according the specified description structure.
		 *
		 * @param[in]	desc	Describes the geometry and material of the sprite.
		 * @param[in]	groupId	Group identifier that forces different materials to be used for different groups (for 
		 *						example you don't want the sprites to share the same material if they use different world
		 *						transform matrices).
		 */
		void update(const IMAGE_SPRITE_DESC& desc, UINT64 groupId);

		/** 
		 * Calculates the required UV scale in order for a texture of size @p sourceSize to be placed on the surface
		 * of @p destSize size, while respecting the chosen scale mode.
		 */
		static Vector2 getTextureUVScale(Vector2I sourceSize, Vector2I destSize, TextureScaleMode scaleMode);

	private:
		/**	Clears internal geometry buffers. */
		void clearMesh();
	};

	/** @} */
}