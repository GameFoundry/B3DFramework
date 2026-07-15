//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define BS_COCOA_INTERNALS 1
#define GL_SILENCE_DEPRECATION 1
#include "Private/MacOS/B3DMacOSWindow.h"
#include "Private/MacOS/B3DMacOSPlatform.h"
#include "Private/MacOS/B3DMacOSDropTarget.h"
#include "Math/B3DArea2.h"
#include "Input/B3DInputFwd.h"
#include "String/B3DUnicode.h"
#include "GpuBackend/B3DRenderWindow.h"

#import <QuartzCore/CAMetalLayer.h>

using namespace b3d;

/** Converts a keycode reported by Cocoa into a potential input command. */
static bool keyCodeToInputCommand(uint32_t keyCode, bool shift, b3d::InputCommandType& inputCommand)
{
	switch(keyCode)
	{
		case 36: // Return
			inputCommand = shift ? b3d::InputCommandType::Return : b3d::InputCommandType::Confirm;
			return true;
		case 51: // Backspace
			inputCommand = b3d::InputCommandType::Backspace;
			return true;
		case 53: // Escape
			inputCommand = b3d::InputCommandType::Escape;
			return true;
		case 117: // Delete
			inputCommand = b3d::InputCommandType::Delete;
			return true;
		case 123: // Left
			inputCommand = shift ? b3d::InputCommandType::SelectLeft : b3d::InputCommandType::CursorMoveLeft;
			return true;
		case 124: // Right
			inputCommand = shift ? b3d::InputCommandType::SelectRight : b3d::InputCommandType::CursorMoveRight;
			return true;
		case 125: // Down
			inputCommand = shift ? b3d::InputCommandType::SelectDown : b3d::InputCommandType::CursorMoveDown;
			return true;
		case 126: // Up
			inputCommand = shift ? b3d::InputCommandType::SelectUp : b3d::InputCommandType::CursorMoveUp;
			return true;
	}

	return false;
}

/** Implementation of NSView that handles custom cursors, transparent background images and reports the right mouse click. */
@interface BSView : NSView
-(void)rightMouseDown:(NSEvent*) event;
-(void)setBackgroundImage:(NSImage*)image;
-(void)setGLContext:(NSOpenGLContext*)context;
@end

@implementation BSView
{
	NSTrackingArea* mTrackingArea;
	NSImageView* mImageView;
	NSOpenGLContext* mGLContext;
}

-(id)init
{
	self = [super init];

	mTrackingArea = nil;
	mImageView = nil;
	mGLContext = nil;

	return self;
}

-(void)setLayer:(CALayer*)layer
{
	[super setLayer:layer];

	if(mGLContext)
		[mGLContext update];
}

-(void)setGLContext:(NSOpenGLContext*)context
{
	mGLContext = context;
}

