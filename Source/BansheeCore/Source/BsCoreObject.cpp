//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreObject.h"
#include "BsCoreObjectCore.h"
#include "BsCoreThread.h"
#include "BsCoreObjectManager.h"

using namespace std::placeholders;

namespace bs
{
	CoreObject::CoreObject(bool initializeOnCoreThread)
		:mFlags(0), mCoreDirtyFlags(0), mInternalID(0)
	{
		mInternalID = CoreObjectManager::instance().registerObject(this);
		mFlags = initializeOnCoreThread ? mFlags | CGO_INIT_ON_CORE_THREAD : mFlags;
	}

	CoreObject::~CoreObject() 
	{
		if(!isDestroyed())
		{
			// Object must be released with destroy() otherwise engine can still try to use it, even if it was destructed
			// (e.g. if an object has one of its methods queued in a command queue, and is destructed, you will be accessing invalid memory)
			BS_EXCEPT(InternalErrorException, "Destructor called but object is not destroyed. This will result in nasty issues.");
		}

#if BS_DEBUG_MODE
		if(!mThis.expired())
		{
			BS_EXCEPT(InternalErrorException, "Shared pointer to this object still has active references but " \
				"the object is being deleted? You shouldn't delete CoreObjects manually.");
		}
#endif
	}

	void CoreObject::destroy()
	{
		CoreObjectManager::instance().unregisterObject(this);
		setIsDestroyed(true);

		if(requiresInitOnCoreThread())
		{
			assert(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId() && "Cannot destroy sim thead object from core thread.");

			// This will only destroy the CoreObjectCore if this was the last reference
			queueDestroyGpuCommand(mCoreSpecific);
		}

		mCoreSpecific = nullptr;
	}

	void CoreObject::initialize()
	{
		mCoreSpecific = createCore();

		if (mCoreSpecific != nullptr)
		{
			if (requiresInitOnCoreThread())
			{
				mCoreSpecific->setScheduledToBeInitialized(true);

				assert(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId() && "Cannot initialize sim thread object from core thread.");

				queueInitializeGpuCommand(mCoreSpecific);
			}
			else
			{
				mCoreSpecific->initialize();

				// Even though this object might not require initialization on the core thread, it will be used on it, therefore
				// do a memory barrier to ensure any stores are finished before continuing (When it requires init on core thread
				// we use the core queue which uses a mutex, and therefore executes all stores as well, so we dont need to 
				// do this explicitly)
				std::atomic_thread_fence(std::memory_order_release); // TODO - Need atomic variable, currently this does nothing
			}
		}

		markDependenciesDirty();
	}

	void CoreObject::blockUntilCoreInitialized() const
	{
		if (mCoreSpecific != nullptr)
			mCoreSpecific->synchronize();
	}

	void CoreObject::syncToCore()
	{
		CoreObjectManager::instance().syncToCore(this);
	}

	void CoreObject::markCoreDirty(UINT32 flags)
	{
		bool wasDirty = isCoreDirty();

		mCoreDirtyFlags |= flags;

		if (!wasDirty && isCoreDirty())
			CoreObjectManager::instance().notifyCoreDirty(this);
	}

	void CoreObject::markDependenciesDirty()
	{
		CoreObjectManager::instance().notifyDependenciesDirty(this);
	}

	void CoreObject::_setThisPtr(SPtr<CoreObject> ptrThis)
	{
		mThis = ptrThis;
	}

	void CoreObject::queueGpuCommand(const SPtr<CoreObjectCore>& obj, std::function<void()> func)
	{
		// We call another internal method and go through an additional layer of abstraction in order to keep an active
		// reference to the obj (saved in the bound function).
		// We could have called the function directly using "this" pointer but then we couldn't have used a shared_ptr for the object,
		// in which case there is a possibility that the object would be released and deleted while still being in the command queue.
		gCoreThread().queueCommand(std::bind(&CoreObject::executeGpuCommand, obj, func));
	}

	AsyncOp CoreObject::queueReturnGpuCommand(const SPtr<CoreObjectCore>& obj, std::function<void(AsyncOp&)> func)
	{
		// See queueGpuCommand
		return gCoreThread().queueReturnCommand(std::bind(&CoreObject::executeReturnGpuCommand, obj, func, _1));
	}

	void CoreObject::queueInitializeGpuCommand(const SPtr<CoreObjectCore>& obj)
	{
		std::function<void()> func = std::bind(&CoreObjectCore::initialize, obj.get());

		CoreThread::instance().queueCommand(std::bind(&CoreObject::executeGpuCommand, obj, func), CTQF_InternalQueue);
	}

	void CoreObject::queueDestroyGpuCommand(const SPtr<CoreObjectCore>& obj)
	{
		std::function<void()> func = [&](){}; // Do nothing function. We just need the shared pointer to stay alive until it reaches the core thread

		gCoreThread().queueCommand(std::bind(&CoreObject::executeGpuCommand, obj, func));
	}

	void CoreObject::executeGpuCommand(const SPtr<CoreObjectCore>& obj, std::function<void()> func)
	{
		volatile SPtr<CoreObjectCore> objParam = obj; // Makes sure obj isn't optimized out?

		func();
	}

	void CoreObject::executeReturnGpuCommand(const SPtr<CoreObjectCore>& obj, std::function<void(AsyncOp&)> func, AsyncOp& op)
	{
		volatile SPtr<CoreObjectCore> objParam = obj; // Makes sure obj isn't optimized out?

		func(op);
	}
}