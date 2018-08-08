//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "BsGLBuffer.h"

namespace bs { namespace ct
{ 
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of an index buffer. */
	class GLIndexBuffer : public IndexBuffer
	{
	public:
		GLIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/**	Returns internal OpenGL index buffer handle. */
		GLuint getGLBufferId() const { return static_cast<GLBuffer*>(mBuffer)->getGLBufferId(); }

	protected:
		/** @copydoc IndexBuffer::initialize */
		void initialize() override;	
	};

	/** @} */
}}