//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsSprite.h"
#include "BsVector2.h"
#include "BsColor.h"

namespace BansheeEngine
{
	/** @addtogroup 2D
	 *  @{
	 */

	/**	Image sprite description structure used for initializing or updating an image sprite. */
	struct IMAGE_SPRITE_DESC
	{
		IMAGE_SPRITE_DESC()
			: width(0), height(0), anchor(SA_TopLeft), uvScale(1.0f, 1.0f), uvOffset(0.0f, 0.0f), transparent(true)
			, borderLeft(0), borderRight(0), borderTop(0), borderBottom(0)
		{ }

		UINT32 width; /**< Width of the image in pixels. */
		UINT32 height; /**< Height of the image in pixels. */
		SpriteAnchor anchor; /**< Determines where in the provided bounds will the sprite be placed. */
		Vector2 uvScale; /**< Scale applied to UV width/height used for rendering the sprite. */
		Vector2 uvOffset; /**< Offset applied to UV coordinates when rendering the sprite. */
		bool transparent; /**< Should the sprite be rendered with transparency. */

		SPtr<SpriteTexture> texture; /**< Texture to overlay on the sprite. */
		Color color; /**< Color tint to apply to the sprite. */

		/** 
		 * Borders (in texels) that allow you to control how is the texture scaled. If borders are 0 the texture will be 
		 * scaled uniformly. If they are not null only the area inside the borders will be scaled and the outside are will
		 * remain the original size as in the texture. This allows you to implement "Scale9Grid" functionality.
		 */
		UINT32 borderLeft, borderRight, borderTop, borderBottom; 
	};

	/**	A sprite consisting of a single image represented by a sprite texture. */
	class BS_EXPORT ImageSprite : public Sprite
	{
	public:
		ImageSprite();
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