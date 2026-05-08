//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DSignal.h"
#include "B3DSignalEvent.h"
#include "B3DThreadPool.h"
#include "Debug/B3DDebug.h"

namespace b3d
{
	namespace detail
	{
		template <typename ReturnType>
		class RunOnNewThreadHelper
		{
		public:
			template <typename F, typename... Args>
			static ReturnType Run(F&& f, Args&&... args) {
				ReturnType result;

				SignalEvent event;
				Scheduler* const scheduler = Scheduler::Get();

				const TShared<PooledThread> thread = ThreadPool::Instance().Run("BlockingCall",
					[&result, &f, &args..., scheduler, &event]()
					{
						if (scheduler != nullptr)
							scheduler->BindToCurrentThread();

						result = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);

						if (scheduler != nullptr)
							Scheduler::UnbindFromCurrentThread();

						event.Signal();
					});

				event.Wait();
				thread->BlockUntilComplete();

				return result;
			}
		};

		template <>
		class RunOnNewThreadHelper<void>
		{
		public:
			template <typename F, typename... Args>
			static void Run(F&& f, Args&&... args) {
				SignalEvent event;
				Scheduler* const scheduler = Scheduler::Get();

				const TShared<PooledThread> thread = ThreadPool::Instance().Run("BlockingCall",
					[&f, &args..., scheduler, &event]()
					{
						if (scheduler != nullptr)
							scheduler->BindToCurrentThread();

						std::invoke(std::forward<F>(f), std::forward<Args>(args)...);

						if (scheduler != nullptr)
							Scheduler::UnbindFromCurrentThread();

						event.Signal();
					});

				event.Wait();
				thread->BlockUntilComplete();
			}
		};
	}  // namespace detail

	/** Runs the provided function on its own thread and allows the calling fiber to yield and let other work happen. Useful if you expect the provided function to block for a longer period of time, but still want to let the current thread continue doing work. */
	template <typename F, typename... Args>
	auto RunBlockingCallAsYieldable(F&& f, Args&&... args) -> decltype(f(args...))
	{
		return detail::RunOnNewThreadHelper<decltype(f(args...))>::Run(std::forward<F>(f), std::forward<Args>(args)...);
	}
}  // namespace b3d
