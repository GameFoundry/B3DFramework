//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsTextureView.h"
#include "Image/BsTexture.h"

using namespace bs;

namespace bs { namespace ct
{
size_t TextureView::HashFunction::operator()(const TEXTURE_VIEW_DESC &key) const
{
	size_t seed = 0;
	B3DCombineHash(seed, key.MostDetailMip);
	B3DCombineHash(seed, key.NumMips);
	B3DCombineHash(seed, key.FirstArraySlice);
	B3DCombineHash(seed, key.NumArraySlices);
	B3DCombineHash(seed, key.Usage);

	return seed;
}

bool TextureView::EqualFunction::operator()(const TEXTURE_VIEW_DESC &a, const TEXTURE_VIEW_DESC &b) const
{
	return a.MostDetailMip == b.MostDetailMip && a.NumMips == b.NumMips && a.FirstArraySlice == b.FirstArraySlice && a.NumArraySlices == b.NumArraySlices && a.Usage == b.Usage;
}

TextureView::TextureView(const TEXTURE_VIEW_DESC &desc)
	: mDesc(desc)
{
}
}}
