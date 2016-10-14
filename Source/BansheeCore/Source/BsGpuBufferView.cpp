//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuBufferView.h"

namespace BansheeEngine
{
	size_t GpuBufferView::HashFunction::operator()(const GPU_BUFFER_VIEW_DESC& key) const
	{
		size_t seed = 0;
		hash_combine(seed, key.elementWidth);
		hash_combine(seed, key.firstElement);
		hash_combine(seed, key.numElements);
		hash_combine(seed, key.useCounter);
		hash_combine(seed, key.usage);
		hash_combine(seed, key.format);

		return seed;
	}

	bool GpuBufferView::EqualFunction::operator()
		(const GPU_BUFFER_VIEW_DESC& a, const GPU_BUFFER_VIEW_DESC& b) const
	{
		return a.elementWidth == b.elementWidth && a.firstElement == b.firstElement && a.numElements == b.numElements 
			&& a.useCounter == b.useCounter && a.usage == b.usage && a.format == b.format;
	}

	GpuBufferView::GpuBufferView()
	{

	}

	GpuBufferView::~GpuBufferView()
	{

	}

	void GpuBufferView::initialize(const SPtr<GpuBufferCore>& buffer, GPU_BUFFER_VIEW_DESC& desc)
	{
		mBuffer = buffer;
		mDesc = desc;
	}
}