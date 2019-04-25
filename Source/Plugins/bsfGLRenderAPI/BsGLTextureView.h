//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsTextureView.h"

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

		/**	Returns OpenGL texture target type. */
		GLuint getGLTextureTarget() const { return mTarget; }

	protected:
		friend class GLTexture;

		GLTextureView(const GLTexture* texture, const TEXTURE_VIEW_DESC& desc);
	private:
		GLuint mViewID = 0;
		GLuint mTarget;
	};

	/** @} */
}}