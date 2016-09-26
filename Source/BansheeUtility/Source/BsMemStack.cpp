//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisitesUtil.h"
#include "BsMemStack.h"

namespace BansheeEngine
{
	BS_THREADLOCAL MemStackInternal<1024 * 1024>* MemStack::ThreadMemStack = nullptr;

	void MemStack::beginThread()
	{
		if(ThreadMemStack != nullptr)
			endThread();

		ThreadMemStack = bs_new<MemStackInternal<1024 * 1024>>();
	}

	void MemStack::endThread()
	{
		if(ThreadMemStack != nullptr)
		{
			bs_delete(ThreadMemStack);
			ThreadMemStack = nullptr;
		}
	}

	UINT8* MemStack::alloc(UINT32 numBytes)
	{
		assert(ThreadMemStack != nullptr && "Stack allocation failed. Did you call beginThread?");

		return ThreadMemStack->alloc(numBytes);
	}

	void MemStack::deallocLast(UINT8* data)
	{
		assert(ThreadMemStack != nullptr && "Stack deallocation failed. Did you call beginThread?");

		ThreadMemStack->dealloc(data);
	}
}
