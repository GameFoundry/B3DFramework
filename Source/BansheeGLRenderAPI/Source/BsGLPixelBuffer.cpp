//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLPixelBuffer.h"
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsGLPixelFormat.h"
#include "BsException.h"
#include "BsBitwise.h"
#include "BsGLRenderTexture.h"
#include "BsRenderStats.h"

namespace bs 
{
	GLPixelBuffer::GLPixelBuffer(UINT32 inWidth, UINT32 inHeight, UINT32 inDepth, PixelFormat inFormat, GpuBufferUsage usage)
		: mUsage(usage), mIsLocked(false), mWidth(inWidth), mHeight(inHeight), mDepth(inDepth), mFormat(inFormat)
		, mBuffer(inWidth, inHeight, inDepth, inFormat), mGLInternalFormat(GL_NONE)
	{
		mSizeInBytes = mHeight*mWidth*PixelUtil::getNumElemBytes(mFormat);
		mCurrentLockOptions = (GpuLockOptions)0;
	}

	GLPixelBuffer::~GLPixelBuffer()
	{
		mBuffer.freeInternalBuffer();
	}

	void GLPixelBuffer::allocateBuffer()
	{
		if(mBuffer.getData())
			return;

		mBuffer.allocateInternalBuffer();
		// TODO: use PBO if we're HBU_DYNAMIC
	}

	void GLPixelBuffer::freeBuffer()
	{
		if(mUsage & GBU_STATIC)
		{
			mBuffer.freeInternalBuffer();
		}
	}

	void* GLPixelBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		assert(!mIsLocked && "Cannot lock this buffer, it is already locked!");
		assert(offset == 0 && length == mSizeInBytes && "Cannot lock memory region, most lock box or entire buffer");

