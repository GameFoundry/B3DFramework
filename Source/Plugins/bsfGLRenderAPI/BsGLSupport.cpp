//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLSupport.h"
#include "BsGLTexture.h"

#if B3D_PLATFORM != B3D_PLATFORM_ID_MACOS
#	include "GL/glew.h"

GLenum GlewContextInit(bs::ct::GLSupport* glSupport);
#endif

using namespace bs;
using namespace bs::ct;

void GLSupport::InitializeExtensions()
{
#if B3D_PLATFORM != B3D_PLATFORM_ID_MACOS
	GlewContextInit(this);
	BS_CHECK_GL_ERROR();
#endif

	// Set version string
	const GLubyte* pcVer = glGetString(GL_VERSION);
	B3D_ASSERT(pcVer && "Problems getting GL version string using glGetString");

	String tmpStr = (const char*)pcVer;
	mVersion = tmpStr.substr(0, tmpStr.find(" "));

	// Get vendor
	const GLubyte* pcVendor = glGetString(GL_VENDOR);
	tmpStr = (const char*)pcVendor;
	mVendor = tmpStr.substr(0, tmpStr.find(" "));

	// Set extension list
	i32 numExtensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	BS_CHECK_GL_ERROR();

	for(i32 i = 0; i < numExtensions; i++)
		extensionList.insert(String((char*)glGetStringi(GL_EXTENSIONS, i)));
}

bool GLSupport::CheckExtension(const String& ext) const
{
	if(extensionList.find(ext) == extensionList.end())
		return false;

	return true;
}
