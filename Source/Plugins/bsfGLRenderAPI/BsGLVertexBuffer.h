//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "BsGLHardwareBuffer.h"
#include "BsGLVertexArrayObjectManager.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a vertex buffer. */
	class GLVertexBuffer : public VertexBuffer
	{
	public:
		GLVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		~GLVertexBuffer();

		/**	Returns internal OpenGL buffer ID. */
		GLuint getGLBufferId() const { return static_cast<GLHardwareBuffer*>(mBuffer)->getGLBufferId(); }

		/**	Registers a new VertexArrayObject that uses this vertex buffer. */
		void registerVAO(const GLVertexArrayObject& vao);

		/**	Unregisters a VAO from this vertex buffer. Does not destroy it. */
		void unregisterVAO(const GLVertexArrayObject& vao);

	protected:
		/** @copydoc VertexBuffer::initialize */
		void initialize() override;

	private:
		Vector<GLVertexArrayObject> mVAObjects;
	};

	/** @} */
}}