//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define BS_COCOA_INTERNALS 1
#include "Private/MacOS/BsMacOSPlatform.h"
#include "Private/MacOS/BsMacOSWindow.h"
#include "Input/BsInputFwd.h"
#include "Image/BsPixelData.h"
#include "Image/BsColor.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Private/MacOS/BsMacOSDropTarget.h"
#include "String/BsUnicode.h"
#include "BsCoreApplication.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

/** Application implementation that overrides the terminate logic with custom shutdown, and tracks Esc key presses. */
@interface BSApplication : NSApplication
-(void)terminate:(nullable id)sender;
-(void)sendEvent:(NSEvent*)event;
@end

@implementation BSApplication
-(void)terminate:(nullable id)sender
{
	bs::gCoreApplication().quitRequested();
}

-(void)sendEvent:(NSEvent *)event
{
	// Handle Esc & Tab key here, as it doesn't seem to be reported elsewhere
	if([event type] == NSEventTypeKeyDown)
	{
		if([event keyCode] == 53) // Escape key
		{
			bs::InputCommandType ic = bs::InputCommandType::Escape;
			bs::MacOSPlatform::sendInputCommandEvent(ic);
		}
		else if([event keyCode] == 48) // Tab key
		{
			bs::InputCommandType ic = bs::InputCommandType::Tab;
			bs::MacOSPlatform::sendInputCommandEvent(ic);
		}
	}

	[super sendEvent:event];
}
@end

/** Application delegate implementation that activates the application when it finishes launching. */
@interface BSAppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation BSAppDelegate : NSObject
-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
	[NSApp activateIgnoringOtherApps:YES];
}
@end

@class BSCursor;
@class BSFPlatform;

namespace bs
{
	/** Contains information about a modal window session. */
	struct ModalWindowInfo
	{
		UINT32 windowId;
		NSModalSession session;
	};

	struct Platform::Pimpl
	{
		BSAppDelegate* appDelegate = nil;

		Mutex windowMutex;
		CocoaWindow* mainWindow = nullptr;
		UnorderedMap<UINT32, CocoaWindow*> allWindows;
		Vector<ModalWindowInfo> modalWindows;

		BSFPlatform* platformManager = nil;

		// Cursor
		BSCursor* cursorManager = nil;

		Mutex cursorMutex;
		bool cursorIsHidden = false;
		Vector2I cursorPos;

		// Clipboard
		Mutex clipboardMutex;
		String cachedClipboardData;
		INT32 clipboardChangeCount = -1;
	};
}

/**
 * Contains cursor specific functionality. Encapsulated in objective C so its selectors can be triggered from other
 * threads.
 */
@interface BSCursor : NSObject
@property NSCursor* currentCursor;

-(BSCursor*) initWithPlatformData:(bs::Platform::Pimpl*)platformData;
-(bs::Vector2I) getPosition;
-(void) setPosition:(const bs::Vector2I&) position;
-(BOOL) clipCursor:(bs::Vector2I&) position;
-(void) updateClipBounds:(NSWindow*) window;
-(void) clipCursorToWindow:(NSValue*) windowValue;
-(void) clipCursorToRect:(NSValue*) rectValue;
-(void) clipCursorDisable;
-(void) setCursor:(NSArray*) params;
-(void) unregisterWindow:(NSWindow*) window;
@end

@implementation BSCursor
{
	bs::Platform::Pimpl* platformData;

	bool cursorClipEnabled;
	bs::Rect2I cursorClipRect;
	NSWindow* cursorClipWindow;
}

- (BSCursor*)initWithPlatformData:(bs::Platform::Pimpl*)data
{
	self = [super init];

	platformData = data;
	return self;
}

- (bs::Vector2I)getPosition
{
	NSPoint point = [NSEvent mouseLocation];

	for (NSScreen* screen in [NSScreen screens])
	{
		NSRect frame = [screen frame];
		if (NSMouseInRect(point, frame, NO))
			bs::flipY(screen, point);
	}

	bs::Vector2I output;
	output.x = (int32_t)point.x;
	output.y = (int32_t)point.y;

	return output;
}

- (void)setPosition:(const bs::Vector2I&)position
{
	NSPoint point = NSMakePoint(position.x, position.y);
	CGWarpMouseCursorPosition(point);

	Lock lock(platformData->cursorMutex);
	platformData->cursorPos = position;
}

