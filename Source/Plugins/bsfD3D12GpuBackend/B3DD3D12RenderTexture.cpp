//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RenderTexture.h"
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "GpuBackend/B3DRenderTarget.h"

namespace b3d::render
{
	D3D12RenderTexture::D3D12RenderTexture(const RenderTextureCreateInformation& createInformation)
		: RenderTexture(createInformation)
	{
	}

	D3D12RenderTexture::~D3D12RenderTexture()
	{
		if (mFramebuffer != nullptr)
			B3DDelete(mFramebuffer);
	}

	void D3D12RenderTexture::Initialize()
	{
		RenderTexture::Initialize();

		D3D12FramebufferCreateInformation framebufferCreateInformation;
		framebufferCreateInformation.Width = mRenderTargetProperties.Width;
		framebufferCreateInformation.Height = mRenderTargetProperties.Height;

		auto fnPopulateAttachment = [](const RenderSurfaceInformation& surfaceInformation, D3D12FramebufferAttachmentCreateInformation& attachment)
		{
			if (surfaceInformation.Texture == nullptr)
				return;

			D3D12Texture* texture = static_cast<D3D12Texture*>(surfaceInformation.Texture.get());
			D3D12Image* image = texture->GetD3D12Image();
			if (image == nullptr || image->GetD3D12Resource() == nullptr)
				return;

			const TextureProperties& properties = surfaceInformation.Texture->GetProperties();
			const u32 availableFaceCount = properties.Type == TEX_TYPE_3D
				? Math::Max(1u, properties.Depth >> surfaceInformation.MipLevel)
				: properties.GetFaceCount();
			const u32 faceCount = surfaceInformation.FaceCount == 0
				? availableFaceCount - surfaceInformation.Face
				: surfaceInformation.FaceCount;

			attachment.Image = image;
			attachment.Surface = TextureSurface(surfaceInformation.MipLevel, 1, surfaceInformation.Face, faceCount);
		};

		for (u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
			fnPopulateAttachment(GetColorSurfaceInformation(colorIndex), framebufferCreateInformation.ColorAttachments[colorIndex]);

		fnPopulateAttachment(GetDepthStencilSurfaceInformation(), framebufferCreateInformation.DepthStencilAttachment);
		mFramebuffer = B3DNew<D3D12Framebuffer>(framebufferCreateInformation);
	}
} // namespace b3d::render
