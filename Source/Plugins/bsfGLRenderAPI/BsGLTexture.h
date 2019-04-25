//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Image/BsTexture.h"
#include "BsGLSupport.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	 /**	OpenGL implementation of a texture. */
	class GLTexture : public Texture
	{
	public:
		virtual ~GLTexture();

		/**	Returns OpenGL texture target type. */
		GLenum getGLTextureTarget() const;

		/**	Returns internal OpenGL texture handle. */
		GLuint getGLID() const;

		/**	Returns the internal OpenGL format used by the texture. */
		GLenum getGLFormat() const { return mGLFormat; }

		/**
		 * Returns a hardware pixel buffer for a certain face and level of the texture.
		 *
		 * @param[in]	face	Index of the texture face, if texture has more than one. Array index for texture arrays and
		 *						a cube face for cube textures.
		 * @param[in]	mipmap	Index of the mip level. 0 being the largest mip level.
		 *
		 * @note	Cube face indices: +X (0), -X (1), +Y (2), -Y (3), +Z (4), -Z (5)
		 */
		SPtr<GLPixelBuffer> getBuffer(UINT32 face, UINT32 mipmap);

		/** 
		 * Picks an OpenGL texture target based on the texture type, number of samples per pixel, and number of faces.
		 */
		static GLenum getGLTextureTarget(TextureType type, UINT32 numSamples, UINT32 numFaces);

		/** Picks an OpenGL texture target based on a GPU program parameter type. */
		static GLenum getGLTextureTarget(GpuParamObjectType type);

	protected:
		friend class GLTextureManager;

		GLTexture(GLSupport& support, const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, 
			GpuDeviceFlags deviceMask);

		/** @copydoc Texture::initialize */
		void initialize() override;

		/** @copydoc Texture::lock */
		PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
						   UINT32 queueIdx = 0) override;

		/** @copydoc Texture::unlock */
		void unlockImpl() override;

		/** @copydoc Texture::copyImpl */
		void copyImpl(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, 
			const SPtr<CommandBuffer>& commandBuffer) override;

		/** @copydoc Texture::readData */
		void readDataImpl(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
					  UINT32 queueIdx = 0) override;

		/** @copydoc Texture::writeData */
		void writeDataImpl(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false,
					   UINT32 queueIdx = 0) override;

		/** Creates pixel buffers for each face and mip level. Texture must have been created previously. */
		void createSurfaceList();

		/**	Creates an empty and uninitialized texture view object. */
		SPtr<TextureView> createView(const TEXTURE_VIEW_DESC& desc) override;

	private:
		GLuint mTextureID = 0;
		GLenum mGLFormat = 0;
		PixelFormat mInternalFormat = PF_UNKNOWN;
		GLSupport& mGLSupport;
		SPtr<GLPixelBuffer> mLockedBuffer;
		
		Vector<SPtr<GLPixelBuffer>>mSurfaceList;
	};

	/** @} */
}}