- (BOOL)clipCursor:(bs::Vector2I&)position
{
	if(!cursorClipEnabled)
		return false;

	int32_t clippedX = position.x - cursorClipRect.x;
	int32_t clippedY = position.y - cursorClipRect.y;

	if(clippedX < 0)
		clippedX = 0;
	else if(clippedX >= (int32_t)cursorClipRect.width)
		clippedX = cursorClipRect.width > 0 ? cursorClipRect.width - 1 : 0;

	if(clippedY < 0)
		clippedY = 0;
	else if(clippedY >= (int32_t)cursorClipRect.height)
		clippedY = cursorClipRect.height > 0 ? cursorClipRect.height - 1 : 0;

	clippedX += cursorClipRect.x;
	clippedY += cursorClipRect.y;

	if(clippedX != position.x || clippedY != position.y)
	{
		position.x = clippedX;
		position.y = clippedY;

		return true;
	}

	return false;
}

- (void)updateClipBounds:(NSWindow*)window
{
	if(!cursorClipEnabled || cursorClipWindow != window)
		return;

	NSRect rect = [window contentRectForFrameRect:[window frame]];
	bs::flipY([window screen], rect);

	cursorClipRect.x = (int32_t)rect.origin.x;
	cursorClipRect.y = (int32_t)rect.origin.y;
	cursorClipRect.width = (uint32_t)rect.size.width;
	cursorClipRect.height = (uint32_t)rect.size.height;
}

- (void)clipCursorToWindow:(NSValue*)windowValue
{
	bs::CocoaWindow* window;
	[windowValue getValue:&window];

	cursorClipEnabled = true;
	cursorClipWindow = window->_getPrivateData()->window;

	[self updateClipBounds:cursorClipWindow];

	bs::Vector2I pos = [self getPosition];

	if([self clipCursor:pos])
		[self setPosition:pos];
}

- (void)clipCursorToRect:(NSValue*)rectValue
{
	bs::Rect2I rect;
	[rectValue getValue:&rect];

	cursorClipEnabled = true;
	cursorClipRect = rect;
	cursorClipWindow = nullptr;

	bs::Vector2I pos = [self getPosition];

	if([self clipCursor:pos])
		[self setPosition:pos];
}

- (void)clipCursorDisable
{
	cursorClipEnabled = false;
	cursorClipWindow = nullptr;
}

- (void)setCursor:(NSArray*)params
{
	NSCursor* cursor = params[0];
	NSValue* hotSpotValue = params[1];

	NSPoint hotSpot;
	[hotSpotValue getValue:&hotSpot];

	[self setCurrentCursor:cursor];

	for(auto& entry : platformData->allWindows)
	{
		NSWindow* window = entry.second->_getPrivateData()->window;
		[window invalidateCursorRectsForView:[window contentView]];
	}
}

- (void)unregisterWindow:(NSWindow*)window
{
	if(cursorClipEnabled && cursorClipWindow == window)
		[self clipCursorDisable];
}

@end

/** Contains platform specific functionality that is meant to be delayed executed from the sim thread, through Platform. */
@interface BSFPlatform : NSObject
-(BSFPlatform*) initWithPlatformData:(bs::Platform::Pimpl*)platformData;
-(void) setCaptionNonClientAreas:(NSArray*) params;
-(void) resetNonClientAreas:(NSValue*) windowIdValue;
-(void) openFolder:(NSURL*) url;
-(void) setClipboardText:(NSString*) text;
-(NSString*) getClipboardText;
-(int32_t) getClipboardChangeCount;
@end

@implementation BSFPlatform
{
	bs::Platform::Pimpl* mPlatformData;
}

- (BSFPlatform*)initWithPlatformData:(bs::Platform::Pimpl*)platformData
{
	self = [super init];

	mPlatformData = platformData;
	return self;
}

- (void)setCaptionNonClientAreas:(NSArray*)params
{
	NSValue* windowIdValue = params[0];

	bs::UINT32 windowId;
	[windowIdValue getValue:&windowId];

	auto iterFind = mPlatformData->allWindows.find(windowId);
	if(iterFind == mPlatformData->allWindows.end())
		return;

	bs::CocoaWindow* window = iterFind->second;

	NSUInteger numEntries = [params count] - 1;

	bs::Vector<bs::Rect2I> areas;
	for(NSUInteger i = 0; i < numEntries; i++)
	{
		NSValue* value = params[i + 1];

		bs::Rect2I area;
		[value getValue:&area];

		areas.push_back(area);
	}

	window->_setDragZones(areas);
}

