//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLFrameBufferObject.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "BsGLRenderTexture.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	GLFrameBufferObject::GLFrameBufferObject()
	{
		glGenFramebuffers(1, &mFB);
		BS_CHECK_GL_ERROR();

		for (UINT32 x = 0; x < BS_MAX_MULTIPLE_RENDER_TARGETS; ++x)
			mColor[x].buffer = nullptr;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_FrameBufferObject);
	}

	GLFrameBufferObject::~GLFrameBufferObject()
	{
		glDeleteFramebuffers(1, &mFB);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_FrameBufferObject);
	}

	void GLFrameBufferObject::bindSurface(UINT32 attachment, const GLSurfaceDesc &target)
	{
		assert(attachment < BS_MAX_MULTIPLE_RENDER_TARGETS);
		mColor[attachment] = target;
	}

	void GLFrameBufferObject::unbindSurface(UINT32 attachment)
	{
		assert(attachment < BS_MAX_MULTIPLE_RENDER_TARGETS);
		mColor[attachment].buffer = nullptr;
	}

	void GLFrameBufferObject::bindDepthStencil(SPtr<GLPixelBuffer> depthStencilBuffer, bool allLayers)
	{
		mDepthStencilBuffer = depthStencilBuffer;
		mDepthStencilAllLayers = allLayers;
	}

	void GLFrameBufferObject::unbindDepthStencil()
	{
		mDepthStencilBuffer = nullptr;
	}

	void GLFrameBufferObject::rebuild()
	{
		// Store basic stats
		UINT16 maxSupportedMRTs = RenderAPI::instancePtr()->getCapabilities(0).numMultiRenderTargets;

		// Bind simple buffer to add color attachments
		glBindFramebuffer(GL_FRAMEBUFFER, mFB);
		BS_CHECK_GL_ERROR();

		// Bind all attachment points to frame buffer
		for (UINT16 x = 0; x < maxSupportedMRTs; ++x)
		{
			if (mColor[x].buffer)
			{
				// Note: I'm attaching textures to FBO while renderbuffers might yield better performance if I
				// don't need to read from them

				mColor[x].buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0 + x, mColor[x].zoffset, mColor[x].allLayers);
			}
			else
			{
				// Detach
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, 0, 0);
				BS_CHECK_GL_ERROR();
			}
		}

		if (mDepthStencilBuffer != nullptr)
		{
			GLenum depthStencilFormat = GLPixelUtil::getDepthStencilFormatFromPF(mDepthStencilBuffer->getFormat());

			GLenum attachmentPoint;
			if (depthStencilFormat == GL_DEPTH_STENCIL)
				attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
			else // Depth only
				attachmentPoint = GL_DEPTH_ATTACHMENT;

			mDepthStencilBuffer->bindToFramebuffer(attachmentPoint, 0, mDepthStencilAllLayers);
		}

		// Do glDrawBuffer calls
		GLenum bufs[BS_MAX_MULTIPLE_RENDER_TARGETS];
		GLsizei n = 0;
		for (UINT32 x = 0; x < BS_MAX_MULTIPLE_RENDER_TARGETS; ++x)
		{
			// Fill attached colour buffers
			if (mColor[x].buffer)
			{
				bufs[x] = GL_COLOR_ATTACHMENT0 + x;
				// Keep highest used buffer + 1
				n = x + 1;
			}
			else
			{
				bufs[x] = GL_NONE;
			}
		}

		glDrawBuffers(n, bufs);
		BS_CHECK_GL_ERROR();

		// No read buffer, by default, if we want to read anyway we must not forget to set this.
		glReadBuffer(GL_NONE);
		BS_CHECK_GL_ERROR();

		// Check status
		GLuint status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		BS_CHECK_GL_ERROR();

		// Bind main buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		BS_CHECK_GL_ERROR();

		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LOGERR("All framebuffer formats with this texture internal format unsupported");
			break;
		default:
			LOGERR("Framebuffer incomplete or other FBO status error");
		}
	}

	void GLFrameBufferObject::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mFB);
		BS_CHECK_GL_ERROR();
	}
}}