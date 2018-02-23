//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#define BS_COCOA_INTERNALS 1
#include "Private/MacOS/BsMacOSWindow.h"
#include "Private/MacOS/BsMacOSPlatform.h"
#include "Private/MacOS/BsMacOSDropTarget.h"
#include "Math/BsRect2I.h"
#include "Input/BsInputFwd.h"
#include "String/BsUnicode.h"
#include "RenderAPI/BsRenderWindow.h"

using namespace bs;

/** Converts a keycode reported by Cocoa into a potential input command. */
static bool keyCodeToInputCommand(uint32_t keyCode, bool shift, bs::InputCommandType& inputCommand)
{
	switch(keyCode)
	{
		case 36: // Return
			inputCommand = shift ? bs::InputCommandType::Return : bs::InputCommandType::Confirm;
			return true;
		case 51: // Backspace
			inputCommand = bs::InputCommandType::Backspace;
			return true;
		case 53: // Escape
			inputCommand = bs::InputCommandType::Escape;
			return true;
		case 117: // Delete
			inputCommand = bs::InputCommandType::Delete;
			return true;
		case 123: // Left
			inputCommand = shift ? bs::InputCommandType::SelectLeft : bs::InputCommandType::CursorMoveLeft;
			return true;
		case 124: // Right
			inputCommand = shift ? bs::InputCommandType::SelectRight : bs::InputCommandType::CursorMoveRight;
			return true;
		case 125: // Down
			inputCommand = shift ? bs::InputCommandType::SelectDown : bs::InputCommandType::CursorMoveDown;
			return true;
		case 126: // Up
			inputCommand = shift ? bs::InputCommandType::SelectUp : bs::InputCommandType::CursorMoveUp;
			return true;
	}

	return false;
}

/** Implementation of NSView that handles custom cursors, transparent background images and reports the right mouse click. */
@interface BSView : NSView
-(void)rightMouseDown:(NSEvent*) event;
-(void)setBackgroundImage:(NSImage*)image;
@end

@implementation BSView
{
	NSTrackingArea* mTrackingArea;
	NSImageView* mImageView;
}

-(id)init
{
	self = [super init];

	mTrackingArea = nil;
	mImageView = nil;

	return self;
}

-(void)resetCursorRects
{
	[super resetCursorRects];

	[self addCursorRect:[self bounds] cursor:bs::MacOSPlatform::_getCurrentCursor()];
}

-(void)updateTrackingAreas
{
	[super updateTrackingAreas];

	if(mTrackingArea)
		[self removeTrackingArea:mTrackingArea];

	NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp;
	mTrackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:[self window] userInfo:nil];

	[self addTrackingArea:mTrackingArea];
}

-(void)rightMouseDown:(NSEvent*)event
{
	// By default the view eats the right mouse event, but we instead forward the event to window's responder for normal
	// handling
	if([event.window nextResponder])
		[[event.window nextResponder] rightMouseDown:event];
}

