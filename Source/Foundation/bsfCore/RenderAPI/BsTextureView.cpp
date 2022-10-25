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
	bs_hash_combine(seed, key.MostDetailMip);
	bs_hash_combine(seed, key.NumMips);
	bs_hash_combine(seed, key.FirstArraySlice);
	bs_hash_combine(seed, key.NumArraySlices);
	bs_hash_combine(seed, key.Usage);

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