- (void)resetNonClientAreas:(NSValue*) windowIdValue
{
	bs::UINT32 windowId;
	[windowIdValue getValue:&windowId];

	auto iterFind = mPlatformData->allWindows.find(windowId);
	if(iterFind == mPlatformData->allWindows.end())
		return;

	bs::CocoaWindow* window = iterFind->second;
	window->_setDragZones({});
}

- (void)openFolder:(NSURL*)url
{
	[[NSWorkspace sharedWorkspace] openURL:url];
}

- (void) setClipboardText:(NSString*) text
{ @autoreleasepool {
	NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
	[pasteboard clearContents];
	NSArray* objects = [NSArray arrayWithObject:text];
	[pasteboard writeObjects:objects];
}}

- (NSString*) getClipboardText
{ @autoreleasepool {
	NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
	NSArray* classes = [NSArray arrayWithObjects:[NSString class], nil];
	NSDictionary* options = [NSDictionary dictionary];

	NSArray* items = [pasteboard readObjectsForClasses:classes options:options];
	if(!items || items.count == 0)
		return nil;

	return (NSString*) items[0];
}}

- (int32_t)getClipboardChangeCount
{
	return (int32_t)[[NSPasteboard generalPasteboard] changeCount];
}

@end

namespace bs
{
	// Maps macOS keycodes to bs button codes
	static constexpr ButtonCode KeyCodeMapping[] =
	{
		/*   0 */   BC_A,
		/*   1 */   BC_S,
		/*   2 */   BC_D,
		/*   3 */   BC_F,
		/*   4 */   BC_H,
		/*   5 */   BC_G,
		/*   6 */   BC_Z,
		/*   7 */   BC_X,
		/*   8 */   BC_C,
		/*   9 */   BC_V,
		/*  10 */   BC_GRAVE,
		/*  11 */   BC_B,
		/*  12 */   BC_Q,
		/*  13 */   BC_W,
		/*  14 */   BC_E,
		/*  15 */   BC_R,
		/*  16 */   BC_Y,
		/*  17 */   BC_T,
		/*  18 */   BC_1,
		/*  19 */   BC_2,
		/*  20 */   BC_3,
		/*  21 */   BC_4,
		/*  22 */   BC_6,
		/*  23 */   BC_5,
		/*  24 */   BC_EQUALS,
		/*  25 */   BC_9,
		/*  26 */   BC_7,
		/*  27 */   BC_MINUS,
		/*  28 */   BC_8,
		/*  29 */   BC_0,
		/*  30 */   BC_RBRACKET,
		/*  31 */   BC_O,
		/*  32 */   BC_U,
		/*  33 */   BC_LBRACKET,
		/*  34 */   BC_I,
		/*  35 */   BC_P,
		/*  36 */   BC_RETURN,
		/*  37 */   BC_L,
		/*  38 */   BC_J,
		/*  39 */   BC_APOSTROPHE,
		/*  40 */   BC_K,
		/*  41 */   BC_SEMICOLON,
		/*  42 */   BC_BACKSLASH,
		/*  43 */   BC_COMMA,
		/*  44 */   BC_SLASH,
		/*  45 */   BC_N,
		/*  46 */   BC_M,
		/*  47 */   BC_PERIOD,
		/*  48 */   BC_TAB,
		/*  49 */   BC_SPACE,
		/*  50 */   BC_GRAVE,
		/*  51 */   BC_BACK,
		/*  52 */   BC_NUMPADENTER,
		/*  53 */   BC_ESCAPE,
		/*  54 */   BC_RWIN,
		/*  55 */   BC_LWIN,
		/*  56 */   BC_LSHIFT,
		/*  57 */   BC_CAPITAL,
		/*  58 */   BC_LMENU,
		/*  59 */   BC_LCONTROL,
		/*  60 */   BC_RSHIFT,
		/*  61 */   BC_RMENU,
		/*  62 */   BC_RCONTROL,
		/*  63 */   BC_RWIN,
		/*  64 */   BC_UNASSIGNED,
		/*  65 */   BC_DECIMAL,
		/*  66 */   BC_UNASSIGNED,
		/*  67 */   BC_MULTIPLY,
		/*  68 */   BC_UNASSIGNED,
		/*  69 */   BC_ADD,
		/*  70 */   BC_UNASSIGNED,
		/*  71 */   BC_NUMLOCK,
		/*  72 */   BC_VOLUMEUP,
		/*  73 */   BC_VOLUMEDOWN,
		/*  74 */   BC_MUTE,
		/*  75 */   BC_DIVIDE,
		/*  76 */   BC_NUMPADENTER,
		/*  77 */   BC_UNASSIGNED,
		/*  78 */   BC_SUBTRACT,
		/*  79 */   BC_UNASSIGNED,
		/*  80 */   BC_UNASSIGNED,
		/*  81 */   BC_NUMPADEQUALS,
		/*  82 */   BC_NUMPAD0,
		/*  83 */   BC_NUMPAD1,
		/*  84 */   BC_NUMPAD2,
		/*  85 */   BC_NUMPAD3,
		/*  86 */   BC_NUMPAD4,
		/*  87 */   BC_NUMPAD5,
		/*  88 */   BC_NUMPAD6,
		/*  89 */   BC_NUMPAD7,
		/*  90 */   BC_UNASSIGNED,
		/*  91 */   BC_NUMPAD8,
		/*  92 */   BC_NUMPAD9,
		/*  93 */   BC_CONVERT,
		/*  94 */   BC_NOCONVERT,
		/*  95 */   BC_NUMPADCOMMA,
		/*  96 */   BC_F5,
		/*  97 */   BC_F6,
		/*  98 */   BC_F7,
		/*  99 */   BC_F3,
		/* 100 */   BC_F8,
		/* 101 */   BC_F9,
		/* 102 */   BC_UNASSIGNED,
		/* 103 */   BC_F11,
		/* 104 */   BC_UNASSIGNED,
		/* 105 */   BC_UNASSIGNED,
		/* 106 */   BC_UNASSIGNED,
		/* 107 */   BC_SCROLL,
		/* 108 */   BC_UNASSIGNED,
		/* 109 */   BC_F10,
		/* 110 */   BC_UNASSIGNED,
		/* 111 */   BC_F12,
		/* 112 */   BC_UNASSIGNED,
		/* 113 */   BC_PAUSE,
		/* 114 */   BC_INSERT,
		/* 115 */   BC_HOME,
		/* 116 */   BC_PGUP,
		/* 117 */   BC_DELETE,
		/* 118 */   BC_F4,
		/* 119 */   BC_END,
		/* 120 */   BC_F2,
		/* 121 */   BC_PGDOWN,
		/* 122 */   BC_F1,
		/* 123 */   BC_LEFT,
		/* 124 */   BC_RIGHT,
		/* 125 */   BC_DOWN,
		/* 126 */   BC_UP,
		/* 127 */   BC_POWER
	};