-(void)setBackgroundImage:(NSImage*)image
{ @autoreleasepool {
	if(image)
	{
		NSRect frame = [self frame];
		frame.origin = NSMakePoint(0, 0);

		mImageView = [[NSImageView alloc] initWithFrame:frame];
		[mImageView setImageScaling:NSImageScaleAxesIndependently];
		[mImageView setImage:image];
		[mImageView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

		self.subviews = @[mImageView];
	}
	else
		self.subviews = @[];
}}
@end

@class BSWindow;

/** Types of mouse events reported by BSWindowListener. */
enum class MouseEventType
{
	ButtonUp, ButtonDown
};

/** Listens to mouse and keyboard events for a specific window. Reports them to Platform accordingly. */
@interface BSWindowListener : NSResponder

// Properties
@property (atomic, strong) NSArray* dragAreas;

// Mouse
-(void) handleMouseEvent:(NSEvent *) event type:(MouseEventType) type button:(bs::OSMouseButton) button;
-(void) mouseDown:(NSEvent *) event;
-(void) rightMouseDown:(NSEvent *) event;
-(void) otherMouseDown:(NSEvent *) event;
-(void) mouseUp:(NSEvent *) event;
-(void) rightMouseUp:(NSEvent *) event;
-(void) otherMouseUp:(NSEvent *) event;
-(void) mouseMoved:(NSEvent *) event;
-(void) mouseDragged:(NSEvent *) event;
-(void) rightMouseDragged:(NSEvent *) event;
-(void) otherMouseDragged:(NSEvent *) event;
-(void) scrollWheel:(NSEvent *) event;

// Keyboard
-(void) keyDown:(NSEvent *)event;

// Other
-(id)initWithWindow:(BSWindow*) window;
@end

/** Listens to window move, resize, focus change and close events, handles drag and drop operations. */
@interface BSWindowDelegate : NSObject<NSWindowDelegate, NSDraggingDestination>
-(id)initWithWindow:(BSWindow*) window;
@end

/**
 * Overrides window so even borderless windows can become key windows (allowing resize events and cursor changes, among
 * others.
 */
@interface BSWindow : NSWindow
@property(atomic,assign) UINT32 WindowId;
@end

@implementation BSWindow

- (BOOL)canBecomeKeyWindow
{
	return YES;
}
@end

@implementation BSWindowListener
{
	BSWindow* mWindow;
}
@synthesize dragAreas;

- (id)initWithWindow:(BSWindow*) window
{
	self = [super init];

	mWindow = window;
	dragAreas = nil;

	return self;
}

- (void)handleMouseEvent:(NSEvent*) event type:(MouseEventType) type button:(bs::OSMouseButton) button
{
	NSPoint screenPos = NSEvent.mouseLocation;
	NSUInteger modifierFlags = NSEvent.modifierFlags;
	uint32_t pressedButtons = (uint32_t)NSEvent.pressedMouseButtons;

	bs::OSPointerButtonStates buttonStates;
	buttonStates.ctrl = (modifierFlags & NSEventModifierFlagControl) != 0;
	buttonStates.shift = (modifierFlags & NSEventModifierFlagShift) != 0;
	buttonStates.mouseButtons[0] = (pressedButtons & (1 << 0)) != 0;
	buttonStates.mouseButtons[1] = (pressedButtons & (1 << 1)) != 0;
	buttonStates.mouseButtons[2] = (pressedButtons & (1 << 2)) != 0;

	NSWindow* window = [event window];
	NSScreen* screen = window ? [window screen] : [NSScreen mainScreen];

	bs::flipY(screen, screenPos);
	bs::Vector2I pos((int32_t)screenPos.x, (int32_t)screenPos.y);

	if(type == MouseEventType::ButtonDown)
		bs::MacOSPlatform::sendPointerButtonPressedEvent(pos, button, buttonStates);
	else // ButtonUp
	{
		if([event clickCount] == 2 && button == bs::OSMouseButton::Left)
			bs::MacOSPlatform::sendPointerDoubleClickEvent(pos, buttonStates);
		else
			bs::MacOSPlatform::sendPointerButtonReleasedEvent(pos, button, buttonStates);
	}
}

- (void)mouseDown:(NSEvent *)event
{
	// Check for manual drag
	bool isManualDrag = false;
	if(dragAreas)
	{
		NSPoint point = [event locationInWindow];
		NSWindow* window = [event window];
		NSScreen* screen = nil;
		if(window)
		{
			NSRect windowFrame = [window frame];
			point.x += windowFrame.origin.x;
			point.y += windowFrame.origin.y;

			screen = [window screen];
		}
		else
			screen = NSScreen.mainScreen;

		bs::flipY(screen, point);

		for (NSUInteger i = 0; i < [dragAreas count]; i++)
		{
			bs::Rect2I rect;
			[dragAreas[i] getValue:&rect];

			if(point.x >= rect.x && point.x < (rect.x + rect.width) &&
					(point.y >= rect.y && point.y < (rect.y + rect.height)))
			{
				[window performWindowDragWithEvent:event];
				isManualDrag = true;
				break;
			}
		}
	}

	if(!isManualDrag)
		[self handleMouseEvent:event type:MouseEventType::ButtonDown button:bs::OSMouseButton::Left];
}

- (void)rightMouseDown:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonDown button:bs::OSMouseButton::Right];
}

