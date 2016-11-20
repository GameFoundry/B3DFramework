//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLTexture.h"
#include "BsGLFrameBufferObject.h"
#include "BsModule.h"

#define GL_DEPTH24_STENCIL8_EXT 0x88F0

namespace bs 
{  
	/** @addtogroup GL
	 *  @{
	 */

	class GLRenderTexture;

	/**
	 * OpenGL implementation of a render texture.
	 *
	 * @note	Core thread only.
	 */
    class BS_RSGL_EXPORT GLRenderTextureCore : public RenderTextureCore
    {
	public:
		GLRenderTextureCore(const RENDER_TEXTURE_DESC_CORE& desc, GpuDeviceFlags deviceMask);
		virtual ~GLRenderTextureCore();

		/** @copydoc RenderTextureCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* data) const override;

	protected:
		friend class GLRenderTexture;

		/** @copydoc RenderTextureCore::initialize */
		void initialize() override;

		/** @copydoc RenderTextureCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
		GLFrameBufferObject* mFB;
    };

    /**
     * Manager that handles valid render texture formats.
     * 			
	 * @note	Must be initialized when RenderSystem is first started.
     */
    class BS_RSGL_EXPORT GLRTTManager : public Module<GLRTTManager>
    {
    public:
        GLRTTManager();
		~GLRTTManager();
        
        /**
         * Check if a certain format is usable as a render target format.
         *
         * @note	Thread safe.
         */
        bool checkFormat(PixelFormat format) const { return mProps[format].valid; }
        
        /**
         * Get the closest supported alternative format. If format is supported, returns format.
         *
         * @note	Thread safe.
         */
        virtual PixelFormat getSupportedAlternative(PixelFormat format);

		/** Returns a persistent FBO that is used as a source buffer for blit operations. */
		GLuint getBlitReadFBO() const { return mBlitReadFBO; }

		/** Returns a persistent FBO that is used as a destination buffer for blit operations. */
		GLuint getBlitDrawFBO() const { return mBlitWriteFBO; }
    private:
        /** Frame buffer object properties for a certain texture format. */
        struct FormatProperties
        {
            /** Allowed modes/properties for this pixel format. */
            struct Mode
            {
                UINT32 depth;     /**< Depth format (0 = no depth). */
                UINT32 stencil;   /**< Stencil format (0 = no stencil). */
            };
            
            Vector<Mode> modes;
			bool valid;
        };

		/** Detect which internal formats are allowed to be used on render target color or depth/stencil surfaces. */
        void detectFBOFormats();

		/**	Checks are the specified depth & stencil formats compatible. */
		bool _tryFormat(GLenum depthFormat, GLenum stencilFormat);

		/**	Checks is the specified packed format valid for using in the render target. */
        bool _tryPackedFormat(GLenum packedFormat);

		FormatProperties mProps[PF_COUNT];
		GLuint mBlitReadFBO;
		GLuint mBlitWriteFBO;
    };

	/**
	 * OpenGL implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class GLRenderTexture : public RenderTexture
	{
	public:
		virtual ~GLRenderTexture() { }

	protected:
		friend class GLTextureManager;

		GLRenderTexture(const RENDER_TEXTURE_DESC& desc);

		/** @copydoc RenderTexture::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	/** @} */
}