	static UINT32 ButtonCodeToKeyCode[255];
	static void initKeyCodeMapping()
	{
		memset(ButtonCodeToKeyCode, 0, sizeof(ButtonCodeToKeyCode));

		UINT32 numKeyCodes = sizeof(KeyCodeMapping) / sizeof(KeyCodeMapping[0]);

		for(UINT32 i = 0; i < numKeyCodes; i++)
			ButtonCodeToKeyCode[KeyCodeMapping[i]] = i;
	}

	void flipY(NSScreen* screen, NSRect& rect)
	{
		NSRect screenFrame = [screen frame];

		rect.origin.y = screenFrame.size.height - (rect.origin.y + rect.size.height);
	}

	void flipY(NSScreen* screen, NSPoint &point)
	{
		NSRect screenFrame = [screen frame];

		point.y = screenFrame.size.height - point.y;
	}

	void flipYWindow(NSWindow* window, NSPoint &point)
	{
		NSRect windowFrame = [window frame];

		point.y = windowFrame.size.height - point.y;
	}

	/** Returns the name of the current application based on the information in the app. bundle. */
	static NSString* getAppName()
	{
		NSString* appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
		if (!appName)
			appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];

		if (![appName length]) {
			appName = [[NSProcessInfo processInfo] processName];
		}