- (void)otherMouseDown:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonDown button:bs::OSMouseButton::Middle];
}

- (void)mouseUp:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonUp button:bs::OSMouseButton::Left];
}

- (void)rightMouseUp:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonUp button:bs::OSMouseButton::Right];
}

- (void)otherMouseUp:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonUp button:bs::OSMouseButton::Middle];
}

- (void)mouseMoved:(NSEvent *)event
{
	uint32_t pressedButtons = (uint32_t)NSEvent.pressedMouseButtons;

	NSPoint point = [event locationInWindow];
	NSWindow* window = [event window];
	NSScreen* screen = nil;
	if(window)
	{
		NSRect windowFrame = [window frame];
		point.x += windowFrame.origin.x;
		point.y += windowFrame.origin.y;

		screen = [window screen];
	}
	else
		screen = NSScreen.mainScreen;

	bs::flipY(screen, point);

	bs::Vector2I pos;
	pos.x = (int32_t)point.x;
	pos.y = (int32_t)point.y;

	if(bs::MacOSPlatform::_clipCursor(pos))
		bs::MacOSPlatform::_setCursorPosition(pos);

	NSUInteger modifierFlags = NSEvent.modifierFlags;

	bs::OSPointerButtonStates buttonStates;
	buttonStates.ctrl = (modifierFlags & NSEventModifierFlagControl) != 0;
	buttonStates.shift = (modifierFlags & NSEventModifierFlagShift) != 0;
	buttonStates.mouseButtons[0] = (pressedButtons & (1 << 0)) != 0;
	buttonStates.mouseButtons[1] = (pressedButtons & (1 << 1)) != 0;
	buttonStates.mouseButtons[2] = (pressedButtons & (1 << 2)) != 0;

	bs::MacOSPlatform::sendPointerMovedEvent(pos, buttonStates);
}

- (void)mouseDragged:(NSEvent *)event
{
	[self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent *)event
{
	[self mouseMoved:event];
}

- (void)otherMouseDragged:(NSEvent *)event
{
	[self mouseMoved:event];
}

- (void)scrollWheel:(NSEvent *)event
{
	float y = (float)[event deltaY];

	bs::MacOSPlatform::sendMouseWheelScrollEvent((float)y);
}

- (void)keyDown:(NSEvent *)event
{
	NSString* string = event.characters;
	uint32_t keyCode = event.keyCode;

	NSUInteger modifierFlags = NSEvent.modifierFlags;
	bool shift = (modifierFlags & NSEventModifierFlagShift) != 0;

	bs::InputCommandType ict;
	if(keyCodeToInputCommand(keyCode, shift, ict))
		bs::MacOSPlatform::sendInputCommandEvent(ict);
	else
	{
		bs::String utf8String = [string UTF8String];
		bs::U32String utf32String = bs::UTF8::toUTF32(utf8String);

		for(size_t i = 0; utf32String.length(); i++)
			bs::MacOSPlatform::sendCharInputEvent(utf32String[i]);
	}
}

- (void)mouseEntered:(NSEvent*)event
{
	// Do nothing
}

- (void)mouseExited:(NSEvent*)event
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::MouseLeft, mWindow.WindowId);
}
@end

/** Converts a point from coordinates relative to window's frame, into coordinates relative to window's content rectangle. */
NSPoint frameToContentRect(NSWindow* window, NSPoint framePoint)
{
	bs::flipYWindow(window, framePoint);

	NSRect frameRect = [window frame];
	NSRect contentRect = [window contentRectForFrameRect:frameRect];

	NSPoint offset;
	offset.x = frameRect.origin.x - contentRect.origin.x;
	offset.y = (frameRect.origin.y + frameRect.size.height) - (contentRect.origin.y + contentRect.size.height);

	framePoint.x -= offset.x;
	framePoint.y -= offset.y;

	return framePoint;
}

