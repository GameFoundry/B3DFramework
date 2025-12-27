//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DTextureView.h"
#include "Image/B3DTexture.h"

using namespace b3d;

namespace b3d { namespace render
{
size_t TextureView::HashFunction::operator()(const TextureViewInformation &key) const
{
	size_t seed = 0;
	B3DCombineHash(seed, key.Surface.MipLevel);
	B3DCombineHash(seed, key.Surface.MipLevelCount);
	B3DCombineHash(seed, key.Surface.Face);
	B3DCombineHash(seed, key.Surface.FaceCount);
	B3DCombineHash(seed, key.Surface.IsBoundAs2DArray);
	B3DCombineHash(seed, key.Usage);

	return seed;
}

bool TextureView::EqualFunction::operator()(const TextureViewInformation &a, const TextureViewInformation &b) const
{
	return a.Surface == b.Surface && a.Usage == b.Usage;
}

TextureView::TextureView(const TextureViewInformation &desc)
	: mInformation(desc)
{
}
}}
