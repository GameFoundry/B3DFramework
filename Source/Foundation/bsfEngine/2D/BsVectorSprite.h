//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsSprite.h"
#include "Image/BsColor.h"

namespace bs
{
	/** @addtogroup 2D
	 *  @{
	 */

	/**	Information used for initializing or updating a vector sprite. */
	struct VectorSpriteInformation
	{
		VectorSpriteInformation() = default;

		u32 Width = 0; /**< Width of the sprite in pixels. */
		u32 Height = 0; /**< Height of the sprite in pixels. */
		bool Transparent = true; /**< Should the sprite be rendered with transparency. */

		HVectorPath VectorPath; /**< Vector path to render on the sprite. */
		Color Color; /**< Color tint to apply to the sprite. */

		/**
		 * Borders (in texels) that allow you to control how is the shape scaled. If borders are 0 the texture will be
		 * scaled uniformly. If they are not null only the area inside the borders will be scaled and the outside are will
		 * remain the original size as in the texture. This allows you to implement "Scale9Grid" functionality.
		 */
		u32 BorderLeft = 0;
		u32 BorderRight = 0;
		u32 BorderTop = 0;
		u32 BorderBottom = 0;
	};

	/**	A sprite consisting of a vector path represented by a sprite texture. */
	class B3D_EXPORT VectorSprite : public Sprite
	{
	public:
		VectorSprite() = default;
		~VectorSprite();

		/**
		 * Recreates internal sprite data according the specified description structure.
		 *
		 * @param	information		Describes the geometry and material of the sprite.
		 * @param	groupId			Group identifier that forces different materials to be used for different groups (for
		 *							example you don't want the sprites to share the same material if they use different world
		 *							transform matrices).
		 */
		void Update(const VectorSpriteInformation& information, u64 groupId);

	private:
		/**	Clears internal geometry buffers. */
		void ClearMesh();

		Array<Vector2, 4> mPositionBuffer;
		Array<Vector2, 4> mUVBuffer;
		Array<u32, 6> mIndexBuffer;
	};

	/** @} */
} // namespace bs
