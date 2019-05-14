//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "MacOS/BsMacOSGLSupport.h"
#include "MacOS/BsMacOSContext.h"
#include "MacOS/BsMacOSRenderWindow.h"
#include "MacOS/BsMacOSVideoModeInfo.h"
#include "BsGLRenderAPI.h"
#include <dlfcn.h>

namespace bs::ct
{
	SPtr<bs::RenderWindow> MacOSGLSupport::newWindow(
		RENDER_WINDOW_DESC& desc,
		UINT32 windowId,
		SPtr<bs::RenderWindow> parentWindow)
	{
		bs::MacOSRenderWindow* window = new (bs_alloc<bs::MacOSRenderWindow>()) bs::MacOSRenderWindow(desc, windowId, *this);
		return SPtr<bs::RenderWindow>(window, &bs::CoreObject::_delete<bs::MacOSRenderWindow, GenAlloc>);
	}

	void MacOSGLSupport::start()
	{
		// Do nothing
	}

	void MacOSGLSupport::stop()
	{
		// Do nothing
	}

	SPtr<MacOSContext> MacOSGLSupport::createContext(bool depthStencil, UINT32 msaaCount)
	{
		GLRenderAPI* rapi = static_cast<GLRenderAPI*>(RenderAPI::instancePtr());

		// If RenderAPI has initialized a context use that, otherwise we create our own
		if (!rapi->_isContextInitialized())
			return bs_shared_ptr_new<MacOSContext>(depthStencil, msaaCount);
		else
		{
			SPtr<GLContext> context = rapi->_getMainContext();
			return std::static_pointer_cast<MacOSContext>(context);
		}
	}

	void* MacOSGLSupport::getProcAddress(const String& procname)
	{
		static void* image = nullptr;

		if (!image)
			image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);

		if(!image)
			return nullptr;

		return dlsym(image, (const char*)procname.c_str());
	}

	SPtr<VideoModeInfo> MacOSGLSupport::getVideoModeInfo() const
	{
		return bs_shared_ptr_new<MacOSVideoModeInfo>();
	}
}

