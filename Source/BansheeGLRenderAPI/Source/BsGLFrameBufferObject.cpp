//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLFrameBufferObject.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "BsGLRenderTexture.h"
#include "BsRenderStats.h"

namespace BansheeEngine 
{
    GLFrameBufferObject::GLFrameBufferObject()
    {
        glGenFramebuffers(1, &mFB);

        for(UINT32 x = 0; x < BS_MAX_MULTIPLE_RENDER_TARGETS; ++x)
            mColor[x].buffer = nullptr;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_FrameBufferObject);
    }

    GLFrameBufferObject::~GLFrameBufferObject()
    {
        glDeleteFramebuffers(1, &mFB);    
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_FrameBufferObject);
    }

    void GLFrameBufferObject::bindSurface(UINT32 attachment, const GLSurfaceDesc &target)
    {
        assert(attachment < BS_MAX_MULTIPLE_RENDER_TARGETS);
        mColor[attachment] = target;

		if(mColor[0].buffer)
			rebuild();
    }

    void GLFrameBufferObject::unbindSurface(UINT32 attachment)
    {
        assert(attachment < BS_MAX_MULTIPLE_RENDER_TARGETS);
        mColor[attachment].buffer = nullptr;

		if(mColor[0].buffer)
			rebuild();
    }

	void GLFrameBufferObject::bindDepthStencil(SPtr<GLPixelBuffer> depthStencilBuffer)
	{
		mDepthStencilBuffer = depthStencilBuffer;

		if (mColor[0].buffer)
			rebuild();
	}

	void GLFrameBufferObject::unbindDepthStencil()
	{
		mDepthStencilBuffer = nullptr;
	}

    void GLFrameBufferObject::rebuild()
    {
        // First buffer must be bound
        if(!mColor[0].buffer)
			BS_EXCEPT(InvalidParametersException, "Attachment 0 must have surface attached");

        // Store basic stats
        UINT32 width = mColor[0].buffer->getWidth();
        UINT32 height = mColor[0].buffer->getHeight();
        UINT16 maxSupportedMRTs = RenderAPICore::instancePtr()->getCapabilities(0).getNumMultiRenderTargets();

		// Bind simple buffer to add color attachments
		glBindFramebuffer(GL_FRAMEBUFFER, mFB);

		bool bindAllLayers = false;

        // Bind all attachment points to frame buffer
        for(UINT16 x = 0; x < maxSupportedMRTs; ++x)
        {
            if(mColor[x].buffer)
            {
                if(mColor[x].buffer->getWidth() != width || mColor[x].buffer->getHeight() != height)
                {
                    StringStream ss;
                    ss << "Attachment " << x << " has incompatible size ";
                    ss << mColor[x].buffer->getWidth() << "x" << mColor[x].buffer->getHeight();
                    ss << ". It must be of the same as the size of surface 0, ";
                    ss << width << "x" << height;
                    ss << ".";
                    BS_EXCEPT(InvalidParametersException, ss.str());
                }

				// Note: I'm attaching textures to FBO while renderbuffers might yield better performance if I
				// don't need to read from them

	            mColor[x].buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0 + x, mColor[x].zoffset, mColor[x].allLayers);
				bindAllLayers |= mColor[x].allLayers;
            }
            else
            {
                // Detach
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, 0, 0);
            }
        }

		if(mDepthStencilBuffer != nullptr)
			mDepthStencilBuffer->bindToFramebuffer(GL_DEPTH_STENCIL_ATTACHMENT, 0, bindAllLayers);

		// Do glDrawBuffer calls
		GLenum bufs[BS_MAX_MULTIPLE_RENDER_TARGETS];
		GLsizei n = 0;
		for(UINT32 x = 0; x < BS_MAX_MULTIPLE_RENDER_TARGETS; ++x)
		{
			// Fill attached colour buffers
			if(mColor[x].buffer)
			{
				bufs[x] = GL_COLOR_ATTACHMENT0 + x;
				// Keep highest used buffer + 1
				n = x+1;
			}
			else
			{
				bufs[x] = GL_NONE;
			}
		}

		glDrawBuffers(n, bufs);

		// No read buffer, by default, if we want to read anyway we must not forget to set this.
		glReadBuffer(GL_NONE);

        // Check status
        GLuint status;
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        
        // Bind main buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        switch(status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            BS_EXCEPT(InvalidParametersException, "All framebuffer formats with this texture internal format unsupported");
        default:
            BS_EXCEPT(InvalidParametersException, "Framebuffer incomplete or other FBO status error");
        }
    }

    void GLFrameBufferObject::bind()
    {
		glBindFramebuffer(GL_FRAMEBUFFER, mFB);
    }
}
