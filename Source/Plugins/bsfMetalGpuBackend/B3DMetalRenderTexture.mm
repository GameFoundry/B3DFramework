//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalRenderTexture.h"
#include "B3DMetalFramebuffer.h"
#include "B3DMetalTexture.h"

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

		MetalRenderTexture::~MetalRenderTexture() = default;

		void MetalRenderTexture::Initialize()
		{
			RenderTexture::Initialize();

			MetalFramebufferCreateInformation framebufferCreateInformation;
			framebufferCreateInformation.Width = mRenderTargetProperties.Width;
			framebufferCreateInformation.Height = mRenderTargetProperties.Height;

			auto fnPopulateAttachment = [](const RenderSurfaceInformation& surfaceInformation, MetalFramebufferAttachmentCreateInformation& attachment)
			{
				attachment.Texture = static_cast<MetalTexture*>(surfaceInformation.Texture.get());
				attachment.Face = surfaceInformation.Face;
				attachment.MipLevel = surfaceInformation.MipLevel;
			};

			for (u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
				fnPopulateAttachment(GetColorSurfaceInformation(colorIndex), framebufferCreateInformation.ColorAttachments[colorIndex]);

			fnPopulateAttachment(GetDepthStencilSurfaceInformation(), framebufferCreateInformation.DepthStencilAttachment);
			mFramebuffer = B3DMakeUnique<MetalFramebuffer>(framebufferCreateInformation);
		}
	} // namespace render
} // namespace b3d