@implementation BSWindowDelegate
{
	BSWindow* mWindow;
	NSRect mStandardZoomFrame;
	bool mIsZooming;
}

- (id)initWithWindow:(BSWindow*)window
{
	self = [super init];

	mWindow = window;
	mIsZooming = false;
	return self;
}

- (void)windowWillClose:(NSNotification *)notification
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::CloseRequested, mWindow.WindowId);
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::FocusReceived, mWindow.WindowId);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::FocusLost, mWindow.WindowId);
}

- (void)windowDidResize:(NSNotification*)notification
{
	if([[notification object] isKindOfClass:[NSWindow class]])
		bs::MacOSPlatform::_updateClipBounds([notification object]);

	MacOSPlatform::notifyWindowEvent(WindowEventType::Resized, mWindow.WindowId);
}

- (void)windowDidMove:(NSNotification*)notification
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::Moved, mWindow.WindowId);
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::Minimized, mWindow.WindowId);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
	MacOSPlatform::notifyWindowEvent(WindowEventType::Restored, mWindow.WindowId);
}

- (BOOL)windowShouldZoom:(NSWindow*)window toFrame:(NSRect)newFrame
{
	// Maximizing, or restoring
	if(mIsZooming)
	{
		if (newFrame.size.width == mStandardZoomFrame.size.width &&
			newFrame.size.height == mStandardZoomFrame.size.height)
			MacOSPlatform::notifyWindowEvent(WindowEventType::Maximized, mWindow.WindowId);
		else
			MacOSPlatform::notifyWindowEvent(WindowEventType::Restored, mWindow.WindowId);

		mIsZooming = true;

		NSRect contentRect = [mWindow contentRectForFrameRect:newFrame];
		flipY([mWindow screen], contentRect);

		Rect2I area(
				(INT32)contentRect.origin.x,
				(INT32)contentRect.origin.y,
				(UINT32)contentRect.size.width,
				(UINT32)contentRect.size.height);
	}

	return YES;
}

- (NSRect)windowWillUseStandardFrame:(NSWindow*)window defaultFrame:(NSRect)newFrame
{
	mIsZooming = true;
	mStandardZoomFrame = newFrame;

	return newFrame;
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	NSPoint point = [sender draggingLocation];
	point = frameToContentRect(mWindow, point);

	bs::Vector2I position((int32_t)point.x, (int32_t)point.y);
	if(bs::CocoaDragAndDrop::_notifyDragEntered(mWindow.WindowId, position))
		return NSDragOperationLink;

	return NSDragOperationNone;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	NSPoint point = [sender draggingLocation];
	point = frameToContentRect(mWindow, point);

	bs::Vector2I position((int32_t)point.x, (int32_t)point.y);
	if(bs::CocoaDragAndDrop::_notifyDragMoved(mWindow.WindowId, position))
		return NSDragOperationLink;

	return NSDragOperationNone;
}

- (void)draggingExited:(nullable id <NSDraggingInfo>)sender
{
	bs::CocoaDragAndDrop::_notifyDragLeft(mWindow.WindowId);
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	NSPasteboard* pasteboard = [sender draggingPasteboard];
	if([[pasteboard types] containsObject:NSFilenamesPboardType])
	{
		NSArray* entries = [pasteboard propertyListForType:NSFilenamesPboardType];

		bs::Vector<bs::Path> paths;
		for(NSString* path in entries)
		{
			const char* pathChars = [path UTF8String];
			paths.push_back(bs::Path(pathChars));
		}

		NSPoint point = [sender draggingLocation];
		point = frameToContentRect(mWindow, point);

		bs::Vector2I position((int32_t)point.x, (int32_t)point.y);

		if(bs::CocoaDragAndDrop::_notifyDragDropped(mWindow.WindowId, position, paths))
			return YES;
	}

	return NO;
}
@end

namespace bs
{
	std::atomic<UINT32> gNextWindowId(1);

