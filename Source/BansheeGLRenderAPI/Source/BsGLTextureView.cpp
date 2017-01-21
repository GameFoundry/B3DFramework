//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTextureView.h"
#include "BsGLTexture.h"

namespace bs { namespace ct {
	GLTextureView::GLTextureView(const GLTexture* texture, const TEXTURE_VIEW_DESC& desc)
		:TextureView(desc), mViewID(0)
	{
		const TextureProperties& props = texture->getProperties();
		GLuint originalTexture = texture->getGLID();

		GLenum target;

		switch (props.getTextureType())
		{
		case TEX_TYPE_1D:
		{
			if (desc.numArraySlices <= 1)
				target = GL_TEXTURE_1D;
			else
				target = GL_TEXTURE_1D_ARRAY;
		}
		break;
		default:
		case TEX_TYPE_2D:
		{
			if(props.getNumSamples() <= 1)
			{
				if (desc.numArraySlices <= 1)
					target = GL_TEXTURE_2D;
				else
					target = GL_TEXTURE_2D_ARRAY;
			}
			else
			{
				if (desc.numArraySlices <= 1)
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
			if(desc.numArraySlices % 6 == 0)
			{
				if (desc.numArraySlices == 6)
					target = GL_TEXTURE_CUBE_MAP;
				else
					target = GL_TEXTURE_CUBE_MAP_ARRAY;
			}
			else
			{
				if(desc.numArraySlices == 1)
					target = GL_TEXTURE_2D;
				else
					target = GL_TEXTURE_2D_ARRAY;
			}
		}
		break;
		}

		glGenTextures(1, &mViewID);
		glTextureView(mViewID, target, originalTexture, texture->getGLFormat(), desc.mostDetailMip, desc.numMips,
					  desc.firstArraySlice, desc.numArraySlices);
	}

	GLTextureView::~GLTextureView()
	{
		glDeleteTextures(1, &mViewID);
	}
}}