		PixelVolume volume(0, 0, 0, mWidth, mHeight, mDepth);
		const PixelData& lockedData = lock(volume, options);
		return lockedData.getData();
	}

	const PixelData& GLPixelBuffer::lock(const PixelVolume& lockBox, GpuLockOptions options)
	{
		allocateBuffer();

		if (options != GBL_WRITE_ONLY_DISCARD)
		{
			// Download the old contents of the texture
			download(mBuffer);
		}

		mCurrentLockOptions = options;
		mLockedBox = lockBox;

		mCurrentLock = mBuffer.getSubVolume(lockBox);
		mIsLocked = true;

		return mCurrentLock;
	}

	void GLPixelBuffer::unlock()
	{
		assert(mIsLocked && "Cannot unlock this buffer, it is not locked!");

		if (mCurrentLockOptions != GBL_READ_ONLY)
		{
			// From buffer to card, only upload if was locked for writing
			upload(mCurrentLock, mLockedBox);
		}

		freeBuffer();
		mIsLocked = false;
	}

	void GLPixelBuffer::upload(const PixelData& data, const PixelVolume& dest)
	{
		BS_EXCEPT(RenderingAPIException, "Upload not possible for this pixel buffer type");
	}

	void GLPixelBuffer::download(const PixelData& data)
	{
		BS_EXCEPT(RenderingAPIException, "Download not possible for this pixel buffer type");
	}

	void GLPixelBuffer::blitFromTexture(GLTextureBuffer* src)
	{
		blitFromTexture(src, 
			PixelVolume(0, 0, 0, src->getWidth(), src->getHeight(), src->getDepth()), 
			PixelVolume(0, 0, 0, mWidth, mHeight, mDepth)
			);
	}

	void GLPixelBuffer::blitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
	{
		BS_EXCEPT(RenderingAPIException, "BlitFromTexture not possible for this pixel buffer type");
	}

	void GLPixelBuffer::bindToFramebuffer(GLenum attachment, UINT32 zoffset, bool allLayers)
	{
		BS_EXCEPT(RenderingAPIException, "Framebuffer bind not possible for this pixel buffer type");
	}

	GLTextureBuffer::GLTextureBuffer(GLenum target, GLuint id, 
									 GLint face, GLint level, GpuBufferUsage usage, 
									 bool writeGamma, UINT32 multisampleCount):
		GLPixelBuffer(0, 0, 0, PF_UNKNOWN, usage),
		mTarget(target), mFaceTarget(0), mTextureID(id), mFace(face), mLevel(level), mMultisampleCount(multisampleCount)
	{
		GLint value = 0;
	
		glBindTexture(mTarget, mTextureID);
	
		// Get face identifier
		mFaceTarget = mTarget;
		if(mTarget == GL_TEXTURE_CUBE_MAP)
			mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (face % 6);
	
		// Get width
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_WIDTH, &value);
		mWidth = value;
	
		// Get height
		if(target == GL_TEXTURE_1D)
			value = 1;	// Height always 1 for 1D textures
		else
			glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_HEIGHT, &value);
		mHeight = value;
	
		// Get depth
		if(target != GL_TEXTURE_3D)
			value = 1; // Depth always 1 for non-3D textures
		else
			glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_DEPTH, &value);
		mDepth = value;

		// Get format
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &value);
		mGLInternalFormat = value;
		mFormat = GLPixelUtil::getClosestEngineFormat(value);
	
		// Default
		mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
	
		// Set up pixel box
		mBuffer = PixelData(mWidth, mHeight, mDepth, mFormat);
	}

	GLTextureBuffer::~GLTextureBuffer()
	{ }

	void GLTextureBuffer::upload(const PixelData &data, const PixelVolume &dest)
	{
		if((mUsage & TU_RENDERTARGET) != 0)
			BS_EXCEPT(NotImplementedException, "Writing to render texture from CPU not supported.");

		if ((mUsage & TU_DEPTHSTENCIL) != 0)
			BS_EXCEPT(NotImplementedException, "Writing to depth stencil texture from CPU not supported.");

		glBindTexture( mTarget, mTextureID );
		if(PixelUtil::isCompressed(data.getFormat()))
		{
			if(data.getFormat() != mFormat || !data.isConsecutive())
				BS_EXCEPT(InvalidParametersException, 
				"Compressed images must be consecutive, in the source format");

			GLenum format = GLPixelUtil::getClosestGLInternalFormat(mFormat);
			switch(mTarget) {
				case GL_TEXTURE_1D:
					if (dest.left == 0)
					{
						glCompressedTexImage1D(GL_TEXTURE_1D, mLevel,
							format,
							dest.getWidth(),
							0,
							data.getConsecutiveSize(),
							data.getData());
					}
					else
					{
						glCompressedTexSubImage1D(GL_TEXTURE_1D, mLevel, 
							dest.left,
							dest.getWidth(),
							format, data.getConsecutiveSize(),
							data.getData());
					}
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					if (dest.left == 0 && dest.top == 0)
					{
						glCompressedTexImage2D(mFaceTarget, mLevel,
							format,
							dest.getWidth(),
							dest.getHeight(),
							0,
							data.getConsecutiveSize(),
							data.getData());
					}
					else
					{
						glCompressedTexSubImage2D(mFaceTarget, mLevel, 
							dest.left, dest.top, 
							dest.getWidth(), dest.getHeight(),
							format, data.getConsecutiveSize(),
							data.getData());
					}
					break;
				case GL_TEXTURE_3D:
					if (dest.left == 0 && dest.top == 0 && dest.front == 0)
					{
						glCompressedTexImage3D(GL_TEXTURE_3D, mLevel,
							format,
							dest.getWidth(),
							dest.getHeight(),
							dest.getDepth(),
							0,
							data.getConsecutiveSize(),
							data.getData());
					}
					else
					{			
						glCompressedTexSubImage3D(GL_TEXTURE_3D, mLevel, 
							dest.left, dest.top, dest.front,
							dest.getWidth(), dest.getHeight(), dest.getDepth(),
							format, data.getConsecutiveSize(),
							data.getData());
					}
					break;
			}
		
		} 
		else
		{
			if(data.getWidth() != data.getRowPitch())
				glPixelStorei(GL_UNPACK_ROW_LENGTH, data.getRowPitch());

			if(data.getHeight()*data.getWidth() != data.getSlicePitch())
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.getSlicePitch()/data.getWidth()));

			if(data.getLeft() > 0 || data.getTop() > 0 || data.getFront() > 0)
				glPixelStorei(GL_UNPACK_SKIP_PIXELS, data.getLeft() + data.getRowPitch() * data.getTop() + data.getSlicePitch() * data.getFront());

			if((data.getWidth()*PixelUtil::getNumElemBytes(data.getFormat())) & 3)
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			switch(mTarget) {
				case GL_TEXTURE_1D:
					glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
						dest.left,
						dest.getWidth(),
						GLPixelUtil::getGLOriginFormat(data.getFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.getData());
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					glTexSubImage2D(mFaceTarget, mLevel, 
						dest.left, dest.top, 
						dest.getWidth(), dest.getHeight(),
						GLPixelUtil::getGLOriginFormat(data.getFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.getData());
					break;
				case GL_TEXTURE_3D:
					glTexSubImage3D(
						GL_TEXTURE_3D, mLevel, 
						dest.left, dest.top, dest.front,
						dest.getWidth(), dest.getHeight(), dest.getDepth(),
						GLPixelUtil::getGLOriginFormat(data.getFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.getData());
					break;
			}	
		}

		// Restore defaults
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
	}

	void GLTextureBuffer::download(const PixelData &data)
	{
		if(data.getWidth() != getWidth() || data.getHeight() != getHeight() || data.getDepth() != getDepth())
			BS_EXCEPT(InvalidParametersException, "only download of entire buffer is supported by GL");

		glBindTexture( mTarget, mTextureID );
		if(PixelUtil::isCompressed(data.getFormat()))
		{
			if(data.getFormat() != mFormat || !data.isConsecutive())
				BS_EXCEPT(InvalidParametersException, 
				"Compressed images must be consecutive, in the source format");

			// Data must be consecutive and at beginning of buffer as PixelStorei not allowed
			// for compressed formate
			glGetCompressedTexImage(mFaceTarget, mLevel, data.getData());
		} 
		else
		{
			if(data.getWidth() != data.getRowPitch())
				glPixelStorei(GL_PACK_ROW_LENGTH, data.getRowPitch());

			if(data.getHeight()*data.getWidth() != data.getSlicePitch())
				glPixelStorei(GL_PACK_IMAGE_HEIGHT, (data.getSlicePitch()/data.getWidth()));

			if(data.getLeft() > 0 || data.getTop() > 0 || data.getFront() > 0)
				glPixelStorei(GL_PACK_SKIP_PIXELS, data.getLeft() + data.getRowPitch() * data.getTop() + data.getSlicePitch() * data.getFront());

			if((data.getWidth()*PixelUtil::getNumElemBytes(data.getFormat())) & 3)
				glPixelStorei(GL_PACK_ALIGNMENT, 1);

			// We can only get the entire texture
			glGetTexImage(mFaceTarget, mLevel, GLPixelUtil::getGLOriginFormat(data.getFormat()), 
				GLPixelUtil::getGLOriginDataType(data.getFormat()), data.getData());

			// Restore defaults
			glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
			glPixelStorei(GL_PACK_ALIGNMENT, 4);
		}

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_Texture);
	}

	void GLTextureBuffer::bindToFramebuffer(GLenum attachment, UINT32 zoffset, bool allLayers)
	{
		if(allLayers)
		{
			switch (mTarget)
			{
			case GL_TEXTURE_1D:
				glFramebufferTexture1D(GL_FRAMEBUFFER, attachment,
					mFaceTarget, mTextureID, mLevel);
				break;
			case GL_TEXTURE_2D:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
					mFaceTarget, mTextureID, mLevel);
				break;
			case GL_TEXTURE_2D_MULTISAMPLE:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
					mFaceTarget, mTextureID, 0);
				break;
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_3D:
			default: // Texture arrays (binding all layers)
				glFramebufferTexture(GL_FRAMEBUFFER, attachment, mTextureID, mLevel);
				break;
			}
		}
		else
		{
			switch (mTarget)
			{
			case GL_TEXTURE_3D:
				glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, mFaceTarget, mTextureID, mLevel, zoffset);
				break;
			default: // Texture arrays and cube maps
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, mTextureID, mLevel, mFace);
				break;
			}
		}
	}

	void GLTextureBuffer::copyFromFramebuffer(UINT32 zoffset)
	{
		glBindTexture(mTarget, mTextureID);
		switch(mTarget)
		{
		case GL_TEXTURE_1D:
			glCopyTexSubImage1D(mFaceTarget, mLevel, 0, 0, 0, mWidth);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			glCopyTexSubImage2D(mFaceTarget, mLevel, 0, 0, 0, 0, mWidth, mHeight);
			break;
		case GL_TEXTURE_3D:
			glCopyTexSubImage3D(mFaceTarget, mLevel, 0, 0, zoffset, 0, 0, mWidth, mHeight);
			break;
		}
	}

	void GLTextureBuffer::blitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
	{
		if (src->mMultisampleCount > 0 && mMultisampleCount == 0) // Resolving MS texture
		{
			if (mTarget != GL_TEXTURE_2D || mTarget != GL_TEXTURE_2D_MULTISAMPLE)
				BS_EXCEPT(InvalidParametersException, "Non-2D multisampled texture not supported.");

			GLint currentFBO = 0;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

			GLuint readFBO = GLRTTManager::instance().getBlitReadFBO();
			GLuint drawFBO = GLRTTManager::instance().getBlitDrawFBO();

			// Attach source texture
			glBindFramebuffer(GL_FRAMEBUFFER, readFBO);
			src->bindToFramebuffer(0, 0, true);

			// Attach destination texture
			glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
			bindToFramebuffer(0, 0, true);

			// Perform blit
			glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);

			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer(srcBox.left, srcBox.top, srcBox.right, srcBox.bottom, 
				dstBox.left, dstBox.top, dstBox.right, dstBox.bottom, GL_COLOR_BUFFER_BIT, GL_NEAREST);

			// Restore the previously bound FBO
			glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
		}
		else // Just plain copy
		{
			if (mMultisampleCount != src->mMultisampleCount)
				BS_EXCEPT(InvalidParametersException, "When copying textures their multisample counts must match.");

			if (mTarget == GL_TEXTURE_3D) // 3D textures can't have arrays so their Z coordinate is handled differently
			{
				glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.left, srcBox.top, srcBox.front,
					mTextureID, mTarget, mLevel, dstBox.left, dstBox.top, dstBox.front, srcBox.getWidth(), srcBox.getHeight(), srcBox.getDepth());
			}
			else
			{
				glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.left, srcBox.top, src->mFace,
					mTextureID, mTarget, mLevel, dstBox.left, dstBox.top, mFace, srcBox.getWidth(), srcBox.getHeight(), 1);
			}
		}		
	}
	
	GLRenderBuffer::GLRenderBuffer(GLenum format, UINT32 width, UINT32 height, GLsizei numSamples):
		GLPixelBuffer(width, height, 1, GLPixelUtil::getClosestEngineFormat(format), GBU_DYNAMIC),
		mRenderbufferID(0)
	{
		mGLInternalFormat = format;
		glGenRenderbuffers(1, &mRenderbufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
    
		/// Allocate storage for depth buffer
		if (numSamples > 0)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
				numSamples, format, width, height);
		}
		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, format,
								width, height);
		}
	}

	GLRenderBuffer::~GLRenderBuffer()
	{
		glDeleteRenderbuffers(1, &mRenderbufferID);
	}

	void GLRenderBuffer::bindToFramebuffer(GLenum attachment, UINT32 zoffset, bool allLayers)
	{
		assert(zoffset < mDepth);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
							GL_RENDERBUFFER, mRenderbufferID);
	}
};
