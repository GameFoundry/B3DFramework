//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "MacOS/BsMacOSContext.h"
#include "MacOS/BsMacOSGLSupport.h"
#define BS_COCOA_INTERNALS
#include "Private/MacOS/BsMacOSWindow.h"
#include "Private/MacOS/BsMacOSPlatform.h"
#import <AppKit/AppKit.h>

namespace bs::ct
{
	struct MacOSContext::Pimpl
	{
		NSOpenGLContext* context = nil;
		bool dirty = true;
	};

	MacOSContext::MacOSContext(bool depthStencil, UINT32 msaaCount)
	{ @autoreleasepool {
		NSOpenGLPixelFormatAttribute attributes[17];

		UINT32 attrIdx = 0;

		attributes[attrIdx++] = NSOpenGLPFAOpenGLProfile;
		attributes[attrIdx++] = NSOpenGLProfileVersion4_1Core;

		attributes[attrIdx++] = NSOpenGLPFADoubleBuffer;
		attributes[attrIdx++] = NSOpenGLPFAAccelerated;

		attributes[attrIdx++] = NSOpenGLPFAColorSize;
		attributes[attrIdx++] = 32;

		if(depthStencil)
		{
			attributes[attrIdx++] = NSOpenGLPFADepthSize;
			attributes[attrIdx++] = 24;
			attributes[attrIdx++] = NSOpenGLPFAStencilSize;
			attributes[attrIdx++] = 8;
		}

		if(msaaCount > 1)
		{
			attributes[attrIdx++] = NSOpenGLPFAMultisample;
			attributes[attrIdx++] = NSOpenGLPFASampleBuffers;
			attributes[attrIdx++] = 1;
			attributes[attrIdx++] = NSOpenGLPFASamples;
			attributes[attrIdx++] = msaaCount;
			attributes[attrIdx++] = NSOpenGLPFANoRecovery;
		}

		attributes[attrIdx] = 0;

		NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
		m = bs_new<Pimpl>();
		m->context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];

		markAsDirty();
	}}

	MacOSContext::~MacOSContext()
	{
		bs_delete(m);
	}

	void MacOSContext::setCurrent(const RenderWindow& renderWindow)
	{
		UINT32 windowId;
		renderWindow.getCustomAttribute("WINDOW_ID", &windowId);

		MacOSPlatform::lockWindows();

		CocoaWindow* window = MacOSPlatform::getWindow(windowId);
		if(!window)
			return;

		NSWindow* nsWindow = window->_getPrivateData()->window;

		[m->context setView:[nsWindow contentView]];
		[m->context makeCurrentContext];
		[m->context update];

		m->dirty = false;

		MacOSPlatform::unlockWindows();
	}

	void MacOSContext::endCurrent()
	{
		[m->context setView:nil];
		[m->context clearDrawable];
		[NSOpenGLContext clearCurrentContext];
	}

	void MacOSContext::releaseContext()
	{
		m->context = nil;
	}

	void MacOSContext::markAsDirty()
	{
		m->dirty = true;
	}

	void MacOSContext::updateIfDirty()
	{
		if(m->dirty)
		{
			[m->context update];
			m->dirty = false;
		}
	}

	void MacOSContext::setVSync(int interval)
	{
		if(interval < 0)
			interval = 0;

		[m->context setValues:&interval forParameter:NSOpenGLCPSwapInterval];
	}

	void MacOSContext::swapBuffers()
	{
		[m->context flushBuffer];

		updateIfDirty();
	}
}

