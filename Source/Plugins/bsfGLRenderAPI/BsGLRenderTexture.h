//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLTexture.h"
#include "BsGLFrameBufferObject.h"
#include "Utility/BsModule.h"

#define GL_DEPTH24_STENCIL8_EXT 0x88F0

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * OpenGL implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class GLRenderTexture : public RenderTexture
	{
	public:
		virtual ~GLRenderTexture() {}

	protected:
		friend class GLTextureManager;

		GLRenderTexture(const RENDER_TEXTURE_DESC& desc);

		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	namespace ct
	{
		/**
		 * OpenGL implementation of a render texture.
		 *
		 * @note	Core thread only.
		 */
		class GLRenderTexture : public RenderTexture
		{
		public:
			GLRenderTexture(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx);
			virtual ~GLRenderTexture();

			void GetCustomAttribute(const String& name, void* data) const override;

		protected:
			friend class bs::GLRenderTexture;

			void Initialize() override;
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			RenderTextureProperties mProperties;
			GLFrameBufferObject* mFB;
		};

		/**
		 * Manager that handles valid render texture formats.
		 *
		 * @note	Must be initialized when RenderSystem is first started.
		 */
		class GLRTTManager : public Module<GLRTTManager>
		{
		public:
			GLRTTManager();
			~GLRTTManager();

			/**
			 * Check if a certain format is usable as a render target format.
			 *
			 * @note	Thread safe.
			 */
			bool CheckFormat(PixelFormat format) const { return mProps[format].Valid; }

			/**
			 * Get the closest supported alternative format. If format is supported, returns format.
			 *
			 * @note	Thread safe.
			 */
			virtual PixelFormat GetSupportedAlternative(PixelFormat format);

			/** Returns a persistent FBO that is used as a source buffer for blit operations. */
			GLuint GetBlitReadFbo() const { return mBlitReadFBO; }

			/** Returns a persistent FBO that is used as a destination buffer for blit operations. */
			GLuint GetBlitDrawFbo() const { return mBlitWriteFBO; }

		private:
			/** Frame buffer object properties for a certain texture format. */
			struct FormatProperties
			{
				/** Allowed modes/properties for this pixel format. */
				struct Mode
				{
					u32 Depth; /**< Depth format (0 = no depth). */
					u32 Stencil; /**< Stencil format (0 = no stencil). */
				};

				Vector<Mode> Modes;
				bool Valid;
			};

			/** Detect which internal formats are allowed to be used on render target color or depth/stencil surfaces. */
			void DetectFboFormats();

			/**	Checks are the specified depth & stencil formats compatible. */
			bool TryFormatInternal(GLenum depthFormat, GLenum stencilFormat);

			/**	Checks is the specified packed format valid for using in the render target. */
			bool TryPackedFormatInternal(GLenum packedFormat);

			FormatProperties mProps[PF_COUNT];
			GLuint mBlitReadFBO;
			GLuint mBlitWriteFBO;
		};
	} // namespace ct

	/** @} */
} // namespace bs