	CocoaWindow::CocoaWindow(const WINDOW_DESC& desc)
	{ @autoreleasepool {
		m = bs_new<Pimpl>();

		BSWindow* window = [BSWindow alloc];
		m->isModal = desc.modal;
		mWindowId = gNextWindowId++;

		NSArray* screens = [NSScreen screens];

		NSScreen* screen = nil;
		INT32 x = 0;
		INT32 y = 0;

		for(NSScreen* entry in screens)
		{
			NSRect screenRect = [entry frame];

			if(((desc.x >= screenRect.origin.x && desc.y < (screenRect.origin.x + screenRect.size.width)) || desc.x == -1) &&
			   ((desc.y >= screenRect.origin.y && desc.y < (screenRect.origin.y + screenRect.size.height)) || desc.y == -1))
			{
				if(desc.x == -1)
					x = (INT32)screenRect.origin.x + std::max(0, (INT32)screenRect.size.width - (INT32)desc.width) / 2;
				else
					x = desc.x - (INT32)screenRect.origin.x;

				if(desc.y == -1)
					y = (INT32)screenRect.origin.y + std::max(0, (INT32)screenRect.size.height - (INT32)desc.height) / 2;
				else
					y = desc.y - (INT32)screenRect.origin.y;

				screen = entry;
				break;
			}
		}

		if(!desc.showDecorations)
			m->style |= NSWindowStyleMaskBorderless;
		else
			m->style |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

		if(desc.allowResize)
			m->style |= NSWindowStyleMaskResizable;

		window = [window
				initWithContentRect:NSMakeRect(x, y, desc.width, desc.height)
				styleMask:(NSWindowStyleMask)m->style
				backing:NSBackingStoreBuffered
				defer:NO
				screen:screen];
		m->window = window;
		window.WindowId = mWindowId;

		if(desc.allowResize)
		{
			bool allowSpaces = NSAppKitVersionNumber > NSAppKitVersionNumber10_6;
			if(allowSpaces)
				[m->window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		}

		NSString* titleString = [NSString stringWithUTF8String:desc.title.c_str()];

		[m->window setAcceptsMouseMovedEvents:YES];
		[m->window setReleasedWhenClosed:NO];
		[m->window setTitle:titleString];
		[m->window makeKeyAndOrderFront:nil];

		m->responder = [[BSWindowListener alloc] initWithWindow:window];
		[m->window setNextResponder:m->responder];

		m->delegate = [[BSWindowDelegate alloc] initWithWindow:window];
		[m->window setDelegate:m->delegate];

		m->view = [[BSView alloc] init];
		[m->window setContentView:m->view];

		if(desc.background)
		{
			[m->window setAlphaValue:1.0f];
			[m->window setOpaque:NO];
			[m->window setBackgroundColor:[NSColor clearColor]];

			NSImage* image = MacOSPlatform::createNSImage(*desc.background);
			[m->view setBackgroundImage:image];
		}

		m->isFullscreen = false;

		if(desc.modal)
			m->modalSession = [NSApp beginModalSessionForWindow:m->window];

		MacOSPlatform::registerWindow(this);
	}}

	CocoaWindow::~CocoaWindow()
	{
		if(m->window != nil)
			_destroy();

		bs_delete(m);
	}

	void CocoaWindow::move(INT32 x, INT32 y)
	{
		@autoreleasepool
		{
			NSPoint point;
			point.x = x;
			point.y = y;

			[m->window setFrameTopLeftPoint:point];
		}
	}

	void CocoaWindow::resize(UINT32 width, UINT32 height)
	{
		@autoreleasepool
		{
			NSSize size;
			size.width = width;
			size.height = height;

			NSRect frameRect = m->window.frame;
			NSRect contentRect = [m->window contentRectForFrameRect:frameRect];

			contentRect.size.width = size.width;
			contentRect.size.height = size.height;

			[m->window setFrame:[m->window frameRectForContentRect:contentRect] display:YES];
		}
	}

	Rect2I CocoaWindow::getArea() const
	{
		@autoreleasepool
		{
			NSRect frameRect = [m->window frame];
			NSRect contentRect = [m->window contentRectForFrameRect:frameRect];

			flipY([m->window screen], contentRect);

			return Rect2I(
					(INT32)contentRect.origin.x,
					(INT32)contentRect.origin.y,
					(UINT32)contentRect.size.width,
					(UINT32)contentRect.size.height);
		}
	}

	void CocoaWindow::hide()
	{
		@autoreleasepool
		{
			[m->window orderOut:nil];
		}
	}

	void CocoaWindow::show()
	{
		@autoreleasepool
		{
			[m->window makeKeyAndOrderFront:nil];
		}
	}

	void CocoaWindow::maximize()
	{
		@autoreleasepool
		{
			if(![m->window isZoomed])
				[m->window zoom:nil];
		}
	}

	void CocoaWindow::minimize()
	{
		@autoreleasepool
		{
			[m->window miniaturize:nil];
		}
	}

	void CocoaWindow::restore()
	{
		@autoreleasepool
		{
			if([m->window isMiniaturized])
				[m->window deminiaturize:nil];
			else if([m->window isZoomed])
				[m->window zoom:nil];
		}
	}

	void CocoaWindow::setWindowed()
	{
		@autoreleasepool
		{
			if(m->isFullscreen)
			{
				[m->window setStyleMask:(NSWindowStyleMask)m->style];
				[m->window setFrame:m->windowedRect display:NO];
				[m->window setLevel:NSNormalWindowLevel];

				m->isFullscreen = false;
			}
		}
	}

	void CocoaWindow::setFullscreen()
	{
		@autoreleasepool
		{
			if(!m->isFullscreen)
				m->windowedRect = [m->window frame];

			NSRect frame = [[m->window screen] frame];
			[m->window setStyleMask:NSWindowStyleMaskBorderless];
			[m->window setFrame:frame display:NO];
			[m->window setLevel:NSMainMenuWindowLevel+1];
			[m->window makeKeyAndOrderFront:nil];

			m->isFullscreen = true;
		}
	}

	Vector2I CocoaWindow::windowToScreenPos(const Vector2I& windowPos) const
	{
		NSRect frameRect = [m->window frame];
		NSRect contentRect = [m->window contentRectForFrameRect:frameRect];

		flipY([m->window screen], contentRect);

		Vector2I screenPos;
		screenPos.x = windowPos.x + (INT32)contentRect.origin.x;
		screenPos.y = windowPos.y + (INT32)contentRect.origin.y;

		return screenPos;
	}

	Vector2I CocoaWindow::screenToWindowPos(const Vector2I& screenPos) const
	{
		NSRect frameRect = [m->window frame];
		NSRect contentRect = [m->window contentRectForFrameRect:frameRect];

		flipY([m->window screen], contentRect);

		Vector2I windowPos;
		windowPos.x = screenPos.x - (INT32) contentRect.origin.x;
		windowPos.y = screenPos.y - (INT32) contentRect.origin.y;

		return windowPos;
	}

	void CocoaWindow::_destroy()
	{
		if(m->isModal)
			[NSApp endModalSession:m->modalSession];

		// TODO - Need to unregister modal window as well
		MacOSPlatform::unregisterWindow(this);

		m->window = nil;
	}

	void CocoaWindow::_setDragZones(const Vector<Rect2I>& rects)
	{
		@autoreleasepool
		{
			NSMutableArray* array = [[NSMutableArray alloc] init];

			for(auto& entry : rects)
				[array addObject:[NSValue valueWithBytes:&entry objCType:@encode(Rect2I)]];

			[m->responder setDragAreas:array];
		}
	}

	void CocoaWindow::_setUserData(void* data)
	{
		m->userData = data;
	}

	void* CocoaWindow::_getUserData() const
	{
		return m->userData;
	}

	void CocoaWindow::_registerForDragAndDrop()
	{
		if(m->numDropTargets == 0)
			[m->window registerForDraggedTypes:@[NSFilenamesPboardType]];
	}

	void CocoaWindow::_unregisterForDragAndDrop()
	{
		if(m->numDropTargets == 0)
			return;

		m->numDropTargets--;

		if(m->numDropTargets == 0)
			[m->window unregisterDraggedTypes];
	}
}
