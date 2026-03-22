//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DRendererId.h"
#include "Utility/B3DModule.h"
#include "Allocators/B3DFrameAllocator.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Module that orchestrates ECS-based batch sync between main thread objects and their renderer-side representations.
	 * Iterates all active scenes each frame and triggers their synchronization. Manages ring-buffered frame allocators
	 * to ensure data survives until the render thread consumes it.
	 */
	class B3D_EXPORT RendererSyncManager : public Module<RendererSyncManager>
	{
	public:
		RendererSyncManager();
		~RendererSyncManager();

		/**
		 * Run the full sync pipeline: for each active scene, reads dirty data on the main thread and posts a command
		 * to apply it on the render thread. If @p swapBuffers is true, rotate the frame allocator ring buffer.
		 */
		void SyncToRenderThread(bool swapBuffers);

	private:
		FrameAllocator* mSyncAllocators[RenderThread::kSyncBufferCount + 1];
		u32 mActiveFrameAllocatorIndex = 0;
	};

	/** @} */
} // namespace b3d
