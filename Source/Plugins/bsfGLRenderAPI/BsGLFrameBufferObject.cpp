//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLFrameBufferObject.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "BsGLRenderTexture.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

GLFrameBufferObject::GLFrameBufferObject()
{
	glGenFramebuffers(1, &mFB);
	B3D_CHECK_GL_ERROR();

	for(u32 x = 0; x < B3D_MAXIMUM_RENDER_TARGET_COUNT; ++x)
		mColor[x].Buffer = nullptr;

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_FrameBufferObject);
}

GLFrameBufferObject::~GLFrameBufferObject()
{
	glDeleteFramebuffers(1, &mFB);
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_FrameBufferObject);
}

void GLFrameBufferObject::BindSurface(u32 attachment, const GLSurfaceDesc &target)
{
	B3D_ASSERT(attachment < B3D_MAXIMUM_RENDER_TARGET_COUNT);
	mColor[attachment] = target;
}

void GLFrameBufferObject::UnbindSurface(u32 attachment)
{
	B3D_ASSERT(attachment < B3D_MAXIMUM_RENDER_TARGET_COUNT);
	mColor[attachment].Buffer = nullptr;
}

void GLFrameBufferObject::BindDepthStencil(SPtr<GLPixelBuffer> depthStencilBuffer, bool allLayers)
{
	mDepthStencilBuffer = depthStencilBuffer;
	mDepthStencilAllLayers = allLayers;
}

void GLFrameBufferObject::UnbindDepthStencil()
{
	mDepthStencilBuffer = nullptr;
}

void GLFrameBufferObject::Rebuild()
{
	// Store basic stats
	u16 maxSupportedMRTs = RenderAPI::InstancePtr()->GetCapabilities(0).NumMultiRenderTargets;

	// Bind simple buffer to add color attachments
	glBindFramebuffer(GL_FRAMEBUFFER, mFB);
	B3D_CHECK_GL_ERROR();

	// Bind all attachment points to frame buffer
	for(u16 x = 0; x < maxSupportedMRTs; ++x)
	{
		if(mColor[x].Buffer)
		{
			// Note: I'm attaching textures to FBO while renderbuffers might yield better performance if I
			// don't need to read from them

			mColor[x].Buffer->BindToFramebuffer(GL_COLOR_ATTACHMENT0 + x, mColor[x].Zoffset, mColor[x].AllLayers);
		}
		else
		{
			// Detach
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, 0, 0);
			B3D_CHECK_GL_ERROR();
		}
	}

	if(mDepthStencilBuffer != nullptr)
	{
		GLenum depthStencilFormat = GLPixelUtil::GetDepthStencilFormatFromPf(mDepthStencilBuffer->GetFormat());

		GLenum attachmentPoint;
		if(depthStencilFormat == GL_DEPTH_STENCIL)
			attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
		else // Depth only
			attachmentPoint = GL_DEPTH_ATTACHMENT;

		mDepthStencilBuffer->BindToFramebuffer(attachmentPoint, 0, mDepthStencilAllLayers);
	}

	// Do glDrawBuffer calls
	GLenum bufs[B3D_MAXIMUM_RENDER_TARGET_COUNT];
	GLsizei n = 0;
	for(u32 x = 0; x < B3D_MAXIMUM_RENDER_TARGET_COUNT; ++x)
	{
		// Fill attached colour buffers
		if(mColor[x].Buffer)
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
	B3D_CHECK_GL_ERROR();

	// No read buffer, by default, if we want to read anyway we must not forget to set this.
	glReadBuffer(GL_NONE);
	B3D_CHECK_GL_ERROR();

	// Check status
	GLuint status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	B3D_CHECK_GL_ERROR();

	// Bind main buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	B3D_CHECK_GL_ERROR();

	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		B3D_LOG(Error, RenderBackend, "All framebuffer formats with this texture internal format unsupported");
		break;
	default:
		B3D_LOG(Error, RenderBackend, "Framebuffer incomplete or other FBO status error");
	}
}

void GLFrameBufferObject::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFB);
	B3D_CHECK_GL_ERROR();
}
