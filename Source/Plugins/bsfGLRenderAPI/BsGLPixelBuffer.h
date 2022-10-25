//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "Image/BsPixelUtil.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		class GLTextureBuffer;

		/**
		 * Represents a hardware buffer that stores a single pixel surface. This may be a 1D, 2D or 3D surface, but unlike a
		 * texture it consists only of a single surface (no mip maps, cube map faces or similar).
		 */
		class GLPixelBuffer
		{
		public:
			/**
			 * Constructs a new pixel buffer with the provided settings.
			 *
			 * @param[in]	width			Width of the pixel buffer in pixels.
			 * @param[in]	height			Height of the pixel buffer in pixels.
			 * @param[in]	depth			Depth of the pixel buffer in pixels (number of 2D slices).
			 * @param[in]	format			Format of each pixel in the buffer.
			 * @param[in]	usage			Usage signaling the render system how we plan on using the buffer.
			 */
			GLPixelBuffer(u32 width, u32 height, u32 depth, PixelFormat format, GpuBufferUsage usage);
			virtual ~GLPixelBuffer();

			/**	Returns width of the surface in pixels. */
			u32 GetWidth() const { return mWidth; }

			/**	Returns height of the surface in pixels. */
			u32 GetHeight() const { return mHeight; }

			/**	Returns depth of the surface in pixels. */
			u32 GetDepth() const { return mDepth; }

			/**	Returns format of the pixels in the surface. */
			PixelFormat GetFormat() const { return mFormat; }

			/**
			 * Locks a certain region of the pixel buffer for reading and returns a pointer to the locked region.
			 *
			 * @param[in]	lockBox		Region of the surface to lock.
			 * @param[in]	options		Lock options that hint the hardware on what you intend to do with the locked data.
			 *
			 * @note	Returned object is only valid while the lock is active.
			 */
			const PixelData& Lock(const PixelVolume& lockBox, GpuLockOptions options);

			/**
			 * Locks a portion of the buffer and returns pointer to the locked area. You must call unlock() when done.
			 *
			 * @param[in]	offset	Offset in bytes from which to lock the buffer.
			 * @param[in]	length	Length of the area you want to lock, in bytes.
			 * @param[in]	options	Signifies what you want to do with the returned pointer. Caller must ensure not to do
			 *						anything he hasn't requested (for example don't try to read from the buffer unless you
			 *						requested it here).
			 */
			void* Lock(u32 offset, u32 length, GpuLockOptions options);

			/**
			 * Locks the entire buffer and returns pointer to the locked area. You must call unlock() when done.
			 *
			 * @param[in]	options	Signifies what you want to do with the returned pointer. Caller must ensure not to do
			 *						anything he hasn't requested (for example don't try to read from the buffer unless you
			 *						requested it here).
			 */
			void* Lock(GpuLockOptions options)
			{
				return Lock(0, mSizeInBytes, options);
			}

			/**	Releases the lock on this buffer. */
			void Unlock();

			/**
			 * Upload some pixel data to the buffer.
			 *
			 * @param[in]	data	Data to upload.
			 * @param[in]	dest	Coordinates to which to upload the data.
			 */
			virtual void Upload(const PixelData& data, const PixelVolume& dest);

			/**
			 * Reads data from the pixel buffer into the provided object. Caller must ensure the data object is of adequate
			 * size.
			 */
			virtual void Download(const PixelData& data);

			/**
			 * Binds the buffers to a frame buffer object at the specified attachment point.
			 *
			 * @param[in]	attachment	Attachment point index in the range [0, BS_MAX_MULTIPLE_RENDER_TARGETS).
			 * @param[in]	zoffset		Depth slice to bind, in the case of a 3D texture.
			 * @param[in]	allLayers	Should all layers of the texture be bound, or just one (zoffset is ignored if true).
			 */
			virtual void BindToFramebuffer(GLenum attachment, u32 zoffset, bool allLayers);

			/**
			 * Blits the contents of the provided buffer into this pixel buffer. Data is bilinearily interpolated in case buffer
			 * sizes don't match.
			 */
			virtual void BlitFromTexture(GLTextureBuffer* src);

			/**
			 * Blits contents of a sub-region of the provided buffer into a sub-region of this pixel buffer. Data is bilinearily
			 * interpolated in case source and destination sizes don't match.
			 */
			virtual void BlitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox);

		protected:
			/**	Allocates an internal buffer on the CPU, the size of the hardware buffer. */
			void AllocateBuffer();

			/**	Deallocates the internal CPU buffer. */
			void FreeBuffer();

		protected:
			u32 mSizeInBytes;
			GpuBufferUsage mUsage;
			bool mIsLocked = false;

			u32 mWidth, mHeight, mDepth;
			PixelFormat mFormat;

			PixelData mCurrentLock;
			PixelVolume mLockedBox;

			PixelData mBuffer;
			GpuLockOptions mCurrentLockOptions;
		};

		/**	Pixel buffer specialization that represents a single surface in a texture. */
		class GLTextureBuffer : public GLPixelBuffer
		{
		public:
			/**
			 * Constructs a new texture buffer from a specific surface in the provided texture.
			 *
			 * @param[in]	target				OpenGL type of the texture to retrieve the surface from.
			 * @param[in]	id					OpenGL handle to the texture to retrieve the surface from.
			 * @param[in]	face				Face index of the texture in the case of cube textures or texture arrays.
			 * @param[in]	level				Mip level of the texture.
			 * @param[in]	format				Format of each pixel in the buffer.
			 * @param[in]	usage				Usage signaling the render system how we plan on using the buffer.
			 * @param[in]	hwGamma				True if buffer colors are assumed to be in sRGB space.
			 * @param[in]	multisampleCount	Number of samples the parent texture was created with.
			 */
			GLTextureBuffer(GLenum target, GLuint id, GLint face, GLint level, PixelFormat format, GpuBufferUsage usage, bool hwGamma, u32 multisampleCount);
			~GLTextureBuffer() = default;

			/** @copydoc GLPixelBuffer::bindToFramebuffer */
			void BindToFramebuffer(GLenum attachment, u32 zoffset, bool allLayers) override;

			/** @copydoc GLPixelBuffer::upload */
			void Upload(const PixelData& data, const PixelVolume& dest) override;

			/** @copydoc GLPixelBuffer::download */
			void Download(const PixelData& data) override;

			/** @copydoc GLPixelBuffer::blitFromTexture */
			void BlitFromTexture(GLTextureBuffer* src) override;

			/** @copydoc GLPixelBuffer::blitFromTexture */
			void BlitFromTexture(GLTextureBuffer* src, const PixelVolume& srcBox, const PixelVolume& dstBox) override;

			/**
			 * Populate texture buffer with the data in the currently attached frame buffer.
			 *
			 * @param[in]	zoffset		3D slice of the texture to copy to. 0 if texture is not 3D.
			 */
			void CopyFromFramebuffer(u32 zoffset);

		protected:
			GLenum mTarget;
			GLenum mFaceTarget = 0;
			GLuint mTextureID;
			GLint mFace;
			GLint mLevel;
			u32 mMultisampleCount;
			bool mHwGamma;
		};

		/** @} */
	} // namespace ct
} // namespace bs
