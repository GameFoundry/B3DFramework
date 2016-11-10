//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsVertexBuffer.h"
#include "BsGLBuffer.h"
#include "BsGLVertexArrayObjectManager.h"

namespace BansheeEngine 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a vertex buffer. */
    class BS_RSGL_EXPORT GLVertexBufferCore : public VertexBufferCore
    {
    public:
		GLVertexBufferCore(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~GLVertexBufferCore();

		/** @copydoc VertexBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* dest, UINT32 queueIdx = 1) override;

		/** @copydoc VertexBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 1) override;

		/**	Returns internal OpenGL buffer ID. */
        GLuint getGLBufferId() const { return mBuffer.getGLBufferId(); }

		/**	Registers a new VertexArrayObject that uses this vertex buffer. */
		void registerVAO(const GLVertexArrayObject& vao);

		/**	Unregisters a VAO from this vertex buffer. Does not destroy it. */
		void unregisterVAO(const GLVertexArrayObject& vao);

	protected:
		/** @copydoc VertexBufferCore::initialize */
		void initialize() override;

		/** @copydoc VertexBufferCore::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc VertexBufferCore::unmap */
		void unmap() override;

	private:
		GLBuffer mBuffer;
		GpuBufferUsage mUsage;

		Vector<GLVertexArrayObject> mVAObjects;
    };

	/** @} */
}