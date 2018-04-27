//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLPixelBuffer.h"
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsGLPixelFormat.h"
#include "Error/BsException.h"
#include "Utility/BsBitwise.h"
#include "BsGLRenderTexture.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	GLPixelBuffer::GLPixelBuffer(UINT32 inWidth, UINT32 inHeight, UINT32 inDepth, PixelFormat inFormat, GpuBufferUsage usage)
		: mUsage(usage), mIsLocked(false), mWidth(inWidth), mHeight(inHeight), mDepth(inDepth), mFormat(inFormat)
		, mBuffer(inWidth, inHeight, inDepth, inFormat)
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
			mBuffer.freeInternalBuffer();
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

	GLTextureBuffer::GLTextureBuffer(
		GLenum target,
		GLuint id,
		GLint face,
		GLint level,
		GpuBufferUsage usage,
		PixelFormat format,
		UINT32 multisampleCount)
		: GLPixelBuffer(0, 0, 0, format, usage), mTarget(target), mFaceTarget(0), mTextureID(id), mFace(face)
		, mLevel(level), mMultisampleCount(multisampleCount)
	{
		GLint value = 0;
	
		glBindTexture(mTarget, mTextureID);
		BS_CHECK_GL_ERROR();
	
		// Get face identifier
		mFaceTarget = mTarget;
		if(mTarget == GL_TEXTURE_CUBE_MAP)
			mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (face % 6);
	
		// Get width
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_WIDTH, &value);
		BS_CHECK_GL_ERROR();

		mWidth = value;
	
		// Get height
		if(target == GL_TEXTURE_1D)
			value = 1;	// Height always 1 for 1D textures
		else
		{
			glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_HEIGHT, &value);
			BS_CHECK_GL_ERROR();
		}

		mHeight = value;
	
		// Get depth
		if(target != GL_TEXTURE_3D)
			value = 1; // Depth always 1 for non-3D textures
		else
		{
			glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_DEPTH, &value);
			BS_CHECK_GL_ERROR();
		}

		mDepth = value;

		// Default
		mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
	
		// Set up pixel box
		mBuffer = PixelData(mWidth, mHeight, mDepth, mFormat);
	}

	GLTextureBuffer::~GLTextureBuffer()
	{ }

	void GLTextureBuffer::upload(const PixelData& data, const PixelVolume& dest)
	{
		if ((mUsage & TU_RENDERTARGET) != 0)
		{
			LOGERR("Writing to render texture from CPU not supported.");
			return;
		}

		if ((mUsage & TU_DEPTHSTENCIL) != 0)
		{
			LOGERR("Writing to depth stencil texture from CPU not supported.");
			return;
		}

		glBindTexture(mTarget, mTextureID);
		BS_CHECK_GL_ERROR();

		if(PixelUtil::isCompressed(data.getFormat()))
		{
			if (data.getFormat() != mFormat || !data.isConsecutive())
			{
				LOGERR("Compressed images must be consecutive, in the source format");
				return;
			}

			GLenum format = GLPixelUtil::getGLInternalFormat(mFormat);
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
						BS_CHECK_GL_ERROR();
					}
					else
					{
						glCompressedTexSubImage1D(GL_TEXTURE_1D, mLevel, 
							dest.left,
							dest.getWidth(),
							format, data.getConsecutiveSize(),
							data.getData());
						BS_CHECK_GL_ERROR();
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
						BS_CHECK_GL_ERROR();
					}
					else
					{
						glCompressedTexSubImage2D(mFaceTarget, mLevel, 
							dest.left, dest.top, 
							dest.getWidth(), dest.getHeight(),
							format, data.getConsecutiveSize(),
							data.getData());
						BS_CHECK_GL_ERROR();
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
						BS_CHECK_GL_ERROR();
					}
					else
					{			
						glCompressedTexSubImage3D(GL_TEXTURE_3D, mLevel, 
							dest.left, dest.top, dest.front,
							dest.getWidth(), dest.getHeight(), dest.getDepth(),
							format, data.getConsecutiveSize(),
							data.getData());
						BS_CHECK_GL_ERROR();
					}
					break;
			}
		
		} 
		else
		{
			if (data.getWidth() != data.getRowPitch())
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, data.getRowPitch());
				BS_CHECK_GL_ERROR();
			}

			if (data.getHeight()*data.getWidth() != data.getSlicePitch())
			{
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.getSlicePitch() / data.getWidth()));
				BS_CHECK_GL_ERROR();
			}

			if (data.getLeft() > 0 || data.getTop() > 0 || data.getFront() > 0)
			{
				glPixelStorei(
					GL_UNPACK_SKIP_PIXELS, 
					data.getLeft() + data.getRowPitch() * data.getTop() + data.getSlicePitch() * data.getFront());
				BS_CHECK_GL_ERROR();
			}

			if ((data.getWidth()*PixelUtil::getNumElemBytes(data.getFormat())) & 3)
			{
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				BS_CHECK_GL_ERROR();
			}

			switch(mTarget) {
				case GL_TEXTURE_1D:
					glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
						dest.left,
						dest.getWidth(),
						GLPixelUtil::getGLOriginFormat(data.getFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.getData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					glTexSubImage2D(mFaceTarget, mLevel, 
						dest.left, dest.top, 
						dest.getWidth(), dest.getHeight(),
						GLPixelUtil::getGLOriginFormat(data.getFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.getData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D_ARRAY:
				case GL_TEXTURE_3D:
					glTexSubImage3D(
						mTarget, mLevel,
						dest.left, dest.top, dest.front,
						dest.getWidth(), dest.getHeight(), dest.getDepth(),
						GLPixelUtil::getGLOriginFormat(data.getFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.getData());
					BS_CHECK_GL_ERROR();
					break;
			}	
		}

		// Restore defaults
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		BS_CHECK_GL_ERROR();

		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
		BS_CHECK_GL_ERROR();

		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		BS_CHECK_GL_ERROR();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
	}

	void GLTextureBuffer::download(const PixelData &data)
	{
		if (data.getWidth() != getWidth() || data.getHeight() != getHeight() || data.getDepth() != getDepth())
		{
			LOGERR("Only download of entire buffer is supported by OpenGL.");
			return;
		}

		glBindTexture(mTarget, mTextureID);
		BS_CHECK_GL_ERROR();

		if(PixelUtil::isCompressed(data.getFormat()))
		{
			if (data.getFormat() != mFormat || !data.isConsecutive())
			{
				LOGERR("Compressed images must be consecutive, in the source format");
				return;
			}

			// Data must be consecutive and at beginning of buffer as PixelStorei not allowed
			// for compressed formate
			glGetCompressedTexImage(mFaceTarget, mLevel, data.getData());
			BS_CHECK_GL_ERROR();
		} 
		else
		{
			if (data.getWidth() != data.getRowPitch())
			{
				glPixelStorei(GL_PACK_ROW_LENGTH, data.getRowPitch());
				BS_CHECK_GL_ERROR();
			}

			if (data.getHeight()*data.getWidth() != data.getSlicePitch())
			{
				glPixelStorei(GL_PACK_IMAGE_HEIGHT, (data.getSlicePitch() / data.getWidth()));
				BS_CHECK_GL_ERROR();
			}

			if (data.getLeft() > 0 || data.getTop() > 0 || data.getFront() > 0)
			{
				glPixelStorei(
					GL_PACK_SKIP_PIXELS, 
					data.getLeft() + data.getRowPitch() * data.getTop() + data.getSlicePitch() * data.getFront());
				BS_CHECK_GL_ERROR();
			}

			if ((data.getWidth()*PixelUtil::getNumElemBytes(data.getFormat())) & 3)
			{
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				BS_CHECK_GL_ERROR();
			}

			// We can only get the entire texture
			glGetTexImage(mFaceTarget, mLevel, GLPixelUtil::getGLOriginFormat(data.getFormat()), 
				GLPixelUtil::getGLOriginDataType(data.getFormat()), data.getData());
			BS_CHECK_GL_ERROR();

			// Restore defaults
			glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			BS_CHECK_GL_ERROR();

			glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
			BS_CHECK_GL_ERROR();

			glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
			BS_CHECK_GL_ERROR();

			glPixelStorei(GL_PACK_ALIGNMENT, 4);
			BS_CHECK_GL_ERROR();
		}

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_Texture);
	}

	void GLTextureBuffer::bindToFramebuffer(GLenum attachment, UINT32 zoffset, bool allLayers)
	{
		if(mTarget == GL_TEXTURE_1D || mTarget == GL_TEXTURE_2D)
			allLayers = true;

		if(allLayers)
		{
			switch (mTarget)
			{
			case GL_TEXTURE_1D:
				glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, mFaceTarget, mTextureID, mLevel);
				BS_CHECK_GL_ERROR();
				break;
			case GL_TEXTURE_2D:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, mFaceTarget, mTextureID, mLevel);
				BS_CHECK_GL_ERROR();
				break;
			case GL_TEXTURE_2D_MULTISAMPLE:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, mFaceTarget, mTextureID, 0);
				BS_CHECK_GL_ERROR();
				break;
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_3D:
			default: // Texture arrays (binding all layers)
				glFramebufferTexture(GL_FRAMEBUFFER, attachment, mTextureID, mLevel);
				BS_CHECK_GL_ERROR();
				break;
			}
		}
		else
		{
			switch (mTarget)
			{
			case GL_TEXTURE_3D:
				glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, mFaceTarget, mTextureID, mLevel, zoffset);
				BS_CHECK_GL_ERROR();
				break;
			case GL_TEXTURE_CUBE_MAP:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, mFaceTarget, mTextureID, mLevel);
				BS_CHECK_GL_ERROR();
				break;
			default: // Texture arrays
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, mTextureID, mLevel, mFace);
				BS_CHECK_GL_ERROR();
				break;
			}
		}
	}

	void GLTextureBuffer::copyFromFramebuffer(UINT32 zoffset)
	{
		glBindTexture(mTarget, mTextureID);
		BS_CHECK_GL_ERROR();

		switch(mTarget)
		{
		case GL_TEXTURE_1D:
			glCopyTexSubImage1D(mFaceTarget, mLevel, 0, 0, 0, mWidth);
			BS_CHECK_GL_ERROR();
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			glCopyTexSubImage2D(mFaceTarget, mLevel, 0, 0, 0, 0, mWidth, mHeight);
			BS_CHECK_GL_ERROR();
			break;
		case GL_TEXTURE_3D:
			glCopyTexSubImage3D(mFaceTarget, mLevel, 0, 0, zoffset, 0, 0, mWidth, mHeight);
			BS_CHECK_GL_ERROR();
			break;
		}
	}

	void GLTextureBuffer::blitFromTexture(GLTextureBuffer* src)
	{
		GLPixelBuffer::blitFromTexture(src);
	}

	void GLTextureBuffer::blitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
	{
		// If supported, prefer direct image copy. If not supported, or if sample counts don't match, fall back to FB blit
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
		if (src->mMultisampleCount > 1 && mMultisampleCount <= 1) // Resolving MS texture
#endif
		{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
			if (mTarget != GL_TEXTURE_2D || mTarget != GL_TEXTURE_2D_MULTISAMPLE)
				BS_EXCEPT(InvalidParametersException, "Non-2D multisampled texture not supported.");
#endif

			GLint currentFBO = 0;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
			BS_CHECK_GL_ERROR();

			GLuint readFBO = GLRTTManager::instance().getBlitReadFBO();
			GLuint drawFBO = GLRTTManager::instance().getBlitDrawFBO();

			// Attach source texture
			glBindFramebuffer(GL_FRAMEBUFFER, readFBO);
			BS_CHECK_GL_ERROR();

			src->bindToFramebuffer(GL_COLOR_ATTACHMENT0, 0, false);

			// Attach destination texture
			glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
			BS_CHECK_GL_ERROR();

			bindToFramebuffer(GL_COLOR_ATTACHMENT0, 0, false);

			// Perform blit
			glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
			BS_CHECK_GL_ERROR();

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
			BS_CHECK_GL_ERROR();

			glReadBuffer(GL_COLOR_ATTACHMENT0);
			BS_CHECK_GL_ERROR();

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			BS_CHECK_GL_ERROR();

			glBlitFramebuffer(srcBox.left, srcBox.top, srcBox.right, srcBox.bottom, 
				dstBox.left, dstBox.top, dstBox.right, dstBox.bottom, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			BS_CHECK_GL_ERROR();

			// Restore the previously bound FBO
			glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
			BS_CHECK_GL_ERROR();
		}
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
		else // Just plain copy
		{
			if (mMultisampleCount != src->mMultisampleCount)
				BS_EXCEPT(InvalidParametersException, "When copying textures their multisample counts must match.");

			if (mTarget == GL_TEXTURE_3D) // 3D textures can't have arrays so their Z coordinate is handled differently
			{
				glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.left, srcBox.top, srcBox.front,
					mTextureID, mTarget, mLevel, dstBox.left, dstBox.top, dstBox.front, srcBox.getWidth(), srcBox.getHeight(), srcBox.getDepth());
				BS_CHECK_GL_ERROR();
			}
			else
			{
				glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.left, srcBox.top, src->mFace,
					mTextureID, mTarget, mLevel, dstBox.left, dstBox.top, mFace, srcBox.getWidth(), srcBox.getHeight(), 1);
				BS_CHECK_GL_ERROR();
			}
		}		
#endif
	}
}}
