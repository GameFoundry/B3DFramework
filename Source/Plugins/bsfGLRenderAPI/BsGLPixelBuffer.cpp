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
#include "Math/BsMath.h"

namespace bs { namespace ct
{
	GLPixelBuffer::GLPixelBuffer(UINT32 inWidth, UINT32 inHeight, UINT32 inDepth, PixelFormat inFormat, GpuBufferUsage usage)
		: MUsage(usage), mWidth(inWidth), mHeight(inHeight), mDepth(inDepth), mFormat(inFormat)
		, MBuffer(inWidth, inHeight, inDepth, inFormat)
	{
		mSizeInBytes = mHeight*mWidth*PixelUtil::getNumElemBytes(mFormat);
		mCurrentLockOptions = (GpuLockOptions)0;
	}

	GLPixelBuffer::~GLPixelBuffer()
	{
		mBuffer.FreeInternalBuffer();
	}

	void GLPixelBuffer::AllocateBuffer()
	{
		if(mBuffer.GetData())
			return;

		mBuffer.AllocateInternalBuffer();
		// TODO: use PBO if we're HBU_DYNAMIC
	}

	void GLPixelBuffer::FreeBuffer()
	{
		if(mUsage & GBU_STATIC)
			mBuffer.FreeInternalBuffer();
	}

	void* GLPixelBuffer::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		assert(!mIsLocked && "Cannot lock this buffer, it is already locked!");
		assert(offset == 0 && length == mSizeInBytes && "Cannot lock memory region, most lock box or entire buffer");

		PixelVolume Volume(0, 0, 0, mWidth, mHeight, mDepth);
		const PixelData& lockedData = lock(volume, options);
		return lockedData.GetData();
	}

	const PixelData& GLPixelBuffer::Lock(const PixelVolume& lockBox, GpuLockOptions options)
	{
		allocateBuffer();

		if (options != GBL_WRITE_ONLY_DISCARD)
		{
			// Download the old contents of the texture
			download(mBuffer);
		}

		mCurrentLockOptions = options;
		mLockedBox = lockBox;

		mCurrentLock = mBuffer.GetSubVolume(lockBox);
		mIsLocked = true;

		return mCurrentLock;
	}

	void GLPixelBuffer::Unlock()
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

	void GLPixelBuffer::Upload(const PixelData& data, const PixelVolume& dest)
	{
		BS_EXCEPT(RenderingAPIException, "Upload not possible for this pixel buffer type");
	}

	void GLPixelBuffer::Download(const PixelData& data)
	{
		BS_EXCEPT(RenderingAPIException, "Download not possible for this pixel buffer type");
	}

	void GLPixelBuffer::BlitFromTexture(GLTextureBuffer* src)
	{
		blitFromTexture(src,
			PixelVolume(0, 0, 0, src->GetWidth(), src->getHeight(), src->getDepth()),
			PixelVolume(0, 0, 0, mWidth, mHeight, mDepth)
			);
	}

	void GLPixelBuffer::BlitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
	{
		BS_EXCEPT(RenderingAPIException, "BlitFromTexture not possible for this pixel buffer type");
	}

	void GLPixelBuffer::BindToFramebuffer(GLenum attachment, UINT32 zoffset, bool allLayers)
	{
		BS_EXCEPT(RenderingAPIException, "Framebuffer bind not possible for this pixel buffer type");
	}

	GLTextureBuffer::GLTextureBuffer(GLenum target, GLuint id, GLint face, GLint level, PixelFormat format,
		GpuBufferUsage usage, bool hwGamma, UINT32 multisampleCount)
		: GLPixelBuffer(0, 0, 0, format, usage), mTarget(target), mTextureID(id), mFace(face)
		, MLevel(level), mMultisampleCount(multisampleCount), mHwGamma(hwGamma)
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