-(void)resetCursorRects
{
	[super resetCursorRects];

	[self addCursorRect:[self bounds] cursor:b3d::MacOSPlatform::GetCurrentCursorInternal()];
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

-(void)mouseUp:(NSEvent*)event
{
	// After a fullscreen switch the view starts eating the mouseUp event, so we instead forward the event to window's
	// responder for normal handling
	if([event.window nextResponder])
		[[event.window nextResponder] mouseUp:event];
}

-(void)rightMouseDown:(NSEvent*)event
{
	// By default the view eats the right mouse event, but we instead forward the event to window's responder for normal
	// handling
	if([event.window nextResponder])
		[[event.window nextResponder] rightMouseDown:event];
}

-(BOOL)acceptsFirstMouse:(NSEvent*)theEvent
{
	// Ensures that first mouse event when user hasn't yet focused the window, is received
	return YES;
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
-(void) handleMouseEvent:(NSEvent *) event type:(MouseEventType) type button:(b3d::OSMouseButton) button;
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
@property(atomic,assign) b3d::u32 WindowId;
@end

@implementation BSWindow

- (BOOL)canBecomeKeyWindow
{
	return YES;
}
@end

@implementation BSWindowListener
{
	__weak BSWindow* mWindow;
}
@synthesize dragAreas;

- (id)initWithWindow:(BSWindow*) window
{
	self = [super init];

	mWindow = window;
	dragAreas = nil;

	return self;
}

- (void)handleMouseEvent:(NSEvent*) event type:(MouseEventType) type button:(b3d::OSMouseButton) button
{
	NSPoint screenPos = NSEvent.mouseLocation;
	NSUInteger modifierFlags = NSEvent.modifierFlags;
	uint32_t pressedButtons = (uint32_t)NSEvent.pressedMouseButtons;

	b3d::OSPointerButtonStates buttonStates;
	buttonStates.Ctrl = (modifierFlags & NSEventModifierFlagControl) != 0;
	buttonStates.Shift = (modifierFlags & NSEventModifierFlagShift) != 0;
	buttonStates.MouseButtons[0] = (pressedButtons & (1 << 0)) != 0;
	buttonStates.MouseButtons[1] = (pressedButtons & (1 << 2)) != 0;
	buttonStates.MouseButtons[2] = (pressedButtons & (1 << 1)) != 0;

	NSWindow* window = [event window];
	NSScreen* screen = window ? [window screen] : [NSScreen mainScreen];

	b3d::flipY(screen, screenPos);
	b3d::Vector2I pos((int32_t)screenPos.x, (int32_t)screenPos.y);

	if(type == MouseEventType::ButtonDown)
		b3d::MacOSPlatform::SendPointerButtonPressedEvent(pos, button, buttonStates);
	else // ButtonUp
	{
		if([event clickCount] == 2 && button == b3d::OSMouseButton::Left)
			b3d::MacOSPlatform::SendPointerDoubleClickEvent(pos, buttonStates);
		else
			b3d::MacOSPlatform::SendPointerButtonReleasedEvent(pos, button, buttonStates);
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

		if(window)
		{
			NSRect windowFrame = [window frame];
			point.y = windowFrame.size.height - point.y;
		}

		for (NSUInteger dragAreaIndex = 0; dragAreaIndex < [dragAreas count]; dragAreaIndex++)
		{
			b3d::Area2I area;
			[dragAreas[dragAreaIndex] getValue:&area];

			if(point.x >= area.X && point.x < (area.X + (b3d::i32)area.Width) &&
					(point.y >= area.Y && point.y < (area.Y + (b3d::i32)area.Height)))
			{
				[window performWindowDragWithEvent:event];
				isManualDrag = true;
				break;
			}
		}
	}

	if(!isManualDrag)
		[self handleMouseEvent:event type:MouseEventType::ButtonDown button:b3d::OSMouseButton::Left];
}

- (void)rightMouseDown:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonDown button:b3d::OSMouseButton::Right];
}

- (void)otherMouseDown:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonDown button:b3d::OSMouseButton::Middle];
}

- (void)mouseUp:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonUp button:b3d::OSMouseButton::Left];
}

- (void)rightMouseUp:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonUp button:b3d::OSMouseButton::Right];
}

- (void)otherMouseUp:(NSEvent *)event
{
	[self handleMouseEvent:event type:MouseEventType::ButtonUp button:b3d::OSMouseButton::Middle];
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

	b3d::flipY(screen, point);

	b3d::Vector2I pos;
	pos.X = (int32_t)point.x;
	pos.Y = (int32_t)point.y;

	if(b3d::MacOSPlatform::ClipCursorInternal(pos))
		b3d::MacOSPlatform::SetCursorPositionInternal(pos);

	NSUInteger modifierFlags = NSEvent.modifierFlags;

	b3d::OSPointerButtonStates buttonStates;
	buttonStates.Ctrl = (modifierFlags & NSEventModifierFlagControl) != 0;
	buttonStates.Shift = (modifierFlags & NSEventModifierFlagShift) != 0;
	buttonStates.MouseButtons[0] = (pressedButtons & (1 << 0)) != 0;
	buttonStates.MouseButtons[1] = (pressedButtons & (1 << 1)) != 0;
	buttonStates.MouseButtons[2] = (pressedButtons & (1 << 2)) != 0;

	b3d::MacOSPlatform::SendPointerMovedEvent(pos, buttonStates);
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

	b3d::MacOSPlatform::SendMouseWheelScrollEvent((float)y);
}

