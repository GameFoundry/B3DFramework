//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsUtilityPrerequisites.h"
#include "Allocators/BsStackAlloc.h"

using namespace bs;

B3D_THREADLOCAL MemStackInternal<1024 * 1024>* MemStack::ThreadMemStack = nullptr;

void MemStack::BeginThread()
{
	if(ThreadMemStack != nullptr)
		EndThread();

	ThreadMemStack = B3DNew<MemStackInternal<1024 * 1024>>();
}

void MemStack::EndThread()
{
	if(ThreadMemStack != nullptr)
	{
		B3DDelete(ThreadMemStack);
		ThreadMemStack = nullptr;
	}
}

u8* MemStack::Alloc(u32 numBytes)
{
	B3D_ASSERT(ThreadMemStack != nullptr && "Stack allocation failed. Did you call beginThread?");

	return ThreadMemStack->Alloc(numBytes);
}

void MemStack::DeallocLast(u8* data)
{
	B3D_ASSERT(ThreadMemStack != nullptr && "Stack deallocation failed. Did you call beginThread?");

	ThreadMemStack->Dealloc(data);
}
