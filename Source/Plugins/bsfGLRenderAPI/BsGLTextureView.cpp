//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLTextureView.h"
#include "BsGLTexture.h"

namespace bs
{
	namespace ct
	{
		GLTextureView::GLTextureView(const GLTexture* texture, const TEXTURE_VIEW_DESC& desc)
			: TextureView(desc)
		{
			const TextureProperties& props = texture->GetProperties();

			GLenum target;
			switch(props.GetTextureType())
			{
			case TEX_TYPE_1D:
				{
					if(desc.NumArraySlices <= 1)
						target = GL_TEXTURE_1D;
					else
						target = GL_TEXTURE_1D_ARRAY;
				}
				break;
			default:
			case TEX_TYPE_2D:
				{
					if(props.GetNumSamples() <= 1)
					{
						if(desc.NumArraySlices <= 1)
							target = GL_TEXTURE_2D;
						else
							target = GL_TEXTURE_2D_ARRAY;
					}
					else
					{
						if(desc.NumArraySlices <= 1)
							target = GL_TEXTURE_2D_MULTISAMPLE;
						else
							target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
					}
				}
				break;
			case TEX_TYPE_3D:
				target = GL_TEXTURE_3D;
				break;
			case TEX_TYPE_CUBE_MAP:
				{
					if(desc.NumArraySlices % 6 == 0)
					{
						if(desc.NumArraySlices == 6)
							target = GL_TEXTURE_CUBE_MAP;
						else
							target = GL_TEXTURE_CUBE_MAP_ARRAY;
					}
					else
					{
						if(desc.NumArraySlices == 1)
							target = GL_TEXTURE_2D;
						else
							target = GL_TEXTURE_2D_ARRAY;
					}
				}
				break;
			}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			GLuint originalTexture = texture->GetGlid();

			glGenTextures(1, &mViewID);
			BS_CHECK_GL_ERROR();

			glTextureView(
				mViewID,
				target,
				originalTexture,
				texture->GetGlFormat(),
				desc.MostDetailMip,
				desc.NumMips,
				desc.FirstArraySlice,
				desc.NumArraySlices);
			BS_CHECK_GL_ERROR();
#endif

			mTarget = GLTexture::GetGlTextureTarget(props.GetTextureType(), props.GetNumSamples(), desc.NumArraySlices);
		}

		GLTextureView::~GLTextureView()
		{
			if(mViewID != 0)
			{
				glDeleteTextures(1, &mViewID);
				BS_CHECK_GL_ERROR();
			}
		}
	} // namespace ct
} // namespace bs