	void GLTextureBuffer::Upload(const PixelData& data, const PixelVolume& dest)
	{
		if ((mUsage & TU_DEPTHSTENCIL) != 0)
		{
			BS_LOG(Error, RenderBackend, "Writing to depth stencil texture from CPU not supported.");
			return;
		}

		glBindTexture(mTarget, mTextureID);
		BS_CHECK_GL_ERROR();

		if(PixelUtil::isCompressed(data.GetFormat()))
		{
			// Block-compressed data cannot be smaller than 4x4, and must be a multiple of 4
			const UINT32 widthInBlocks = Math::divideAndRoundUp(std::max(mWidth, 4U), 4U);
			const UINT32 heightInBlocks = Math::divideAndRoundUp(std::max(mHeight, 4U), 4U);

			const UINT32 blockSize = PixelUtil::getBlockSize(data.GetFormat());
			const UINT32 expectedRowPitch = widthInBlocks * blockSize;
			const UINT32 expectedSlicePitch = widthInBlocks * heightInBlocks * blockSize;

			const bool isConsecutive = data.GetRowPitch() == expectedRowPitch && data.getSlicePitch() == expectedSlicePitch;
			if (data.GetFormat() != mFormat || !isConsecutive)
			{
				BS_LOG(Error, RenderBackend, "Compressed images must be consecutive, in the source format");
				return;
			}

			GLenum format = GLPixelUtil::getGLInternalFormat(mFormat, mHwGamma);
			switch(mTarget)
			{
				case GL_TEXTURE_1D:
					glCompressedTexSubImage1D(GL_TEXTURE_1D, mLevel,
						dest.left,
						dest.GetWidth(),
						format, data.GetConsecutiveSize(),
						data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					glCompressedTexSubImage2D(mFaceTarget, mLevel,
						dest.left, dest.top,
						dest.GetWidth(), dest.getHeight(),
						format, data.GetConsecutiveSize(),
						data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_3D:
					glCompressedTexSubImage3D(GL_TEXTURE_3D, mLevel,
						dest.left, dest.top, dest.front,
						dest.GetWidth(), dest.getHeight(), dest.getDepth(),
						format, data.GetConsecutiveSize(),
						data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				default:
					break;
			}
		
		}
		else
		{
			UINT32 pixelSize = PixelUtil::getNumElemBytes(data.GetFormat());
			UINT32 rowPitchInPixels = data.GetRowPitch() / pixelSize;
			UINT32 slicePitchInPixels = data.GetSlicePitch() / pixelSize;

			if (data.GetWidth() != rowPitchInPixels)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, rowPitchInPixels);
				BS_CHECK_GL_ERROR();
			}

			if (data.GetHeight()*data.getWidth() != slicePitchInPixels)
			{
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (slicePitchInPixels / data.GetWidth()));
				BS_CHECK_GL_ERROR();
			}

			if (data.GetLeft() > 0 || data.getTop() > 0 || data.getFront() > 0)
			{
				glPixelStorei(
					GL_UNPACK_SKIP_PIXELS,
					data.GetLeft() + rowPitchInPixels * data.getTop() + slicePitchInPixels * data.getFront());
				BS_CHECK_GL_ERROR();
			}

			if ((data.GetWidth()*pixelSize) & 3)
			{
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				BS_CHECK_GL_ERROR();
			}

			switch(mTarget) {
				case GL_TEXTURE_1D:
					glTexSubImage1D(GL_TEXTURE_1D, mLevel,
						dest.left,
						dest.GetWidth(),
						GLPixelUtil::getGLOriginFormat(data.GetFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					glTexSubImage2D(mFaceTarget, mLevel,
						dest.left, dest.top,
						dest.GetWidth(), dest.getHeight(),
						GLPixelUtil::getGLOriginFormat(data.GetFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D_ARRAY:
				case GL_TEXTURE_3D:
					glTexSubImage3D(
						mTarget, mLevel,
						dest.left, dest.top, dest.front,
						dest.GetWidth(), dest.getHeight(), dest.getDepth(),
						GLPixelUtil::getGLOriginFormat(data.GetFormat()), GLPixelUtil::getGLOriginDataType(data.getFormat()),
						data.GetData());
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

	void GLTextureBuffer::Download(const PixelData &data)
	{
		if (data.GetWidth() != getWidth() || data.getHeight() != getHeight() || data.getDepth() != getDepth())
		{
			BS_LOG(Error, RenderBackend, "Only download of entire buffer is supported by OpenGL.");
			return;
		}

		glBindTexture(mTarget, mTextureID);
		BS_CHECK_GL_ERROR();

		if(PixelUtil::isCompressed(data.GetFormat()))
		{
			// Block-compressed data cannot be smaller than 4x4, and must be a multiple of 4
			const UINT32 widthInBlocks = Math::divideAndRoundUp(std::max(mWidth, 4U), 4U);
			const UINT32 heightInBlocks = Math::divideAndRoundUp(std::max(mHeight, 4U), 4U);

			const UINT32 blockSize = PixelUtil::getBlockSize(data.GetFormat());
			const UINT32 expectedRowPitch = widthInBlocks * blockSize;
			const UINT32 expectedSlicePitch = widthInBlocks * heightInBlocks * blockSize;

			const bool isConsecutive = data.GetRowPitch() == expectedRowPitch && data.getSlicePitch() == expectedSlicePitch;
			if (data.GetFormat() != mFormat || !isConsecutive)
			{
				BS_LOG(Error, RenderBackend, "Compressed images must be consecutive, in the source format");
				return;
			}

			// Data must be consecutive and at beginning of buffer as PixelStorei not allowed
			// for compressed formate
			glGetCompressedTexImage(mFaceTarget, mLevel, data.GetData());
			BS_CHECK_GL_ERROR();
		}
		else
		{
			UINT32 pixelSize = PixelUtil::getNumElemBytes(data.GetFormat());
			UINT32 rowPitchInPixels = data.GetRowPitch() / pixelSize;
			UINT32 slicePitchInPixels = data.GetSlicePitch() / pixelSize;

			if (data.GetWidth() != rowPitchInPixels)
			{
				glPixelStorei(GL_PACK_ROW_LENGTH, rowPitchInPixels);
				BS_CHECK_GL_ERROR();
			}

			if (data.GetHeight()*data.getWidth() != slicePitchInPixels)
			{
				glPixelStorei(GL_PACK_IMAGE_HEIGHT, (slicePitchInPixels / data.GetWidth()));
				BS_CHECK_GL_ERROR();
			}

			if (data.GetLeft() > 0 || data.getTop() > 0 || data.getFront() > 0)
			{
				glPixelStorei(
					GL_PACK_SKIP_PIXELS,
					data.GetLeft() + rowPitchInPixels * data.getTop() + slicePitchInPixels * data.getFront());
				BS_CHECK_GL_ERROR();
			}

			if ((data.GetWidth()*pixelSize) & 3)
			{
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				BS_CHECK_GL_ERROR();
			}

			// We can only get the entire texture
			glGetTexImage(mFaceTarget, mLevel, GLPixelUtil::getGLOriginFormat(data.GetFormat()),
				GLPixelUtil::getGLOriginDataType(data.GetFormat()), data.getData());
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

	void GLTextureBuffer::BindToFramebuffer(GLenum attachment, UINT32 zoffset, bool allLayers)
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

	void GLTextureBuffer::CopyFromFramebuffer(UINT32 zoffset)
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

	void GLTextureBuffer::BlitFromTexture(GLTextureBuffer* src)
	{
		GLPixelBuffer::blitFromTexture(src);
	}

	void GLTextureBuffer::BlitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
	{
		// If supported, prefer direct image copy. If not supported, or if sample counts don't match, fall back to FB blit
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
		if (src->mMultisampleCount > 1 && mMultisampleCount <= 1) // Resolving MS texture
#endif
		{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
			if ( !(mTarget == GL_TEXTURE_2D || mTarget == GL_TEXTURE_2D_MULTISAMPLE) )
				BS_EXCEPT(InvalidParametersException, "Non-2D multisampled texture not supported.");
#endif

			GLint currentFBO = 0;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
			BS_CHECK_GL_ERROR();

			GLuint readFBO = GLRTTManager::instance().GetBlitReadFBO();
			GLuint drawFBO = GLRTTManager::instance().GetBlitDrawFBO();

			// Attach source texture
			glBindFramebuffer(GL_FRAMEBUFFER, readFBO);
			BS_CHECK_GL_ERROR();

			src->BindToFramebuffer(GL_COLOR_ATTACHMENT0, 0, false);

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
					mTextureID, mTarget, mLevel, dstBox.left, dstBox.top, dstBox.front, srcBox.GetWidth(), srcBox.getHeight(), srcBox.getDepth());
				BS_CHECK_GL_ERROR();
			}
			else
			{
				glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.left, srcBox.top, src->mFace,
					mTextureID, mTarget, mLevel, dstBox.left, dstBox.top, mFace, srcBox.GetWidth(), srcBox.getHeight(), 1);
				BS_CHECK_GL_ERROR();
			}
		}		
#endif
	}
}}
