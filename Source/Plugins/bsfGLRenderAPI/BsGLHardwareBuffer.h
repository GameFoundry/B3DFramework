//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Allocators/BsPoolAlloc.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/** Wrapper around a generic OpenGL buffer. */
		class GLHardwareBuffer : public GpuBuffer
	{
		public:
			/** Creates and initializes the buffer object. */
			GLHardwareBuffer(GLenum target, u32 size, GpuBufferUsage usage);
			~GLHardwareBuffer() override;

			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override;
			void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<ct::CommandBuffer>& commandBuffer = nullptr) override;

			/**	Returns internal OpenGL buffer ID. */
			GLuint GetGlBufferId() const { return mBufferId; }

		private:
			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) override;
			void Unmap() override;

			GLenum mTarget;
			GLuint mBufferId = 0;

			bool mZeroLocked = false;
		};

		/** @} */
	} // namespace ct
} // namespace bs

namespace bs
{
	B3D_IMPLEMENT_GLOBAL_POOL(ct::GLHardwareBuffer, 32)
}
