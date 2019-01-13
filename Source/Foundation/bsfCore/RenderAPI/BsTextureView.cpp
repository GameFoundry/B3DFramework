//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsTextureView.h"
#include "Image/BsTexture.h"

namespace bs { namespace ct 
{
	size_t TextureView::HashFunction::operator()(const TEXTURE_VIEW_DESC &key) const
	{
		size_t seed = 0;
		hash_combine(seed, key.mostDetailMip);
		hash_combine(seed, key.numMips);
		hash_combine(seed, key.firstArraySlice);
		hash_combine(seed, key.numArraySlices);
		hash_combine(seed, key.usage);

		return seed;
	}

	bool TextureView::EqualFunction::operator()
		(const TEXTURE_VIEW_DESC &a, const TEXTURE_VIEW_DESC &b) const
	{
		return a.mostDetailMip == b.mostDetailMip && a.numMips == b.numMips 
			&& a.firstArraySlice == b.firstArraySlice && a.numArraySlices == b.numArraySlices && a.usage == b.usage;
	}

	TextureView::TextureView(const TEXTURE_VIEW_DESC& desc)
		:mDesc(desc)
	{

	}
}}