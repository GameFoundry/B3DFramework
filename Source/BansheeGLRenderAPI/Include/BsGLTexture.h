//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsTexture.h"
#include "BsGLSupport.h"

namespace bs 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a texture. */
    class BS_RSGL_EXPORT GLTextureCore : public TextureCore
    {
    public:
		virtual ~GLTextureCore();

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

    protected:
		friend class GLTextureCoreManager;

		GLTextureCore(GLSupport& support, const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, 
			GpuDeviceFlags deviceMask);

		/** @copydoc TextureCore::initialize */
		void initialize() override;

		/** @copydoc TextureCore::lock */
		PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
						   UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::unlock */
		void unlockImpl() override;

		/** @copydoc TextureCore::copyImpl */
		void copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
					  const SPtr<TextureCore>& target, UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::readData */
		void readData(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
					  UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::writeData */
		void writeData(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false,
					   UINT32 queueIdx = 0) override;

		/** Creates pixel buffers for each face and mip level. Texture must have been created previously. */
		void createSurfaceList();

    private:
        GLuint mTextureID;
		GLenum mGLFormat;
        GLSupport& mGLSupport;
		SPtr<GLPixelBuffer> mLockedBuffer;
		
		Vector<SPtr<GLPixelBuffer>>mSurfaceList;
    };

	/** @} */
}