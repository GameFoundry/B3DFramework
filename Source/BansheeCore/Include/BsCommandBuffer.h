//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** 
	 * Contains a list of render API commands that can be queued for execution on the GPU. User is allowed to populate the
	 * command buffer from any thread, ensuring render API command generation can be multi-threaded. Command buffers
	 * must always be created on the core thread. Same command buffer cannot be used on multiple threads simulateously
	 * without external synchronization.
	 */
	class BS_CORE_EXPORT CommandBuffer
	{
	public:
		virtual ~CommandBuffer() {}

		/**
		 * Creates a new CommandBuffer.
		 * 
		 * @param[in]	type		Determines what type of commands can be added to the command buffer.
		 * @param[in]	deviceIdx	Index of the GPU the command buffer will be used to queue commands on. 0 is always
		 *							the primary available GPU.
		 * @param[in]	syncMask	Determines how is concurrency handled between multiple command buffers on the same 
		 *							device. If buffers don't share the same bits in the sync mask they are allowed to
		 *							execute concurrently on the GPU, however it is up to the user to ensure that they
		 *							do not contain any resources depended on each other. Leave on default to ensure
		 *							no concurrency is possible (safest).
		 * @param[in]	secondary	If true the command buffer will not be allowed to execute on its own, but it can
		 *							be appended to a primary command buffer. 
		 * @return					New CommandBuffer instance.
		 * 
		 */
		static SPtr<CommandBuffer> create(CommandBufferType type, UINT32 deviceIdx = 0, UINT32 syncMask = 0xFFFFFFFF, 
			bool secondary = false);

	protected:
		CommandBuffer(CommandBufferType type, UINT32 syncMask, bool secondary);

		CommandBufferType mType;
		UINT32 mSyncMask;
		bool mIsSecondary;
	};

	/** @} */
}