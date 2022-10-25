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

namespace bs
{
	namespace ct
	{
		GLPixelBuffer::GLPixelBuffer(u32 inWidth, u32 inHeight, u32 inDepth, PixelFormat inFormat, GpuBufferUsage usage)
			: mUsage(usage), mWidth(inWidth), mHeight(inHeight), mDepth(inDepth), mFormat(inFormat), mBuffer(inWidth, inHeight, inDepth, inFormat)
		{
			mSizeInBytes = mHeight * mWidth * PixelUtil::GetNumElemBytes(mFormat);
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

		void* GLPixelBuffer::Lock(u32 offset, u32 length, GpuLockOptions options)
		{
			assert(!mIsLocked && "Cannot lock this buffer, it is already locked!");
			assert(offset == 0 && length == mSizeInBytes && "Cannot lock memory region, most lock box or entire buffer");

			PixelVolume volume(0, 0, 0, mWidth, mHeight, mDepth);
			const PixelData& lockedData = Lock(volume, options);
			return lockedData.GetData();
		}

		const PixelData& GLPixelBuffer::Lock(const PixelVolume& lockBox, GpuLockOptions options)
		{
			AllocateBuffer();

			if(options != GBL_WRITE_ONLY_DISCARD)
			{
				// Download the old contents of the texture
				Download(mBuffer);
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

			if(mCurrentLockOptions != GBL_READ_ONLY)
			{
				// From buffer to card, only upload if was locked for writing
				Upload(mCurrentLock, mLockedBox);
			}

			FreeBuffer();
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
			BlitFromTexture(src, PixelVolume(0, 0, 0, src->GetWidth(), src->GetHeight(), src->GetDepth()), PixelVolume(0, 0, 0, mWidth, mHeight, mDepth));
		}

		void GLPixelBuffer::BlitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
		{
			BS_EXCEPT(RenderingAPIException, "BlitFromTexture not possible for this pixel buffer type");
		}

		void GLPixelBuffer::BindToFramebuffer(GLenum attachment, u32 zoffset, bool allLayers)
		{
			BS_EXCEPT(RenderingAPIException, "Framebuffer bind not possible for this pixel buffer type");
		}

		GLTextureBuffer::GLTextureBuffer(GLenum target, GLuint id, GLint face, GLint level, PixelFormat format, GpuBufferUsage usage, bool hwGamma, u32 multisampleCount)
			: GLPixelBuffer(0, 0, 0, format, usage), mTarget(target), mTextureID(id), mFace(face), mLevel(level), mMultisampleCount(multisampleCount), mHwGamma(hwGamma)
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
				value = 1; // Height always 1 for 1D textures
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
			mSizeInBytes = PixelUtil::GetMemorySize(mWidth, mHeight, mDepth, mFormat);

			// Set up pixel box
			mBuffer = PixelData(mWidth, mHeight, mDepth, mFormat);
		}

		void GLTextureBuffer::Upload(const PixelData& data, const PixelVolume& dest)
		{
			if((mUsage & TU_DEPTHSTENCIL) != 0)
			{
				BS_LOG(Error, RenderBackend, "Writing to depth stencil texture from CPU not supported.");
				return;
			}

			glBindTexture(mTarget, mTextureID);
			BS_CHECK_GL_ERROR();

			if(PixelUtil::IsCompressed(data.GetFormat()))
			{
				// Block-compressed data cannot be smaller than 4x4, and must be a multiple of 4
				const u32 widthInBlocks = Math::DivideAndRoundUp(std::max(mWidth, 4U), 4U);
				const u32 heightInBlocks = Math::DivideAndRoundUp(std::max(mHeight, 4U), 4U);

				const u32 blockSize = PixelUtil::GetBlockSize(data.GetFormat());
				const u32 expectedRowPitch = widthInBlocks * blockSize;
				const u32 expectedSlicePitch = widthInBlocks * heightInBlocks * blockSize;

				const bool isConsecutive = data.GetRowPitch() == expectedRowPitch && data.GetSlicePitch() == expectedSlicePitch;
				if(data.GetFormat() != mFormat || !isConsecutive)
				{
					BS_LOG(Error, RenderBackend, "Compressed images must be consecutive, in the source format");
					return;
				}

				GLenum format = GLPixelUtil::GetGlInternalFormat(mFormat, mHwGamma);
				switch(mTarget)
				{
				case GL_TEXTURE_1D:
					glCompressedTexSubImage1D(GL_TEXTURE_1D, mLevel, dest.Left, dest.GetWidth(), format, data.GetConsecutiveSize(), data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					glCompressedTexSubImage2D(mFaceTarget, mLevel, dest.Left, dest.Top, dest.GetWidth(), dest.GetHeight(), format, data.GetConsecutiveSize(), data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_3D:
					glCompressedTexSubImage3D(GL_TEXTURE_3D, mLevel, dest.Left, dest.Top, dest.Front, dest.GetWidth(), dest.GetHeight(), dest.GetDepth(), format, data.GetConsecutiveSize(), data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				default:
					break;
				}
			}
			else
			{
				u32 pixelSize = PixelUtil::GetNumElemBytes(data.GetFormat());
				u32 rowPitchInPixels = data.GetRowPitch() / pixelSize;
				u32 slicePitchInPixels = data.GetSlicePitch() / pixelSize;

				if(data.GetWidth() != rowPitchInPixels)
				{
					glPixelStorei(GL_UNPACK_ROW_LENGTH, rowPitchInPixels);
					BS_CHECK_GL_ERROR();
				}

				if(data.GetHeight() * data.GetWidth() != slicePitchInPixels)
				{
					glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (slicePitchInPixels / data.GetWidth()));
					BS_CHECK_GL_ERROR();
				}

				if(data.GetLeft() > 0 || data.GetTop() > 0 || data.GetFront() > 0)
				{
					glPixelStorei(
						GL_UNPACK_SKIP_PIXELS,
						data.GetLeft() + rowPitchInPixels * data.GetTop() + slicePitchInPixels * data.GetFront());
					BS_CHECK_GL_ERROR();
				}

				if((data.GetWidth() * pixelSize) & 3)
				{
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					BS_CHECK_GL_ERROR();
				}

				switch(mTarget)
				{
				case GL_TEXTURE_1D:
					glTexSubImage1D(GL_TEXTURE_1D, mLevel, dest.Left, dest.GetWidth(), GLPixelUtil::GetGlOriginFormat(data.GetFormat()), GLPixelUtil::GetGlOriginDataType(data.GetFormat()), data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_CUBE_MAP:
					glTexSubImage2D(mFaceTarget, mLevel, dest.Left, dest.Top, dest.GetWidth(), dest.GetHeight(), GLPixelUtil::GetGlOriginFormat(data.GetFormat()), GLPixelUtil::GetGlOriginDataType(data.GetFormat()), data.GetData());
					BS_CHECK_GL_ERROR();
					break;
				case GL_TEXTURE_2D_ARRAY:
				case GL_TEXTURE_3D:
					glTexSubImage3D(
						mTarget, mLevel,
						dest.Left, dest.Top, dest.Front,
						dest.GetWidth(), dest.GetHeight(), dest.GetDepth(),
						GLPixelUtil::GetGlOriginFormat(data.GetFormat()), GLPixelUtil::GetGlOriginDataType(data.GetFormat()),
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

		void GLTextureBuffer::Download(const PixelData& data)
		{
			if(data.GetWidth() != GetWidth() || data.GetHeight() != GetHeight() || data.GetDepth() != GetDepth())
			{
				BS_LOG(Error, RenderBackend, "Only download of entire buffer is supported by OpenGL.");
				return;
			}

			glBindTexture(mTarget, mTextureID);
			BS_CHECK_GL_ERROR();

			if(PixelUtil::IsCompressed(data.GetFormat()))
			{
				// Block-compressed data cannot be smaller than 4x4, and must be a multiple of 4
				const u32 widthInBlocks = Math::DivideAndRoundUp(std::max(mWidth, 4U), 4U);
				const u32 heightInBlocks = Math::DivideAndRoundUp(std::max(mHeight, 4U), 4U);

				const u32 blockSize = PixelUtil::GetBlockSize(data.GetFormat());
				const u32 expectedRowPitch = widthInBlocks * blockSize;
				const u32 expectedSlicePitch = widthInBlocks * heightInBlocks * blockSize;

				const bool isConsecutive = data.GetRowPitch() == expectedRowPitch && data.GetSlicePitch() == expectedSlicePitch;
				if(data.GetFormat() != mFormat || !isConsecutive)
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
				u32 pixelSize = PixelUtil::GetNumElemBytes(data.GetFormat());
				u32 rowPitchInPixels = data.GetRowPitch() / pixelSize;
				u32 slicePitchInPixels = data.GetSlicePitch() / pixelSize;

				if(data.GetWidth() != rowPitchInPixels)
				{
					glPixelStorei(GL_PACK_ROW_LENGTH, rowPitchInPixels);
					BS_CHECK_GL_ERROR();
				}

				if(data.GetHeight() * data.GetWidth() != slicePitchInPixels)
				{
					glPixelStorei(GL_PACK_IMAGE_HEIGHT, (slicePitchInPixels / data.GetWidth()));
					BS_CHECK_GL_ERROR();
				}

				if(data.GetLeft() > 0 || data.GetTop() > 0 || data.GetFront() > 0)
				{
					glPixelStorei(
						GL_PACK_SKIP_PIXELS,
						data.GetLeft() + rowPitchInPixels * data.GetTop() + slicePitchInPixels * data.GetFront());
					BS_CHECK_GL_ERROR();
				}

				if((data.GetWidth() * pixelSize) & 3)
				{
					glPixelStorei(GL_PACK_ALIGNMENT, 1);
					BS_CHECK_GL_ERROR();
				}

				// We can only get the entire texture
				glGetTexImage(mFaceTarget, mLevel, GLPixelUtil::GetGlOriginFormat(data.GetFormat()), GLPixelUtil::GetGlOriginDataType(data.GetFormat()), data.GetData());
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

		void GLTextureBuffer::BindToFramebuffer(GLenum attachment, u32 zoffset, bool allLayers)
		{
			if(mTarget == GL_TEXTURE_1D || mTarget == GL_TEXTURE_2D)
				allLayers = true;

			if(allLayers)
			{
				switch(mTarget)
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
				switch(mTarget)
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

		void GLTextureBuffer::CopyFromFramebuffer(u32 zoffset)
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
			GLPixelBuffer::BlitFromTexture(src);
		}

		void GLTextureBuffer::BlitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox)
		{
			// If supported, prefer direct image copy. If not supported, or if sample counts don't match, fall back to FB blit
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
			if(src->mMultisampleCount > 1 && mMultisampleCount <= 1) // Resolving MS texture
#endif
			{
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
				if(!(mTarget == GL_TEXTURE_2D || mTarget == GL_TEXTURE_2D_MULTISAMPLE))
					BS_EXCEPT(InvalidParametersException, "Non-2D multisampled texture not supported.");
#endif

				GLint currentFBO = 0;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
				BS_CHECK_GL_ERROR();

				GLuint readFBO = GLRTTManager::Instance().GetBlitReadFbo();
				GLuint drawFBO = GLRTTManager::Instance().GetBlitDrawFbo();

				// Attach source texture
				glBindFramebuffer(GL_FRAMEBUFFER, readFBO);
				BS_CHECK_GL_ERROR();

				src->BindToFramebuffer(GL_COLOR_ATTACHMENT0, 0, false);

				// Attach destination texture
				glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
				BS_CHECK_GL_ERROR();

				BindToFramebuffer(GL_COLOR_ATTACHMENT0, 0, false);

				// Perform blit
				glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
				BS_CHECK_GL_ERROR();

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
				BS_CHECK_GL_ERROR();

				glReadBuffer(GL_COLOR_ATTACHMENT0);
				BS_CHECK_GL_ERROR();

				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				BS_CHECK_GL_ERROR();

				glBlitFramebuffer(srcBox.Left, srcBox.Top, srcBox.Right, srcBox.Bottom, dstBox.Left, dstBox.Top, dstBox.Right, dstBox.Bottom, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				BS_CHECK_GL_ERROR();

				// Restore the previously bound FBO
				glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
				BS_CHECK_GL_ERROR();
			}
#if BS_OPENGL_4_3 || BS_OPENGLES_3_2
			else // Just plain copy
			{
				if(mMultisampleCount != src->mMultisampleCount)
					BS_EXCEPT(InvalidParametersException, "When copying textures their multisample counts must match.");

				if(mTarget == GL_TEXTURE_3D) // 3D textures can't have arrays so their Z coordinate is handled differently
				{
					glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.Left, srcBox.Top, srcBox.Front, mTextureID, mTarget, mLevel, dstBox.Left, dstBox.Top, dstBox.Front, srcBox.GetWidth(), srcBox.GetHeight(), srcBox.GetDepth());
					BS_CHECK_GL_ERROR();
				}
				else
				{
					glCopyImageSubData(src->mTextureID, src->mTarget, src->mLevel, srcBox.Left, srcBox.Top, src->mFace, mTextureID, mTarget, mLevel, dstBox.Left, dstBox.Top, mFace, srcBox.GetWidth(), srcBox.GetHeight(), 1);
					BS_CHECK_GL_ERROR();
				}
			}
#endif
		}
	} // namespace ct
} // namespace bs
