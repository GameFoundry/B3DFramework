//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLRenderTexture.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "BsTextureView.h"

namespace BansheeEngine 
{
#define PROBE_SIZE 16

	static const GLenum depthFormats[] =
	{
		GL_NONE,
		GL_DEPTH_COMPONENT16,
		GL_DEPTH_COMPONENT32,
		GL_DEPTH24_STENCIL8,
		GL_DEPTH32F_STENCIL8
	};

#define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))

	GLRenderTextureCore::GLRenderTextureCore(const RENDER_TEXTURE_DESC_CORE& desc, GpuDeviceFlags deviceMask)
		:RenderTextureCore(desc, deviceMask), mProperties(desc, true), mFB(nullptr)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLRenderTextureCore::~GLRenderTextureCore()
	{ 
		if (mFB != nullptr)
			bs_delete(mFB);
	}

	void GLRenderTextureCore::initialize()
	{
		RenderTextureCore::initialize();

		if (mFB != nullptr)
			bs_delete(mFB);

		mFB = bs_new<GLFrameBufferObject>();

		for (size_t i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			if (mColorSurfaces[i] != nullptr)
			{
				GLTextureCore* glColorSurface = static_cast<GLTextureCore*>(mColorSurfaces[i]->getTexture().get());
				GLSurfaceDesc surfaceDesc;
				surfaceDesc.numSamples = getProperties().getMultisampleCount();

				if (mColorSurfaces[i]->getNumArraySlices() == 1) // Binding a single texture layer
				{
					surfaceDesc.allLayers = glColorSurface->getProperties().getNumFaces() == 1;

					if (glColorSurface->getProperties().getTextureType() != TEX_TYPE_3D)
					{
						surfaceDesc.zoffset = 0;
						surfaceDesc.buffer = glColorSurface->getBuffer(mColorSurfaces[i]->getFirstArraySlice(),
							mColorSurfaces[i]->getMostDetailedMip());
					}
					else
					{
						surfaceDesc.zoffset = mColorSurfaces[i]->getFirstArraySlice();
						surfaceDesc.buffer = glColorSurface->getBuffer(0, mColorSurfaces[i]->getMostDetailedMip());
					}
				}
				else // Binding an array of textures or a range of 3D texture slices
				{
					surfaceDesc.allLayers = true;

					if (glColorSurface->getProperties().getTextureType() != TEX_TYPE_3D)
					{
						if (mColorSurfaces[i]->getNumArraySlices() != glColorSurface->getProperties().getNumFaces())
							LOGWRN("OpenGL doesn't support binding of arbitrary ranges for array textures. The entire range will be bound instead.");

						surfaceDesc.zoffset = 0;
						surfaceDesc.buffer = glColorSurface->getBuffer(0, mColorSurfaces[i]->getMostDetailedMip());
					}
					else
					{
						if (mColorSurfaces[i]->getNumArraySlices() != glColorSurface->getProperties().getDepth())
							LOGWRN("OpenGL doesn't support binding of arbitrary ranges for array textures. The entire range will be bound instead.");

						surfaceDesc.zoffset = 0;
						surfaceDesc.buffer = glColorSurface->getBuffer(0, mColorSurfaces[i]->getMostDetailedMip());
					}
				}

				mFB->bindSurface((UINT32)i, surfaceDesc);
			}
			else
			{
				mFB->unbindSurface((UINT32)i);
			}
		}

		if (mDepthStencilSurface != nullptr && mDepthStencilSurface->getTexture() != nullptr)
		{
			GLTextureCore* glDepthStencilTexture = static_cast<GLTextureCore*>(mDepthStencilSurface->getTexture().get());
			SPtr<GLPixelBuffer> depthStencilBuffer = nullptr;

			if (glDepthStencilTexture->getProperties().getTextureType() != TEX_TYPE_3D)
			{
				depthStencilBuffer = glDepthStencilTexture->getBuffer(mDepthStencilSurface->getFirstArraySlice(),
					mDepthStencilSurface->getMostDetailedMip());
			}

			mFB->bindDepthStencil(depthStencilBuffer);
		}
	}

	void GLRenderTextureCore::getCustomAttribute(const String& name, void* data) const
	{
		if(name=="FBO")
		{
			*static_cast<GLFrameBufferObject**>(data) = mFB;
		}
		else if (name == "GL_FBOID" || name == "GL_MULTISAMPLEFBOID")
		{
			*static_cast<GLuint*>(data) = mFB->getGLFBOID();
		}
	}

	GLRTTManager::GLRTTManager()
		:mBlitReadFBO(0), mBlitWriteFBO(0)
    {
		detectFBOFormats();
		
		glGenFramebuffers(1, &mBlitReadFBO);
		glGenFramebuffers(1, &mBlitWriteFBO);
    }

	GLRTTManager::~GLRTTManager()
	{
		glDeleteFramebuffers(1, &mBlitReadFBO);
		glDeleteFramebuffers(1, &mBlitWriteFBO);
	}

	bool GLRTTManager::_tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status, depthRB = 0, stencilRB = 0;
        bool failed = false;

        if(depthFormat != GL_NONE)
        {
            // Generate depth renderbuffer
            glGenRenderbuffers(1, &depthRB);

            // Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, depthRB);
            
            // Allocate storage for depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat,
                                PROBE_SIZE, PROBE_SIZE);
            