- (void)keyDown:(NSEvent *)event
{
	NSString* string = event.characters;
	uint32_t keyCode = event.keyCode;

	NSUInteger modifierFlags = NSEvent.modifierFlags;
	bool shift = (modifierFlags & NSEventModifierFlagShift) != 0;
	bool control = (modifierFlags & NSEventModifierFlagControl) != 0;
	bool command = (modifierFlags & NSEventModifierFlagCommand) != 0;

	if(!control && !command)
	{
		b3d::InputCommandType ict;
		if (keyCodeToInputCommand(keyCode, shift, ict))
			b3d::MacOSPlatform::SendInputCommandEvent(ict);
		else
		{
			const char* chars = [string UTF8String];

			b3d::String utf8String(chars);
			b3d::U32String utf32String = b3d::UTF8::ToUtF32(utf8String);

			for (size_t characterIndex = 0; characterIndex < utf32String.length(); characterIndex++)
				b3d::MacOSPlatform::SendCharInputEvent(utf32String[characterIndex]);
		}
	}
}

- (void)mouseEntered:(NSEvent*)event
{
	// Do nothing
}

- (void)mouseExited:(NSEvent*)event
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::MouseLeft, mWindow.WindowId);
}
@end

/** Converts a point from coordinates relative to window's frame, into coordinates relative to window's content rectangle. */
NSPoint frameToContentRect(NSWindow* window, NSPoint framePoint)
{
	b3d::flipYWindow(window, framePoint);

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
	__weak BSWindow* mWindow;
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
	// Do nothing
}

- (BOOL)windowShouldClose:(id)sender
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::CloseRequested, mWindow.WindowId);
	return NO;
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::FocusReceived, mWindow.WindowId);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::FocusLost, mWindow.WindowId);
}

- (void)windowDidResize:(NSNotification*)notification
{
	if([[notification object] isKindOfClass:[NSWindow class]])
		b3d::MacOSPlatform::UpdateClipBoundsInternal([notification object]);

	MacOSPlatform::NotifyWindowEvent(WindowEventType::Resized, mWindow.WindowId);
}

- (void)windowDidMove:(NSNotification*)notification
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::Moved, mWindow.WindowId);
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::Minimized, mWindow.WindowId);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
	MacOSPlatform::NotifyWindowEvent(WindowEventType::Restored, mWindow.WindowId);
}

