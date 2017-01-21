//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsTextureView.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a texture resource view. */
	class GLTextureView : public TextureView
	{
	public:
		~GLTextureView();

		/**	Returns internal OpenGL texture view handle. */
		GLuint getGLID() const { return mViewID; }

	protected:
		friend class GLTexture;

		GLTextureView(const GLTexture* texture, const TEXTURE_VIEW_DESC& desc);
	private:
		GLuint mViewID;
	};

	/** @} */
}}