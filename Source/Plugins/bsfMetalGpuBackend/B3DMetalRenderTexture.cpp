//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalRenderTexture.h"

namespace b3d
{
	MetalRenderTexture::MetalRenderTexture(const RenderTextureCreateInformation& createInformation)
		: RenderTexture(createInformation)
	{ }

	namespace render
	{
		MetalRenderTexture::MetalRenderTexture(const RenderTextureCreateInformation& createInformation)
			: RenderTexture(createInformation)
		{ }
	} // namespace render
} // namespace b3d
