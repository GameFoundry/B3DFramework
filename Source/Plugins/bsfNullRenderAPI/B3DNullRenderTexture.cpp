//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderTexture.h"

namespace b3d
{
	NullRenderTexture::NullRenderTexture(const RenderTextureCreateInformation& createInformation)
		: RenderTexture(createInformation)
	{ }

	namespace render
	{
		NullRenderTexture::NullRenderTexture(const RenderTextureCreateInformation& createInformation)
			: RenderTexture(createInformation)
		{ }
	} // namespace render
} // namespace b3d
