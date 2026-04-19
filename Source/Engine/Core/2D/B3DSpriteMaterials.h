//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "2D/B3DSpriteMaterial.h"

namespace b3d
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Sprite material used for rendering images. */
	class B3D_EXPORT SpriteImageMaterial : public SpriteMaterial
	{
	public:
		SpriteImageMaterial(SpriteMaterialTransparency transparency, bool animated = false);
	};

	/** Sprite material used for rendering text. */
	class B3D_EXPORT SpriteTextMaterial : public SpriteMaterial
	{
	public:
		SpriteTextMaterial();
	};

	/** Sprite material used for antialiased lines. */
	class B3D_EXPORT SpriteLineMaterial : public SpriteMaterial
	{
	public:
		SpriteLineMaterial();
	};

	/** @} */
} // namespace b3d