- (BOOL)windowShouldZoom:(NSWindow*)window toFrame:(NSRect)newFrame
{
	// Maximizing, or restoring
	if(mIsZooming)
	{
		if (newFrame.size.width == mStandardZoomFrame.size.width &&
			newFrame.size.height == mStandardZoomFrame.size.height)
			MacOSPlatform::NotifyWindowEvent(WindowEventType::Maximized, mWindow.WindowId);
		else
			MacOSPlatform::NotifyWindowEvent(WindowEventType::Restored, mWindow.WindowId);

		mIsZooming = true;
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

	b3d::Vector2I position((int32_t)point.x, (int32_t)point.y);
	if(b3d::CocoaDragAndDrop::NotifyDragEnteredInternal(mWindow.WindowId, position))
		return NSDragOperationLink;

	return NSDragOperationNone;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	NSPoint point = [sender draggingLocation];
	point = frameToContentRect(mWindow, point);

	b3d::Vector2I position((int32_t)point.x, (int32_t)point.y);
	if(b3d::CocoaDragAndDrop::NotifyDragMovedInternal(mWindow.WindowId, position))
		return NSDragOperationLink;

	return NSDragOperationNone;
}

- (void)draggingExited:(nullable id <NSDraggingInfo>)sender
{
	b3d::CocoaDragAndDrop::NotifyDragLeftInternal(mWindow.WindowId);
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	NSPasteboard* pasteboard = [sender draggingPasteboard];
	if([[pasteboard types] containsObject:NSFilenamesPboardType])
	{
		NSArray* entries = [pasteboard propertyListForType:NSFilenamesPboardType];

		b3d::Vector<b3d::Path> paths;
		for(NSString* path in entries)
		{
			const char* pathChars = [path UTF8String];
			paths.push_back(b3d::Path(pathChars));
		}

		NSPoint point = [sender draggingLocation];
		point = frameToContentRect(mWindow, point);

		b3d::Vector2I position((int32_t)point.x, (int32_t)point.y);

		if(b3d::CocoaDragAndDrop::NotifyDragDroppedInternal(mWindow.WindowId, position, paths))
			return YES;
	}

	return NO;
}
@end

namespace b3d
{
	std::atomic<u32> gNextWindowId(1);

	CocoaWindow::CocoaWindow(const WindowCreateInformation& createInformation)
	{ @autoreleasepool {
		m = B3DNew<Pimpl>();

		BSWindow* window = [BSWindow alloc];
		m->IsModal = createInformation.Modal;
		mWindowId = gNextWindowId++;

		NSArray* screens = [NSScreen screens];

		NSScreen* screen = nil;
		i32 x = 0;
		i32 y = 0;

		for(NSScreen* entry in screens)
		{
			NSRect screenRect = [entry frame];

			i32 left = (i32)screenRect.origin.x;
			i32 right = left + (i32)screenRect.size.width;
			i32 bottom = (i32)screenRect.origin.y;
			i32 top = bottom + (i32)screenRect.size.height;

			if(((createInformation.X >= left && createInformation.X < right) || createInformation.X == -1) &&
			   ((createInformation.Y >= bottom && createInformation.Y < top) || createInformation.Y == -1))
			{
				if(createInformation.X == -1)
					x = left + std::max(0, (i32)screenRect.size.width - (i32)createInformation.Width) / 2;
				else
					x = createInformation.X - left;

				if(createInformation.Y == -1)
					y = bottom + std::max(0, (i32)screenRect.size.height - (i32)createInformation.Height) / 2;
				else
					y = ((i32)screenRect.size.height - (createInformation.Y + (i32)createInformation.Height)) - bottom;

				screen = entry;
				break;
			}
		}

		if(!createInformation.ShowDecorations)
			m->Style |= NSWindowStyleMaskBorderless;
		else
			m->Style |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

		if(createInformation.AllowResize)
			m->Style |= NSWindowStyleMaskResizable;

		window = [window
				initWithContentRect:NSMakeRect(x, y, createInformation.Width, createInformation.Height)
				styleMask:(NSWindowStyleMask)m->Style
				backing:NSBackingStoreBuffered
				defer:NO
				screen:screen];
		m->Window = window;
		window.WindowId = mWindowId;

		if(createInformation.AllowResize)
		{
			bool allowSpaces = NSAppKitVersionNumber > NSAppKitVersionNumber10_6;
			if(allowSpaces)
				[m->Window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		}

		NSString* titleString = [NSString stringWithUTF8String:createInformation.Title.c_str()];

		[m->Window setAcceptsMouseMovedEvents:YES];
		[m->Window setReleasedWhenClosed:YES];
		[m->Window setTitle:titleString];
		[m->Window makeKeyAndOrderFront:nil];

		m->Responder = [[BSWindowListener alloc] initWithWindow:window];
		[m->Window setNextResponder:m->Responder];

		m->Delegate = [[BSWindowDelegate alloc] initWithWindow:window];
		[m->Window setDelegate:m->Delegate];

		m->View = [[BSView alloc] init];

		[m->Window setContentView:m->View];

		if(createInformation.Background)
		{
			[m->Window setAlphaValue:1.0f];
			[m->Window setOpaque:NO];
			[m->Window setBackgroundColor:[NSColor clearColor]];

			NSImage* image = MacOSPlatform::CreateNSImage(*createInformation.Background);
			[m->View setBackgroundImage:image];
		}

		m->IsFullscreen = false;

		// Makes sure that floating windows hide together with main app
		// Also, for some reason it makes makeKeyAndOrderFront work properly when multiple windows are opened at the same
		// frame. (Without it, only the last opened window moves to front)
		if(createInformation.Floating || createInformation.Modal)
			[m->Window setHidesOnDeactivate:YES];

		if(createInformation.Floating)
			[m->Window setLevel:NSFloatingWindowLevel];

		if(createInformation.Modal)
			m->ModalSession = [NSApp beginModalSessionForWindow:m->Window];

		MacOSPlatform::RegisterWindow(this);
	}}

	CocoaWindow::~CocoaWindow()
	{
		if (m->Window != nil)
			DestroyInternal();

		m->Delegate = nil;
		m->Responder = nil;
		m->View = nil;
		m->Layer = nil;

		B3DDelete(m);
	}

	void CocoaWindow::Move(i32 left, i32 top)
	{
		@autoreleasepool
		{
			NSPoint point;
			point.x = left;
			point.y = top;

			flipY(m->Window.screen, point);
			[m->Window setFrameTopLeftPoint:point];
		}
	}

	void CocoaWindow::Resize(u32 width, u32 height)
	{
		@autoreleasepool
		{
			NSSize size;
			size.width = width;
			size.height = height;

			NSRect frameRect = m->Window.frame;
			NSRect contentRect = [m->Window contentRectForFrameRect:frameRect];

			contentRect.size.width = size.width;
			contentRect.size.height = size.height;

			[m->Window setFrame:[m->Window frameRectForContentRect:contentRect] display:YES];
		}
	}

	Area2I CocoaWindow::GetArea() const
	{
		@autoreleasepool
		{
			NSRect frameRect = [m->Window frame];
			NSRect contentRect = [m->Window contentRectForFrameRect:frameRect];

			flipY([m->Window screen], contentRect);

			return Area2I(
					(i32)contentRect.origin.x,
					(i32)contentRect.origin.y,
					(u32)contentRect.size.width,
					(u32)contentRect.size.height);
		}
	}

	i32 CocoaWindow::GetLeft() const
	{
		return GetArea().X;
	}

	i32 CocoaWindow::GetTop() const
	{
		return GetArea().Y;
	}

	u32 CocoaWindow::GetWidth() const
	{
		return GetArea().Width;
	}

	u32 CocoaWindow::GetHeight() const
	{
		return GetArea().Height;
	}

	void CocoaWindow::SetHidden(bool hidden)
	{
		@autoreleasepool
		{
			if(hidden)
				[m->Window orderOut:nil];
			else
				[m->Window makeKeyAndOrderFront:nil];
		}
	}

	void CocoaWindow::Maximize()
	{
		@autoreleasepool
		{
			if(![m->Window isZoomed])
				[m->Window zoom:nil];
		}
	}

	void CocoaWindow::Minimize()
	{
		@autoreleasepool
		{
			[m->Window miniaturize:nil];
		}
	}

	void CocoaWindow::Restore()
	{
		@autoreleasepool
		{
			if([m->Window isMiniaturized])
				[m->Window deminiaturize:nil];
			else if([m->Window isZoomed])
				[m->Window zoom:nil];
		}
	}

	void CocoaWindow::SetWindowed()
	{
		@autoreleasepool
		{
			if(m->IsFullscreen)
			{
				[m->Window setStyleMask:(NSWindowStyleMask)m->Style];
				[m->Window setFrame:m->WindowedRect display:NO];
				[m->Window setLevel:NSNormalWindowLevel];

				m->IsFullscreen = false;
			}
		}
	}

	void CocoaWindow::SetFullscreen()
	{
		@autoreleasepool
		{
			if(!m->IsFullscreen)
				m->WindowedRect = [m->Window frame];

			NSRect frame = [[m->Window screen] frame];
			[m->Window setStyleMask:NSWindowStyleMaskBorderless];
			[m->Window setFrame:frame display:NO];
			[m->Window setLevel:NSMainMenuWindowLevel+1];
			[m->Window makeKeyAndOrderFront:nil];

			m->IsFullscreen = true;
		}
	}

	Vector2I CocoaWindow::WindowToScreenPos(const Vector2I& windowPos) const
	{
		NSRect frameRect = [m->Window frame];
		NSRect contentRect = [m->Window contentRectForFrameRect:frameRect];

		flipY([m->Window screen], contentRect);

		Vector2I screenPos;
		screenPos.X = windowPos.X + (i32)contentRect.origin.x;
		screenPos.Y = windowPos.Y + (i32)contentRect.origin.y;

		return screenPos;
	}

	Vector2I CocoaWindow::ScreenToWindowPos(const Vector2I& screenPos) const
	{
		NSRect frameRect = [m->Window frame];
		NSRect contentRect = [m->Window contentRectForFrameRect:frameRect];

		flipY([m->Window screen], contentRect);

		Vector2I windowPos;
		windowPos.X = screenPos.X - (i32) contentRect.origin.x;
		windowPos.Y = screenPos.Y - (i32) contentRect.origin.y;

		return windowPos;
	}

	void CocoaWindow::DoOnWindowMovedOrResized()
	{
		// On macOS the window position/size is always queried directly from NSWindow
		// so there's nothing to cache here. This method exists for API consistency.
	}

	void CocoaWindow::DestroyInternal()
	{
		if(m->IsModal)
			[NSApp endModalSession:m->ModalSession];

		MacOSPlatform::UnregisterWindow(this);

		[m->Window close];
		m->Window = nil;
	}

	void CocoaWindow::SetDragZonesInternal(const Vector<Area2I>& areas)
	{
		@autoreleasepool
		{
			NSMutableArray* array = [[NSMutableArray alloc] init];

			for(auto& entry : areas)
				[array addObject:[NSValue valueWithBytes:&entry objCType:@encode(Area2I)]];

			[m->Responder setDragAreas:array];
		}
	}

	void CocoaWindow::SetUserDataInternal(void* data)
	{
		m->UserData = data;
	}

	void* CocoaWindow::GetUserDataInternal() const
	{
		return m->UserData;
	}

	void CocoaWindow::RegisterForDragAndDropInternal()
	{
		if(m->NumDropTargets == 0)
			[m->Window registerForDraggedTypes:@[NSFilenamesPboardType]];

		m->NumDropTargets++;
	}

	void CocoaWindow::UnregisterForDragAndDropInternal()
	{
		if(m->NumDropTargets == 0)
			return;

		m->NumDropTargets--;

		if(m->NumDropTargets == 0)
			[m->Window unregisterDraggedTypes];
	}

	void CocoaWindow::RegisterGLContextInternal(void* context)
	{
		NSOpenGLContext* glContext = (__bridge_transfer NSOpenGLContext* )context;
		[m->View setGLContext:glContext];
	}

	void CocoaWindow::SetLayerInternal(void* layer)
	{
		[m->View setLayer:(__bridge CALayer*)layer];
		[m->View setWantsLayer:TRUE];

		m->Layer = (__bridge CALayer*)layer;
	}

	void* CocoaWindow::GetLayerInternal() const
	{
		return (__bridge void *)m->Layer;
	}
}
