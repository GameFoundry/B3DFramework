//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RenderTexture.h"
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "RenderAPI/B3DRenderTarget.h"

using namespace b3d;
using namespace b3d::render;

D3D12RenderTexture::D3D12RenderTexture(const RenderTextureCreateInformation& createInformation)
	: RenderTexture(createInformation)
{
}

D3D12RenderTexture::~D3D12RenderTexture()
{
	mFramebuffer = nullptr;
}

void D3D12RenderTexture::Initialize()
{
	RenderTexture::Initialize();

	// Create the framebuffer now that textures are initialized
	CreateFramebuffer();
}

void D3D12RenderTexture::CreateFramebuffer()
{
	// Create D3D12 framebuffer with render target views and depth-stencil view
	// The D3D12Framebuffer constructor will handle:
	// 1. Getting color and depth-stencil textures from the RenderTarget
	// 2. Creating render target views (RTVs) for each color attachment
	// 3. Creating depth-stencil view (DSV) if depth-stencil attachment exists
	// 4. Storing descriptor handles
	mFramebuffer = B3DNew<D3D12Framebuffer>(this);
}
