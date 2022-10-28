//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Image/BsTexture.h"
#include "BsGLSupport.h"

namespace bs
{
	namespace ct
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
			GLenum GetGlTextureTarget() const;

			/**	Returns internal OpenGL texture handle. */
			GLuint GetGlid() const;

			/**	Returns the internal OpenGL format used by the texture. */
			GLenum GetGlFormat() const { return mGLFormat; }

			/**
			 * Returns a hardware pixel buffer for a certain face and level of the texture.
			 *
			 * @param[in]	face	Index of the texture face, if texture has more than one. Array index for texture arrays and
			 *						a cube face for cube textures.
			 * @param[in]	mipmap	Index of the mip level. 0 being the largest mip level.
			 *
			 * @note	Cube face indices: +X (0), -X (1), +Y (2), -Y (3), +Z (4), -Z (5)
			 */
			SPtr<GLPixelBuffer> GetBuffer(u32 face, u32 mipmap);

			/**
			 * Picks an OpenGL texture target based on the texture type, number of samples per pixel, and number of faces.
			 */
			static GLenum GetGlTextureTarget(TextureType type, u32 numSamples, u32 numFaces);

			/** Picks an OpenGL texture target based on a GPU program parameter type. */
			static GLenum GetGlTextureTarget(GpuParamObjectType type);

		protected:
			friend class GLTextureManager;

			GLTexture(GLSupport& support, const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);

			void Initialize() override;
			PixelData LockImpl(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void UnlockImpl() override;
			void CopyImpl(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, const SPtr<CommandBuffer>& commandBuffer) override;
			void ReadDataImpl(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteDataImpl(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0) override;

			/** Creates pixel buffers for each face and mip level. Texture must have been created previously. */
			void CreateSurfaceList();

			/**	Creates an empty and uninitialized texture view object. */
			SPtr<TextureView> CreateView(const TEXTURE_VIEW_DESC& desc);

		private:
			GLuint mTextureID = 0;
			GLenum mGLFormat = 0;
			PixelFormat mInternalFormat = PF_UNKNOWN;
			GLSupport& mGLSupport;
			SPtr<GLPixelBuffer> mLockedBuffer;

			Vector<SPtr<GLPixelBuffer>> mSurfaceList;
		};

		/** @} */
	} // namespace ct
} // namespace bs
