//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisitesUtil.h"
#include "BsGlobalFrameAlloc.h"
#include "BsFrameAlloc.h"

namespace bs
{
	BS_THREADLOCAL FrameAlloc* _GlobalFrameAlloc = nullptr;

	inline BS_UTILITY_EXPORT FrameAlloc& gFrameAlloc()
	{
		if (_GlobalFrameAlloc == nullptr)
		{
			// Note: This will leak memory but since it should exist throughout the entirety 
			// of runtime it should only leak on shutdown when the OS will free it anyway.
			_GlobalFrameAlloc = new FrameAlloc();
		}

		return *_GlobalFrameAlloc;
	}

	inline BS_UTILITY_EXPORT UINT8* bs_frame_alloc(UINT32 numBytes)
	{
		return gFrameAlloc().alloc(numBytes);
	}

	inline BS_UTILITY_EXPORT UINT8* bs_frame_alloc_aligned(UINT32 count, UINT32 align)
	{
		return gFrameAlloc().allocAligned(count, align);
	}

	inline BS_UTILITY_EXPORT void bs_frame_free(void* data)
	{
		gFrameAlloc().dealloc((UINT8*)data);
	}

	inline BS_UTILITY_EXPORT void bs_frame_free_aligned(void* data)
	{
		gFrameAlloc().dealloc((UINT8*)data);
	}

	inline BS_UTILITY_EXPORT void bs_frame_mark()
	{
		gFrameAlloc().markFrame();
	}

	inline BS_UTILITY_EXPORT void bs_frame_clear()
	{
		gFrameAlloc().clear();
	}
}