            // Attach depth
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                    GL_RENDERBUFFER, depthRB);
        }

        if(stencilFormat != GL_NONE)
        {
            // Generate stencil renderbuffer
            glGenRenderbuffers(1, &stencilRB);

            // Bind it to FBO
            glBindRenderbuffer(GL_RENDERBUFFER, stencilRB);
            glGetError(); 

            // Allocate storage for stencil buffer
            glRenderbufferStorage(GL_RENDERBUFFER, stencilFormat,
                                PROBE_SIZE, PROBE_SIZE); 

            if(glGetError() != GL_NO_ERROR)
                failed = true;

            // Attach stencil
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, stencilRB);

            if(glGetError() != GL_NO_ERROR)
                failed = true;
        }
        
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        // Detach and destroy
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

        if (depthRB)
            glDeleteRenderbuffers(1, &depthRB);

        if (stencilRB)
            glDeleteRenderbuffers(1, &stencilRB);
        
        return status == GL_FRAMEBUFFER_COMPLETE && !failed;
    }
    
    bool GLRTTManager::_tryPackedFormat(GLenum packedFormat)
    {
        GLuint packedRB = 0;
        bool failed = false; // flag on GL errors

        // Generate renderbuffer
        glGenRenderbuffers(1, &packedRB);

        // Bind it to FBO
        glBindRenderbuffer(GL_RENDERBUFFER, packedRB);

        // Allocate storage for buffer
        glRenderbufferStorage(GL_RENDERBUFFER, packedFormat, PROBE_SIZE, PROBE_SIZE);
        glGetError();

        // Attach depth
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        if(glGetError() != GL_NO_ERROR)
            failed = true;

        // Attach stencil
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, packedRB);

        if(glGetError() != GL_NO_ERROR)
            failed = true;

        GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        // Detach and destroy
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &packedRB);

        return status == GL_FRAMEBUFFER_COMPLETE && !failed;
    }

    void GLRTTManager::detectFBOFormats()
    {
        // Try all formats, and report which ones work as target
        GLuint fb = 0, tid = 0;
        GLint old_drawbuffer = 0, old_readbuffer = 0;
        GLenum target = GL_TEXTURE_2D;

        glGetIntegerv (GL_DRAW_BUFFER, &old_drawbuffer);
        glGetIntegerv (GL_READ_BUFFER, &old_readbuffer);

        for(size_t x=0; x<PF_COUNT; ++x)
        {
            mProps[x].valid = false;

			// Fetch GL format token
			GLenum fmt = GLPixelUtil::getGLInternalFormat((PixelFormat)x);
            if(fmt == GL_NONE && x!=0)
                continue;

			// No test for compressed formats
			if(PixelUtil::isCompressed((PixelFormat)x))
				continue;

            // Create and attach framebuffer
            glGenFramebuffers(1, &fb);
            glBindFramebuffer(GL_FRAMEBUFFER, fb);
            if (fmt!=GL_NONE && !PixelUtil::isDepth((PixelFormat)x))
            {
				// Create and attach texture
				glGenTextures(1, &tid);
				glBindTexture(target, tid);
				
                // Set some default parameters so it won't fail on NVidia cards         
				if (GLEW_VERSION_1_2)
					glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                            
				glTexImage2D(target, 0, fmt, PROBE_SIZE, PROBE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tid, 0);
            }
			else
			{
				// Draw to nowhere -- stencil/depth only
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

            // Check status
            GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			// Ignore status in case of fmt==GL_NONE, because no implementation will accept
			// a buffer without *any* attachment. Buffers with only stencil and depth attachment
			// might still be supported, so we must continue probing.
            if(fmt == GL_NONE || status == GL_FRAMEBUFFER_COMPLETE)
            {
                mProps[x].valid = true;

                // For each depth/stencil formats
                for (UINT32 depth = 0; depth < DEPTHFORMAT_COUNT; ++depth)
                {
                    if (depthFormats[depth] != GL_DEPTH24_STENCIL8 && depthFormats[depth] != GL_DEPTH32F_STENCIL8)
                    {
                        if (_tryFormat(depthFormats[depth], GL_NONE))
                        {
                            /// Add mode to allowed modes
                            FormatProperties::Mode mode;
                            mode.depth = depth;
                            mode.stencil = 0;
                            mProps[x].modes.push_back(mode);
                        }
                    }
                    else
                    {
                        // Packed depth/stencil format
                        if (_tryPackedFormat(depthFormats[depth]))
                        {
                            /// Add mode to allowed modes
                            FormatProperties::Mode mode;
                            mode.depth = depth;
                            mode.stencil = 0;   // unuse
                            mProps[x].modes.push_back(mode);
                        }
                    }
                }
            }

            // Delete texture and framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fb);
			
			glFinish();
			
            if (fmt != GL_NONE)
                glDeleteTextures(1, &tid);
        }

        // It seems a bug in nVidia driver: glBindFramebuffer should restore
        // draw and read buffers, but in some unclear circumstances it won't.
        glDrawBuffer(old_drawbuffer);
        glReadBuffer(old_readbuffer);

		String fmtstring = "";
        for(size_t x = 0; x < PF_COUNT; ++x)
        {
            if(mProps[x].valid)
                fmtstring += PixelUtil::getFormatName((PixelFormat)x)+" ";
        }
    }
    
    PixelFormat GLRTTManager::getSupportedAlternative(PixelFormat format)
    {
        if(checkFormat(format))
            return format;

        // Find first alternative
        PixelComponentType pct = PixelUtil::getElementType(format);
        switch(pct)
        {
        case PCT_BYTE: format = PF_A8R8G8B8; break;
        case PCT_FLOAT16: format = PF_FLOAT16_RGBA; break;
        case PCT_FLOAT32: format = PF_FLOAT32_RGBA; break;
        default: break;
        }

        if(checkFormat(format))
            return format;

        // If none at all, return to default
        return PF_A8R8G8B8;
    }

	GLRenderTexture::GLRenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, true)
	{

	}
}