		return appName;
	}

	/** Creates the default menu for the application menu bar. */
	static void createApplicationMenu()
	{ @autoreleasepool {
		NSMenu* mainMenu = [[NSMenu alloc] init];
		[NSApp setMainMenu:mainMenu];

		NSString* appName = getAppName();
		NSMenu* appleMenu = [[NSMenu alloc] initWithTitle:@""];

		NSString* aboutTitle = [@"About " stringByAppendingString:appName];
		[appleMenu addItemWithTitle:aboutTitle
				   action:@selector(orderFrontStandardAboutPanel:)
				   keyEquivalent:@""];

		[appleMenu addItem:[NSMenuItem separatorItem]];

		NSString* hideTitle = [@"Hide " stringByAppendingString:appName];
		[appleMenu addItemWithTitle:hideTitle action:@selector(hide:) keyEquivalent:@"h"];

		NSMenuItem* hideOthersMenuItem = [appleMenu
			addItemWithTitle:@"Hide Others"
			action:@selector(hideOtherApplications:)
			keyEquivalent:@"h"];

		[hideOthersMenuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];

		[appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

		[appleMenu addItem:[NSMenuItem separatorItem]];

		NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
		[appleMenu addItemWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];

		NSMenuItem* appleMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
		[appleMenuItem setSubmenu:appleMenu];
		[[NSApp mainMenu] addItem:appleMenuItem];
	}}

	Event<void(const Vector2I &, const OSPointerButtonStates &)> Platform::onCursorMoved;
	Event<void(const Vector2I &, OSMouseButton button, const OSPointerButtonStates &)> Platform::onCursorButtonPressed;
	Event<void(const Vector2I &, OSMouseButton button, const OSPointerButtonStates &)> Platform::onCursorButtonReleased;
	Event<void(const Vector2I &, const OSPointerButtonStates &)> Platform::onCursorDoubleClick;
	Event<void(InputCommandType)> Platform::onInputCommand;
	Event<void(float)> Platform::onMouseWheelScrolled;
	Event<void(UINT32)> Platform::onCharInput;

	Event<void()> Platform::onMouseCaptureChanged;

	Platform::Pimpl* Platform::mData = bs_new<Platform::Pimpl>();

	Platform::~Platform()
	{
	}

	Vector2I Platform::getCursorPosition()
	{
		Lock lock(mData->cursorMutex);
		return mData->cursorPos;
	}

	void Platform::setCursorPosition(const Vector2I& screenPos)
	{
		[mData->cursorManager setPosition:screenPos];
	}

	void Platform::captureMouse(const RenderWindow& window)
	{
		// Do nothing
	}

	void Platform::releaseMouseCapture()
	{
		// Do nothing
	}

	bool Platform::isPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
	{
		CFArrayRef windowDicts = CGWindowListCopyWindowInfo(
			kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
			kCGNullWindowID);

		if(!windowDicts)
			return nil;

		CocoaWindow* cocoaWindow;
		window.getCustomAttribute("COCOA_WINDOW", &cocoaWindow);

		int32_t requestedWindowNumber = (int32_t)[cocoaWindow->_getPrivateData()->window windowNumber];
		CGPoint point = CGPointMake(screenPos.x, screenPos.y);

		CFIndex numEntries = CFArrayGetCount(windowDicts);
		for(CFIndex i = 0; i < numEntries; i++)
		{
			CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(windowDicts, i);

			CFNumberRef layerRef = (CFNumberRef) CFDictionaryGetValue(dict, kCGWindowLayer);
			if(!layerRef)
				continue;

			// Ignore windows outside of layer 0, as those appear to be desktop elements
			int32_t layer;
			CFNumberGetValue(layerRef, kCFNumberIntType, &layer);

			// Layer 0 appear to be normal windows
			// Layer 3 appear to be floating windows
			// Layer 8 appear to be modal windows
			// Layer 25 appear to be fullscreen windows
			// Note: This is based on experimentation, as no documentation about it exists
			if(layer != 0 && layer != 3 && layer != 8 && layer != 25)
				continue;

			CFDictionaryRef boundsRef = (CFDictionaryRef)CFDictionaryGetValue(dict, kCGWindowBounds);

			CGRect rect;
			CGRectMakeWithDictionaryRepresentation(boundsRef, &rect);

			if(CGRectContainsPoint(rect, point))
			{
				// Windows are ordered front to back intrinsically, so the first one we are within bounds of is the one we want
				CFNumberRef windowNumRef = (CFNumberRef)CFDictionaryGetValue(dict, kCGWindowNumber);
				int32_t windowNumber;
				CFNumberGetValue(windowNumRef, kCGWindowIDCFNumberType, &windowNumber);

				return requestedWindowNumber == windowNumber;
			}
		}

		return false;
	}

	void Platform::hideCursor()
	{
		Lock lock(mData->cursorMutex);

		if(!mData->cursorIsHidden)
		{
			[NSCursor performSelectorOnMainThread:@selector(hide) withObject:nil waitUntilDone:NO];
			mData->cursorIsHidden = true;
		}
	}

	void Platform::showCursor()
	{
		Lock lock(mData->cursorMutex);

		if(mData->cursorIsHidden)
		{
			[NSCursor performSelectorOnMainThread:@selector(unhide) withObject:nil waitUntilDone:NO];
			mData->cursorIsHidden = false;
		}
	}

	bool Platform::isCursorHidden()
	{
		Lock lock(mData->cursorMutex);

		return mData->cursorIsHidden;
	}

	void Platform::clipCursorToWindow(const RenderWindow& window)
	{
		CocoaWindow* cocoaWindow;
		window.getCustomAttribute("COCOA_WINDOW", &cocoaWindow);

		[mData->cursorManager
			performSelectorOnMainThread:@selector(clipCursorToWindow:)
			withObject:[NSValue valueWithPointer:cocoaWindow]
			waitUntilDone:NO];
	}

	void Platform::clipCursorToRect(const Rect2I& screenRect)
	{
		[mData->cursorManager
			performSelectorOnMainThread:@selector(clipCursorToRect:)
			withObject:[NSValue value:&screenRect withObjCType:@encode(Rect2I)]
			waitUntilDone:NO];
	}

	void Platform::clipCursorDisable()
	{
		[mData->cursorManager
			performSelectorOnMainThread:@selector(clipCursorDisable)
			withObject:nil
			waitUntilDone:NO];
	}

	void Platform::setCursor(PixelData& pixelData, const Vector2I& hotSpot)
	{ @autoreleasepool {
		NSImage* image = MacOSPlatform::createNSImage(pixelData);
		NSPoint point = NSMakePoint(hotSpot.x, hotSpot.y);

		NSCursor* cursor = [[NSCursor alloc] initWithImage:image hotSpot:point];
		NSArray* params = @[cursor, [NSValue valueWithPoint:point]];

		[mData->cursorManager
			performSelectorOnMainThread:@selector(setCursor:) withObject:params waitUntilDone:NO];
	}}

	void Platform::setIcon(const PixelData& pixelData)
	{ @autoreleasepool {
		NSImage* image = MacOSPlatform::createNSImage(pixelData);

		[NSApp performSelectorOnMainThread:@selector(setApplicationIconImage:) withObject:image waitUntilDone:NO];
	}}

	void Platform::setCaptionNonClientAreas(const ct::RenderWindow& window, const Vector<Rect2I>& nonClientAreas)
	{ @autoreleasepool {
		NSMutableArray* params = [[NSMutableArray alloc] init];

		UINT32 windowId;
		window.getCustomAttribute("WINDOW_ID", &windowId);

		NSValue* windowIdValue = [NSValue valueWithBytes:&windowId objCType:@encode(UINT32)];

		[params addObject:windowIdValue];
		for(auto& entry : nonClientAreas)
			[params addObject:[NSValue value:&entry withObjCType:@encode(bs::Rect2I)]];

		[mData->platformManager
			performSelectorOnMainThread:@selector(setCaptionNonClientAreas:)
			withObject:params
			waitUntilDone:NO];
	}}

	void Platform::setResizeNonClientAreas(const ct::RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
	{
		// Do nothing, custom resize areas not needed on MacOS
	}

	void Platform::resetNonClientAreas(const ct::RenderWindow& window)
	{
		UINT32 windowId;
		window.getCustomAttribute("WINDOW_ID", &windowId);

		NSValue* windowIdValue = [NSValue valueWithBytes:&windowId objCType:@encode(UINT32)];

		[mData->platformManager
			performSelectorOnMainThread:@selector(resetNonClientAreas:)
			withObject:windowIdValue
			waitUntilDone:NO];
	}

	void Platform::sleep(UINT32 duration)
	{
		usleep(duration * 1000);
	}

	void Platform::copyToClipboard(const String& string)
	{ @autoreleasepool {
		NSString* text = [NSString stringWithUTF8String:string.c_str()];
		[mData->platformManager performSelectorOnMainThread:@selector(setClipboardText:)
			withObject:text
			waitUntilDone:NO];
	}}

	String Platform::copyFromClipboard()
	{
		Lock lock(mData->clipboardMutex);
		return mData->cachedClipboardData;
	}

	String Platform::keyCodeToUnicode(UINT32 buttonCode)
	{
		UINT32 keyCode = ButtonCodeToKeyCode[buttonCode];

		TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
		if(!currentKeyboard)
			return "";

		auto layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);

		CFRelease(currentKeyboard);

		if(!layoutData)
		{
			currentKeyboard = TISCopyCurrentASCIICapableKeyboardInputSource();
			layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
			CFRelease(currentKeyboard);
		}

		if(!layoutData)
			return "";

		auto keyLayout = (const UCKeyboardLayout*)CFDataGetBytePtr(layoutData);

		UINT32 keysDown = 0;
		UniChar chars[4];
		UniCharCount length = 0;

		UCKeyTranslate(
			keyLayout,
			(unsigned short)keyCode,
			kUCKeyActionDisplay,
			0,
			LMGetKbdType(),
			kUCKeyTranslateNoDeadKeysBit,
			&keysDown,
			sizeof(chars) / sizeof(chars[0]),
			&length,
			chars);

		U16String u16String((char16_t*)chars, (size_t)length);
		String utf8String = UTF8::fromUTF16(u16String);

		return utf8String;
	}

	void Platform::openFolder(const Path& path)
	{
		String pathStr = path.toString();

		NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:pathStr.c_str()]];
		[mData->platformManager
			performSelectorOnMainThread:@selector(openFolder:)
			withObject:url
			waitUntilDone:NO];
	}

	void Platform::_startUp()
	{
		initKeyCodeMapping();

		mData->appDelegate = [[BSAppDelegate alloc] init];
		mData->cursorManager = [[BSCursor alloc] initWithPlatformData:mData];
		mData->platformManager = [[BSFPlatform alloc] initWithPlatformData:mData];
		[BSApplication sharedApplication];

		[NSApp setDelegate:mData->appDelegate];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

		createApplicationMenu();

		[NSApp finishLaunching];
	}

	void Platform::_update()
	{
		CocoaDragAndDrop::update();

		{
			Lock lock(mData->cursorMutex);
			mData->cursorPos = [mData->cursorManager getPosition];
		}

		INT32 changeCount = [mData->platformManager getClipboardChangeCount];
		if(mData->clipboardChangeCount != changeCount)
		{
			NSString* string = [mData->platformManager getClipboardText];
			String utf8String;

			if(string)
				utf8String = [string UTF8String];

			{
				Lock lock(mData->clipboardMutex);
				mData->cachedClipboardData = utf8String;
			}

			mData->clipboardChangeCount = changeCount;
		}

		_messagePump();
	}

	void Platform::_coreUpdate()
	{
		// Do nothing
	}

	void Platform::_shutDown()
	{
		// Do nothing
	}

	void Platform::_messagePump()
	{ @autoreleasepool {
		while(true)
		{
			if(!mData->modalWindows.empty())
			{
				NSModalSession session = mData->modalWindows.back().session;
				[NSApp runModalSession:session];
				break;
			}
			else
			{
				NSEvent* event = [NSApp
					nextEventMatchingMask:NSEventMaskAny
					untilDate:[NSDate distantPast]
					inMode:NSDefaultRunLoopMode
					dequeue:YES];

				if (!event)
					break;

				[NSApp sendEvent:event];
			}
		}
	}}

	void MacOSPlatform::registerWindow(CocoaWindow* window)
	{
		// First window is assumed to be main
		if(!mData->mainWindow)
			mData->mainWindow = window;

		CocoaWindow::Pimpl* windowData = window->_getPrivateData();
		if(windowData->isModal)
		{
			ModalWindowInfo info = { window->_getWindowId(), windowData->modalSession };
			mData->modalWindows.push_back(info);
		}

		Lock lock(mData->windowMutex);
		mData->allWindows[window->_getWindowId()] = window;
	}

	void MacOSPlatform::unregisterWindow(CocoaWindow* window)
	{
		CocoaWindow::Pimpl* windowData = window->_getPrivateData();
		if(windowData->isModal)
		{
			UINT32 windowId = window->_getWindowId();
			auto iterFind = std::find_if(mData->modalWindows.begin(), mData->modalWindows.end(),
										 [windowId](const ModalWindowInfo& x)
										 {
											 return x.windowId == windowId;
										 });

			if(iterFind != mData->modalWindows.end())
				mData->modalWindows.erase(iterFind);
		}

		Lock lock(mData->windowMutex);
		mData->allWindows.erase(window->_getWindowId());

		[mData->cursorManager unregisterWindow:windowData->window];

		// Shut down app when the main window is closed
		if(mData->mainWindow == window)
		{
			bs::gCoreApplication().quitRequested();
			mData->mainWindow = nullptr;
		}
	}

	void MacOSPlatform::lockWindows()
	{
		mData->windowMutex.lock();
	}

	void MacOSPlatform::unlockWindows()
	{
		mData->windowMutex.unlock();
	}

	CocoaWindow* MacOSPlatform::getWindow(UINT32 windowId)
	{
		auto iterFind = mData->allWindows.find(windowId);
		if(iterFind == mData->allWindows.end())
			return nullptr;

		return iterFind->second;
	}

	NSImage* MacOSPlatform::createNSImage(const PixelData& data)
	{
		// Premultiply alpha
		Vector<Color> colors = data.getColors();
		for(auto& color : colors)
		{
			color.r *= color.a;
			color.g *= color.a;
			color.b *= color.a;
		}

		// Convert to RGBA
		SPtr<PixelData> rgbaData = PixelData::create(data.getWidth(), data.getHeight(), 1, PF_RGBA8);
		rgbaData->setColors(colors);
		@autoreleasepool
		{
			INT32 pitch = data.getWidth() * sizeof(UINT32);
			NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc]
										  initWithBitmapDataPlanes:nullptr
										  pixelsWide:data.getWidth()
										  pixelsHigh:data.getHeight()
										  bitsPerSample:8
										  samplesPerPixel:4
										  hasAlpha:YES
										  isPlanar:NO
										  colorSpaceName:NSDeviceRGBColorSpace
										  bytesPerRow:pitch
										  bitsPerPixel:32];

			unsigned char* pixels = [imageRep bitmapData];
			memcpy(pixels, rgbaData->getData(), data.getHeight() * pitch);

			NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(data.getWidth(), data.getHeight())];
			[image addRepresentation:imageRep];

			return image;
		}
	}

	void MacOSPlatform::sendInputCommandEvent(InputCommandType inputCommand)
	{
		onInputCommand(inputCommand);
	}

	void MacOSPlatform::sendCharInputEvent(UINT32 character)
	{
		onCharInput(character);
	}

	void MacOSPlatform::sendPointerButtonPressedEvent(
		const Vector2I& pos,
		OSMouseButton button,
		const OSPointerButtonStates& buttonStates)
	{
		onCursorButtonPressed(pos, button, buttonStates);
	}

	void MacOSPlatform::sendPointerButtonReleasedEvent(
		const Vector2I& pos,
		OSMouseButton button,
		const OSPointerButtonStates& buttonStates)
	{
		onCursorButtonReleased(pos, button, buttonStates);
	}

	void MacOSPlatform::sendPointerDoubleClickEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates)
	{
		onCursorDoubleClick(pos, buttonStates);
	}

	void MacOSPlatform::sendPointerMovedEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates)
	{
		onCursorMoved(pos, buttonStates);
	}

	void MacOSPlatform::sendMouseWheelScrollEvent(float delta)
	{
		onMouseWheelScrolled(delta);
	}

	void MacOSPlatform::notifyWindowEvent(bs::WindowEventType type, bs::UINT32 windowId)
	{
		CocoaWindow* window = nullptr;
		{
			auto iterFind = mData->allWindows.find(windowId);
			if(iterFind == mData->allWindows.end())
				return;

			window = iterFind->second;
		}

		auto renderWindow = (RenderWindow*)window->_getUserData();
		if(renderWindow == nullptr)
		{
			// If it's a render window we allow the client code to handle the message, otherwise we just destroy it
			if(type == WindowEventType::CloseRequested)
				window->_destroy();

			return;
		}

		renderWindow->_notifyWindowEvent(type);
	}

	NSCursor* MacOSPlatform::_getCurrentCursor()
	{
		return [mData->cursorManager currentCursor];
	}

	bool MacOSPlatform::_clipCursor(Vector2I& pos)
	{
		return [mData->cursorManager clipCursor:pos];
	}

	void MacOSPlatform::_updateClipBounds(NSWindow* window)
	{
		[mData->cursorManager updateClipBounds:window];
	}

	void MacOSPlatform::_setCursorPosition(const Vector2I& position)
	{
		[mData->cursorManager setPosition:position];
	}
}
