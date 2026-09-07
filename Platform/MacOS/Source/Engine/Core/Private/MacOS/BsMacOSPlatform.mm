//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define BS_COCOA_INTERNALS 1
#include "Private/MacOS/B3DMacOSPlatform.h"
#include "Private/MacOS/B3DMacOSWindow.h"
#include "Input/B3DInputFwd.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DColor.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Private/MacOS/B3DMacOSDropTarget.h"
#include "String/B3DUnicode.h"
#include "B3DApplication.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <IOKit/hidsystem/IOLLEvent.h>

namespace b3d
{
	namespace
	{
		void SendKeyEvent(u16 keyCode, bool pressed, u64 timestamp)
		{
			ButtonCode button;
			switch(keyCode)
			{
			case kVK_ANSI_A: button = ButtonCode::A; break;
			case kVK_ANSI_B: button = ButtonCode::B; break;
			case kVK_ANSI_C: button = ButtonCode::C; break;
			case kVK_ANSI_D: button = ButtonCode::D; break;
			case kVK_ANSI_E: button = ButtonCode::E; break;
			case kVK_ANSI_F: button = ButtonCode::F; break;
			case kVK_ANSI_G: button = ButtonCode::G; break;
			case kVK_ANSI_H: button = ButtonCode::H; break;
			case kVK_ANSI_I: button = ButtonCode::I; break;
			case kVK_ANSI_J: button = ButtonCode::J; break;
			case kVK_ANSI_K: button = ButtonCode::K; break;
			case kVK_ANSI_L: button = ButtonCode::L; break;
			case kVK_ANSI_M: button = ButtonCode::M; break;
			case kVK_ANSI_N: button = ButtonCode::N; break;
			case kVK_ANSI_O: button = ButtonCode::O; break;
			case kVK_ANSI_P: button = ButtonCode::P; break;
			case kVK_ANSI_Q: button = ButtonCode::Q; break;
			case kVK_ANSI_R: button = ButtonCode::R; break;
			case kVK_ANSI_S: button = ButtonCode::S; break;
			case kVK_ANSI_T: button = ButtonCode::T; break;
			case kVK_ANSI_U: button = ButtonCode::U; break;
			case kVK_ANSI_V: button = ButtonCode::V; break;
			case kVK_ANSI_W: button = ButtonCode::W; break;
			case kVK_ANSI_X: button = ButtonCode::X; break;
			case kVK_ANSI_Y: button = ButtonCode::Y; break;
			case kVK_ANSI_Z: button = ButtonCode::Z; break;
			case kVK_ANSI_0: button = ButtonCode::Key0; break;
			case kVK_ANSI_1: button = ButtonCode::Key1; break;
			case kVK_ANSI_2: button = ButtonCode::Key2; break;
			case kVK_ANSI_3: button = ButtonCode::Key3; break;
			case kVK_ANSI_4: button = ButtonCode::Key4; break;
			case kVK_ANSI_5: button = ButtonCode::Key5; break;
			case kVK_ANSI_6: button = ButtonCode::Key6; break;
			case kVK_ANSI_7: button = ButtonCode::Key7; break;
			case kVK_ANSI_8: button = ButtonCode::Key8; break;
			case kVK_ANSI_9: button = ButtonCode::Key9; break;
			case kVK_ANSI_Equal: button = ButtonCode::Equals; break;
			case kVK_ANSI_Minus: button = ButtonCode::Minus; break;
			case kVK_ANSI_RightBracket: button = ButtonCode::RightBracket; break;
			case kVK_ANSI_LeftBracket: button = ButtonCode::LeftBracket; break;
			case kVK_ANSI_Quote: button = ButtonCode::Apostrophe; break;
			case kVK_ANSI_Semicolon: button = ButtonCode::Semicolon; break;
			case kVK_ANSI_Backslash: button = ButtonCode::Backslash; break;
			case kVK_ANSI_Comma: button = ButtonCode::Comma; break;
			case kVK_ANSI_Slash: button = ButtonCode::Slash; break;
			case kVK_ANSI_Period: button = ButtonCode::Period; break;
			case kVK_ANSI_Grave: button = ButtonCode::Grave; break;
			case kVK_ANSI_KeypadDecimal: button = ButtonCode::NumpadDecimal; break;
			case kVK_ANSI_KeypadMultiply: button = ButtonCode::NumpadMultiply; break;
			case kVK_ANSI_KeypadPlus: button = ButtonCode::NumpadPlus; break;
			case kVK_ANSI_KeypadClear: button = ButtonCode::NumLock; break;
			case kVK_ANSI_KeypadDivide: button = ButtonCode::NumpadDivide; break;
			case kVK_ANSI_KeypadEnter: button = ButtonCode::NumpadEnter; break;
			case kVK_ANSI_KeypadMinus: button = ButtonCode::NumpadMinus; break;
			case kVK_ANSI_KeypadEquals: button = ButtonCode::NumadEquals; break;
			case kVK_ANSI_Keypad0: button = ButtonCode::Numpad0; break;
			case kVK_ANSI_Keypad1: button = ButtonCode::Numpad1; break;
			case kVK_ANSI_Keypad2: button = ButtonCode::Numpad2; break;
			case kVK_ANSI_Keypad3: button = ButtonCode::Numpad3; break;
			case kVK_ANSI_Keypad4: button = ButtonCode::Numpad4; break;
			case kVK_ANSI_Keypad5: button = ButtonCode::Numpad5; break;
			case kVK_ANSI_Keypad6: button = ButtonCode::Numpad6; break;
			case kVK_ANSI_Keypad7: button = ButtonCode::Numpad7; break;
			case kVK_ANSI_Keypad8: button = ButtonCode::Numpad8; break;
			case kVK_ANSI_Keypad9: button = ButtonCode::Numpad9; break;
			case kVK_Return: button = ButtonCode::Enter; break;
			case kVK_Tab: button = ButtonCode::Tab; break;
			case kVK_Space: button = ButtonCode::Space; break;
			case kVK_Delete: button = ButtonCode::Backspace; break;
			case kVK_Escape: button = ButtonCode::Escape; break;
			case kVK_Command: button = ButtonCode::LeftWindows; break;
			case kVK_RightCommand: button = ButtonCode::RightWindows; break;
			case kVK_Shift: button = ButtonCode::LeftShift; break;
			case kVK_CapsLock: button = ButtonCode::CapsLock; break;
			case kVK_Option: button = ButtonCode::LeftAlt; break;
			case kVK_Control: button = ButtonCode::LeftControl; break;
			case kVK_RightShift: button = ButtonCode::RightShift; break;
			case kVK_RightOption: button = ButtonCode::RightAlt; break;
			case kVK_RightControl: button = ButtonCode::RightControl; break;
			case kVK_ISO_Section: button = ButtonCode::OEM102; break;
			case kVK_JIS_Yen: button = ButtonCode::Yen; break;
			case kVK_JIS_Underscore: button = ButtonCode::Underline; break;
			case kVK_JIS_KeypadComma: button = ButtonCode::NumpadComma; break;
			case kVK_JIS_Kana: button = ButtonCode::Kana; break;
			case kVK_JIS_Eisu: button = ButtonCode::Kanji; break;
			case kVK_VolumeUp: button = ButtonCode::VolumeUp; break;
			case kVK_VolumeDown: button = ButtonCode::VolumeDown; break;
			case kVK_Mute: button = ButtonCode::Mute; break;
			case kVK_Help: button = ButtonCode::Insert; break;
			case kVK_Home: button = ButtonCode::Home; break;
			case kVK_PageUp: button = ButtonCode::PageUp; break;
			case kVK_ForwardDelete: button = ButtonCode::Delete; break;
			case kVK_End: button = ButtonCode::End; break;
			case kVK_PageDown: button = ButtonCode::PageDown; break;
			case kVK_LeftArrow: button = ButtonCode::ArrowLeft; break;
			case kVK_RightArrow: button = ButtonCode::ArrowRight; break;
			case kVK_DownArrow: button = ButtonCode::ArrowDown; break;
			case kVK_UpArrow: button = ButtonCode::ArrowUp; break;
			case kVK_F1: button = ButtonCode::F1; break;
			case kVK_F2: button = ButtonCode::F2; break;
			case kVK_F3: button = ButtonCode::F3; break;
			case kVK_F4: button = ButtonCode::F4; break;
			case kVK_F5: button = ButtonCode::F5; break;
			case kVK_F6: button = ButtonCode::F6; break;
			case kVK_F7: button = ButtonCode::F7; break;
			case kVK_F8: button = ButtonCode::F8; break;
			case kVK_F9: button = ButtonCode::F9; break;
			case kVK_F10: button = ButtonCode::F10; break;
			case kVK_F11: button = ButtonCode::F11; break;
			case kVK_F12: button = ButtonCode::F12; break;
			case kVK_F13: button = ButtonCode::F13; break;
			case kVK_F14: button = ButtonCode::F14; break;
			case kVK_F15: button = ButtonCode::F15; break;
			default: return;
			}

			MacOSPlatform::OnButtonChanged(button, pressed, timestamp);
		}
	}
}

/** Application implementation that overrides the terminate logic with custom shutdown, and tracks Esc key presses. */
@interface BSApplication : NSApplication
-(void)terminate:(nullable id)sender;
-(void)sendEvent:(NSEvent*)event;
@end

@implementation BSApplication
-(void)terminate:(nullable id)sender
{
	b3d::GetApplication().NotifyQuitRequested();
}

-(void)sendEvent:(NSEvent *)event
{
	static Class engineWindowClass = NSClassFromString(@"BSWindow");
	if(self.active && [event.window isKindOfClass:engineWindowClass])
	{
		const b3d::u64 timestamp = (b3d::u64)(event.timestamp * 1000.0);
		switch(event.type)
		{
		case NSEventTypeKeyDown:
		case NSEventTypeKeyUp:
			b3d::SendKeyEvent(event.keyCode, event.type == NSEventTypeKeyDown, timestamp);
			break;
		case NSEventTypeFlagsChanged:
		{
			const NSUInteger flags = event.modifierFlags;
			b3d::SendKeyEvent(kVK_Shift, (flags & NX_DEVICELSHIFTKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_RightShift, (flags & NX_DEVICERSHIFTKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_Control, (flags & NX_DEVICELCTLKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_RightControl, (flags & NX_DEVICERCTLKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_Option, (flags & NX_DEVICELALTKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_RightOption, (flags & NX_DEVICERALTKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_Command, (flags & NX_DEVICELCMDKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_RightCommand, (flags & NX_DEVICERCMDKEYMASK) != 0, timestamp);
			b3d::SendKeyEvent(kVK_CapsLock, (flags & NSEventModifierFlagCapsLock) != 0, timestamp);
			break;
		}
		case NSEventTypeLeftMouseDown:
		case NSEventTypeRightMouseDown:
		case NSEventTypeOtherMouseDown:
		case NSEventTypeLeftMouseUp:
		case NSEventTypeRightMouseUp:
		case NSEventTypeOtherMouseUp:
			if(event.buttonNumber >= 0 && event.buttonNumber < (NSInteger)b3d::ButtonCode::MouseKeyCount)
			{
				const bool pressed = event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeOtherMouseDown;
				b3d::MacOSPlatform::OnButtonChanged((b3d::ButtonCode)((b3d::u32)b3d::ButtonCode::MouseLeft + event.buttonNumber), pressed, timestamp);
			}
			break;
		case NSEventTypeMouseMoved:
		case NSEventTypeLeftMouseDragged:
		case NSEventTypeRightMouseDragged:
		case NSEventTypeOtherMouseDragged:
			b3d::MacOSPlatform::OnMouseMoved((float)event.deltaX, (float)event.deltaY, 0.0f);
			break;
		case NSEventTypeScrollWheel:
			b3d::MacOSPlatform::OnMouseMoved(0.0f, 0.0f, (float)event.deltaY);
			break;
		default:
			break;
		}
	}

	// Handle Esc & Tab key here, as it doesn't seem to be reported elsewhere
	if([event type] == NSEventTypeKeyDown)
	{
		if([event keyCode] == 53) // Escape key
		{
			b3d::InputCommandType inputCommand = b3d::InputCommandType::Escape;
			b3d::MacOSPlatform::SendInputCommandEvent(inputCommand);
		}
		else if([event keyCode] == 48) // Tab key
		{
			b3d::InputCommandType inputCommand = b3d::InputCommandType::Tab;
			b3d::MacOSPlatform::SendInputCommandEvent(inputCommand);
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

namespace b3d
{
	/** Contains information about a modal window session. */
	struct ModalWindowInfo
	{
		u32 WindowId;
		NSModalSession Session;
	};

	struct Platform::Private
	{
		BSAppDelegate* AppDelegate = nil;

		Mutex WindowMutex;
		CocoaWindow* MainWindow = nullptr;
		UnorderedMap<u32, CocoaWindow*> AllWindows;
		Vector<ModalWindowInfo> ModalWindows;

		BSFPlatform* PlatformManager = nil;

		// Cursor
		BSCursor* CursorManager = nil;

		Mutex CursorMutex;
		bool CursorIsHidden = false;
		Vector2I CursorPos;

		// Clipboard
		Mutex ClipboardMutex;
		String CachedClipboardData;
		i32 ClipboardChangeCount = -1;
	};
}

/**
 * Contains cursor specific functionality. Encapsulated in objective C so its selectors can be triggered from other
 * threads.
 */
@interface BSCursor : NSObject
@property NSCursor* currentCursor;

-(BSCursor*) initWithPlatformData:(b3d::Platform::Private*)platformData;
-(b3d::Vector2I) getPosition;
-(void) setPosition:(const b3d::Vector2I&) position;
-(BOOL) clipCursor:(b3d::Vector2I&) position;
-(void) updateClipBounds:(NSWindow*) window;
-(void) clipCursorToWindow:(NSValue*) windowValue;
-(void) clipCursorToRect:(NSValue*) rectValue;
-(void) clipCursorDisable;
-(void) setCursor:(NSArray*) params;
-(void) unregisterWindow:(NSWindow*) window;
@end

@implementation BSCursor
{
	b3d::Platform::Private* platformData;

	bool cursorClipEnabled;
	b3d::Area2I cursorClipRect;
	NSWindow* cursorClipWindow;
}

- (BSCursor*)initWithPlatformData:(b3d::Platform::Private*)data
{
	self = [super init];

	platformData = data;
	return self;
}

- (b3d::Vector2I)getPosition
{
	NSPoint point = [NSEvent mouseLocation];

	for (NSScreen* screen in [NSScreen screens])
	{
		NSRect frame = [screen frame];
		if (NSMouseInRect(point, frame, NO))
			b3d::flipY(screen, point);
	}

	b3d::Vector2I output;
	output.X = (b3d::i32)point.x;
	output.Y = (b3d::i32)point.y;

	return output;
}

- (void)setPosition:(const b3d::Vector2I&)position
{
	NSPoint point = NSMakePoint(position.X, position.Y);
	CGWarpMouseCursorPosition(point);

	b3d::Lock lock(platformData->CursorMutex);
	platformData->CursorPos = position;
}

- (BOOL)clipCursor:(b3d::Vector2I&)position
{
	if(!cursorClipEnabled)
		return false;

	b3d::i32 clippedX = position.X - cursorClipRect.X;
	b3d::i32 clippedY = position.Y - cursorClipRect.Y;

	if(clippedX < 0)
		clippedX = 0;
	else if(clippedX >= (b3d::i32)cursorClipRect.Width)
		clippedX = cursorClipRect.Width > 0 ? cursorClipRect.Width - 1 : 0;

	if(clippedY < 0)
		clippedY = 0;
	else if(clippedY >= (b3d::i32)cursorClipRect.Height)
		clippedY = cursorClipRect.Height > 0 ? cursorClipRect.Height - 1 : 0;

	clippedX += cursorClipRect.X;
	clippedY += cursorClipRect.Y;

	if(clippedX != position.X || clippedY != position.Y)
	{
		position.X = clippedX;
		position.Y = clippedY;

		return true;
	}

	return false;
}

- (void)updateClipBounds:(NSWindow*)window
{
	if(!cursorClipEnabled || cursorClipWindow != window)
		return;

	NSRect rect = [window contentRectForFrameRect:[window frame]];
	b3d::flipY([window screen], rect);

	cursorClipRect.X = (b3d::i32)rect.origin.x;
	cursorClipRect.Y = (b3d::i32)rect.origin.y;
	cursorClipRect.Width = (b3d::u32)rect.size.width;
	cursorClipRect.Height = (b3d::u32)rect.size.height;
}

- (void)clipCursorToWindow:(NSValue*)windowValue
{
	b3d::CocoaWindow* window;
	[windowValue getValue:&window];

	cursorClipEnabled = true;
	cursorClipWindow = window->GetPrivateDataInternal()->Window;

	[self updateClipBounds:cursorClipWindow];

	b3d::Vector2I pos = [self getPosition];

	if([self clipCursor:pos])
		[self setPosition:pos];
}

- (void)clipCursorToRect:(NSValue*)rectValue
{
	b3d::Area2I rect;
	[rectValue getValue:&rect];

	cursorClipEnabled = true;
	cursorClipRect = rect;
	cursorClipWindow = nullptr;

	b3d::Vector2I pos = [self getPosition];

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

	[self setCurrentCursor:cursor];

	for(auto& entry : platformData->AllWindows)
	{
		NSWindow* window = entry.second->GetPrivateDataInternal()->Window;
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
-(BSFPlatform*) initWithPlatformData:(b3d::Platform::Private*)platformData;
-(void) setCaptionNonClientAreas:(NSArray*) params;
-(void) resetNonClientAreas:(NSValue*) windowIdValue;
-(void) openFolder:(NSURL*) url;
-(void) setClipboardText:(NSString*) text;
-(NSString*) getClipboardText;
-(b3d::i32) getClipboardChangeCount;
@end

@implementation BSFPlatform
{
	b3d::Platform::Private* mPlatformData;
}

- (BSFPlatform*)initWithPlatformData:(b3d::Platform::Private*)platformData
{
	self = [super init];

	mPlatformData = platformData;
	return self;
}

- (void)setCaptionNonClientAreas:(NSArray*)params
{
	NSValue* windowIdValue = params[0];

	b3d::u32 windowId;
	[windowIdValue getValue:&windowId];

	auto iterFind = mPlatformData->AllWindows.find(windowId);
	if(iterFind == mPlatformData->AllWindows.end())
		return;

	b3d::CocoaWindow* window = iterFind->second;

	NSUInteger numEntries = [params count] - 1;

	b3d::Vector<b3d::Area2I> areas;
	for(NSUInteger entryIndex = 0; entryIndex < numEntries; entryIndex++)
	{
		NSValue* value = params[entryIndex + 1];

		b3d::Area2I area;
		[value getValue:&area];

		areas.push_back(area);
	}

	window->SetDragZonesInternal(areas);
}

- (void)resetNonClientAreas:(NSValue*) windowIdValue
{
	b3d::u32 windowId;
	[windowIdValue getValue:&windowId];

	auto iterFind = mPlatformData->AllWindows.find(windowId);
	if(iterFind == mPlatformData->AllWindows.end())
		return;

	b3d::CocoaWindow* window = iterFind->second;
	window->SetDragZonesInternal({});
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

- (b3d::i32)getClipboardChangeCount
{
	return (b3d::i32)[[NSPasteboard generalPasteboard] changeCount];
}

@end

namespace b3d
{
	// Maps macOS keycodes to b3d button codes
	static constexpr ButtonCode kKeyCodeMapping[] =
	{
		/*   0 */   ButtonCode::A,
		/*   1 */   ButtonCode::S,
		/*   2 */   ButtonCode::D,
		/*   3 */   ButtonCode::F,
		/*   4 */   ButtonCode::H,
		/*   5 */   ButtonCode::G,
		/*   6 */   ButtonCode::Z,
		/*   7 */   ButtonCode::X,
		/*   8 */   ButtonCode::C,
		/*   9 */   ButtonCode::V,
		/*  10 */   ButtonCode::Grave,
		/*  11 */   ButtonCode::B,
		/*  12 */   ButtonCode::Q,
		/*  13 */   ButtonCode::W,
		/*  14 */   ButtonCode::E,
		/*  15 */   ButtonCode::R,
		/*  16 */   ButtonCode::Y,
		/*  17 */   ButtonCode::T,
		/*  18 */   ButtonCode::Key1,
		/*  19 */   ButtonCode::Key2,
		/*  20 */   ButtonCode::Key3,
		/*  21 */   ButtonCode::Key4,
		/*  22 */   ButtonCode::Key6,
		/*  23 */   ButtonCode::Key5,
		/*  24 */   ButtonCode::Equals,
		/*  25 */   ButtonCode::Key9,
		/*  26 */   ButtonCode::Key7,
		/*  27 */   ButtonCode::Minus,
		/*  28 */   ButtonCode::Key8,
		/*  29 */   ButtonCode::Key0,
		/*  30 */   ButtonCode::RightBracket,
		/*  31 */   ButtonCode::O,
		/*  32 */   ButtonCode::U,
		/*  33 */   ButtonCode::LeftBracket,
		/*  34 */   ButtonCode::I,
		/*  35 */   ButtonCode::P,
		/*  36 */   ButtonCode::Enter,
		/*  37 */   ButtonCode::L,
		/*  38 */   ButtonCode::J,
		/*  39 */   ButtonCode::Apostrophe,
		/*  40 */   ButtonCode::K,
		/*  41 */   ButtonCode::Semicolon,
		/*  42 */   ButtonCode::Backslash,
		/*  43 */   ButtonCode::Comma,
		/*  44 */   ButtonCode::Slash,
		/*  45 */   ButtonCode::N,
		/*  46 */   ButtonCode::M,
		/*  47 */   ButtonCode::Period,
		/*  48 */   ButtonCode::Tab,
		/*  49 */   ButtonCode::Space,
		/*  50 */   ButtonCode::Grave,
		/*  51 */   ButtonCode::Backspace,
		/*  52 */   ButtonCode::NumpadEnter,
		/*  53 */   ButtonCode::Escape,
		/*  54 */   ButtonCode::RightWindows,
		/*  55 */   ButtonCode::LeftWindows,
		/*  56 */   ButtonCode::LeftShift,
		/*  57 */   ButtonCode::CapsLock,
		/*  58 */   ButtonCode::LeftAlt,
		/*  59 */   ButtonCode::LeftControl,
		/*  60 */   ButtonCode::RightShift,
		/*  61 */   ButtonCode::RightAlt,
		/*  62 */   ButtonCode::RightControl,
		/*  63 */   ButtonCode::RightWindows,
		/*  64 */   ButtonCode::Unassigned,
		/*  65 */   ButtonCode::NumpadDecimal,
		/*  66 */   ButtonCode::Unassigned,
		/*  67 */   ButtonCode::NumpadMultiply,
		/*  68 */   ButtonCode::Unassigned,
		/*  69 */   ButtonCode::NumpadPlus,
		/*  70 */   ButtonCode::Unassigned,
		/*  71 */   ButtonCode::NumLock,
		/*  72 */   ButtonCode::VolumeUp,
		/*  73 */   ButtonCode::VolumeDown,
		/*  74 */   ButtonCode::Mute,
		/*  75 */   ButtonCode::NumpadDivide,
		/*  76 */   ButtonCode::NumpadEnter,
		/*  77 */   ButtonCode::Unassigned,
		/*  78 */   ButtonCode::NumpadMinus,
		/*  79 */   ButtonCode::Unassigned,
		/*  80 */   ButtonCode::Unassigned,
		/*  81 */   ButtonCode::NumadEquals,
		/*  82 */   ButtonCode::Numpad0,
		/*  83 */   ButtonCode::Numpad1,
		/*  84 */   ButtonCode::Numpad2,
		/*  85 */   ButtonCode::Numpad3,
		/*  86 */   ButtonCode::Numpad4,
		/*  87 */   ButtonCode::Numpad5,
		/*  88 */   ButtonCode::Numpad6,
		/*  89 */   ButtonCode::Numpad7,
		/*  90 */   ButtonCode::Unassigned,
		/*  91 */   ButtonCode::Numpad8,
		/*  92 */   ButtonCode::Numpad9,
		/*  93 */   ButtonCode::Convert,
		/*  94 */   ButtonCode::NoConvert,
		/*  95 */   ButtonCode::NumpadComma,
		/*  96 */   ButtonCode::F5,
		/*  97 */   ButtonCode::F6,
		/*  98 */   ButtonCode::F7,
		/*  99 */   ButtonCode::F3,
		/* 100 */   ButtonCode::F8,
		/* 101 */   ButtonCode::F9,
		/* 102 */   ButtonCode::Unassigned,
		/* 103 */   ButtonCode::F11,
		/* 104 */   ButtonCode::Unassigned,
		/* 105 */   ButtonCode::Unassigned,
		/* 106 */   ButtonCode::Unassigned,
		/* 107 */   ButtonCode::ScrollLock,
		/* 108 */   ButtonCode::Unassigned,
		/* 109 */   ButtonCode::F10,
		/* 110 */   ButtonCode::Unassigned,
		/* 111 */   ButtonCode::F12,
		/* 112 */   ButtonCode::Unassigned,
		/* 113 */   ButtonCode::Pause,
		/* 114 */   ButtonCode::Insert,
		/* 115 */   ButtonCode::Home,
		/* 116 */   ButtonCode::PageUp,
		/* 117 */   ButtonCode::Delete,
		/* 118 */   ButtonCode::F4,
		/* 119 */   ButtonCode::End,
		/* 120 */   ButtonCode::F2,
		/* 121 */   ButtonCode::PageDown,
		/* 122 */   ButtonCode::F1,
		/* 123 */   ButtonCode::ArrowLeft,
		/* 124 */   ButtonCode::ArrowRight,
		/* 125 */   ButtonCode::ArrowDown,
		/* 126 */   ButtonCode::ArrowUp,
		/* 127 */   ButtonCode::Power
	};

	// Maps b3d keyboard button codes to macOS keycodes
	static u32 ButtonCodeToKeyCode[256];
	static void InitKeyCodeMapping()
	{
		memset(ButtonCodeToKeyCode, 0, sizeof(ButtonCodeToKeyCode));

		u32 keyCodeCount = sizeof(kKeyCodeMapping) / sizeof(kKeyCodeMapping[0]);

		for(u32 keyCodeIndex = 0; keyCodeIndex < keyCodeCount; keyCodeIndex++)
			ButtonCodeToKeyCode[(u32)kKeyCodeMapping[keyCodeIndex]] = keyCodeIndex;
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
	static NSString* GetAppName()
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
	static void CreateApplicationMenu()
	{ @autoreleasepool {
		NSMenu* mainMenu = [[NSMenu alloc] init];
		[NSApp setMainMenu:mainMenu];

		NSString* appName = GetAppName();
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

	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::OnPointerMoved;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::OnPointerButtonPressed;
	Event<void(const Vector2I&, OSMouseButton button, const OSPointerButtonStates&)> Platform::OnPointerButtonReleased;
	Event<void(const Vector2I&, const OSPointerButtonStates&)> Platform::OnPointerDoubleClick;
	Event<void(ButtonCode, bool, u64)> MacOSPlatform::OnButtonChanged;
	Event<void(float, float, float)> MacOSPlatform::OnMouseMoved;

	Event<void(InputCommandType)> Platform::OnInputCommand;
	Event<void(float)> Platform::OnMouseWheelScrolled;
	Event<void(u32)> Platform::OnCharInput;

	Event<void()> Platform::OnMouseCaptureChanged;
	Event<void()> Platform::OnInputDevicesChanged;

	Platform::Private* Platform::mData = B3DNew<Platform::Private>();

	Platform::~Platform()
	{
		B3DDelete(mData);
		mData = nullptr;
	}

	Vector2I Platform::GetCursorPosition()
	{
		Lock lock(mData->CursorMutex);
		return mData->CursorPos;
	}

	void Platform::SetCursorPosition(const Vector2I& screenPos)
	{
		[mData->CursorManager setPosition:screenPos];
	}

	void Platform::CaptureMouse(const RenderWindow& window)
	{
		// Do nothing
	}

	void Platform::ReleaseMouseCapture()
	{
		// Do nothing
	}

	bool Platform::IsPointOverWindow(const RenderWindow& window, const Vector2I& screenPos)
	{
		CFArrayRef windowDicts = CGWindowListCopyWindowInfo(
			kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
			kCGNullWindowID);

		if(!windowDicts)
			return false;

		u32 windowId = (u32)window.GetPlatformWindowHandle();
		auto iterFind = mData->AllWindows.find(windowId);
		if(iterFind == mData->AllWindows.end())
			return false;

		CocoaWindow* cocoaWindow = iterFind->second;
		i32 requestedWindowNumber = (i32)[cocoaWindow->GetPrivateDataInternal()->Window windowNumber];
		CGPoint point = CGPointMake(screenPos.X, screenPos.Y);

		CFIndex numEntries = CFArrayGetCount(windowDicts);
		for(CFIndex entryIndex = 0; entryIndex < numEntries; entryIndex++)
		{
			CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(windowDicts, entryIndex);

			CFNumberRef layerRef = (CFNumberRef) CFDictionaryGetValue(dict, kCGWindowLayer);
			if(!layerRef)
				continue;

			// Ignore windows outside of layer 0, as those appear to be desktop elements
			i32 layer;
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
				i32 windowNumber;
				CFNumberGetValue(windowNumRef, kCGWindowIDCFNumberType, &windowNumber);

				return requestedWindowNumber == windowNumber;
			}
		}

		return false;
	}

	void Platform::HideCursor()
	{
		Lock lock(mData->CursorMutex);

		if(!mData->CursorIsHidden)
		{
			[NSCursor performSelectorOnMainThread:@selector(hide) withObject:nil waitUntilDone:NO];
			mData->CursorIsHidden = true;
		}
	}

	void Platform::ShowCursor()
	{
		Lock lock(mData->CursorMutex);

		if(mData->CursorIsHidden)
		{
			[NSCursor performSelectorOnMainThread:@selector(unhide) withObject:nil waitUntilDone:NO];
			mData->CursorIsHidden = false;
		}
	}

	bool Platform::IsCursorHidden()
	{
		Lock lock(mData->CursorMutex);

		return mData->CursorIsHidden;
	}

	void Platform::ClipCursorToWindow(const RenderWindow& window)
	{
		u32 windowId = (u32)window.GetPlatformWindowHandle();
		auto iterFind = mData->AllWindows.find(windowId);
		if(iterFind == mData->AllWindows.end())
			return;

		CocoaWindow* cocoaWindow = iterFind->second;
		[mData->CursorManager
			performSelectorOnMainThread:@selector(clipCursorToWindow:)
			withObject:[NSValue valueWithPointer:cocoaWindow]
			waitUntilDone:NO];
	}

	void Platform::ClipCursorToRect(const Area2I& screenRect)
	{
		[mData->CursorManager
			performSelectorOnMainThread:@selector(clipCursorToRect:)
			withObject:[NSValue value:&screenRect withObjCType:@encode(Area2I)]
			waitUntilDone:NO];
	}

	void Platform::ClipCursorDisable()
	{
		[mData->CursorManager
			performSelectorOnMainThread:@selector(clipCursorDisable)
			withObject:nil
			waitUntilDone:NO];
	}

	void Platform::SetCursor(PixelData& pixelData, const Vector2I& hotSpot)
	{ @autoreleasepool {
		NSImage* image = MacOSPlatform::CreateNSImage(pixelData);
		NSPoint point = NSMakePoint(hotSpot.X, hotSpot.Y);

		NSCursor* cursor = [[NSCursor alloc] initWithImage:image hotSpot:point];
		NSArray* params = @[cursor, [NSValue valueWithPoint:point]];

		[mData->CursorManager
			performSelectorOnMainThread:@selector(setCursor:) withObject:params waitUntilDone:NO];
	}}

	void Platform::SetIcon(const PixelData& pixelData)
	{ @autoreleasepool {
		NSImage* image = MacOSPlatform::CreateNSImage(pixelData);

		[NSApp performSelectorOnMainThread:@selector(setApplicationIconImage:) withObject:image waitUntilDone:NO];
	}}

	void Platform::SetCaptionNonClientAreas(const RenderWindow& window, const Vector<Area2I>& nonClientAreas)
	{ @autoreleasepool {
		NSMutableArray* params = [[NSMutableArray alloc] init];

		u32 windowId = (u32)window.GetPlatformWindowHandle();
		NSValue* windowIdValue = [NSValue valueWithBytes:&windowId objCType:@encode(u32)];

		[params addObject:windowIdValue];
		for(auto& entry : nonClientAreas)
			[params addObject:[NSValue value:&entry withObjCType:@encode(Area2I)]];

		[mData->PlatformManager
			performSelectorOnMainThread:@selector(setCaptionNonClientAreas:)
			withObject:params
			waitUntilDone:NO];
	}}

	void Platform::SetResizeNonClientAreas(const RenderWindow& window, const Vector<NonClientResizeArea>& nonClientAreas)
	{
		// Do nothing, custom resize areas not needed on MacOS
	}

	void Platform::ResetNonClientAreas(const RenderWindow& window)
	{
		u32 windowId = (u32)window.GetPlatformWindowHandle();
		NSValue* windowIdValue = [NSValue valueWithBytes:&windowId objCType:@encode(u32)];

		[mData->PlatformManager
			performSelectorOnMainThread:@selector(resetNonClientAreas:)
			withObject:windowIdValue
			waitUntilDone:NO];
	}

	void Platform::Sleep(u32 duration)
	{
		usleep(duration * 1000);
	}

	void Platform::CopyToClipboard(const String& string)
	{ @autoreleasepool {
		NSString* text = [NSString stringWithUTF8String:string.c_str()];
		[mData->PlatformManager performSelectorOnMainThread:@selector(setClipboardText:)
			withObject:text
			waitUntilDone:NO];
	}}

	String Platform::CopyFromClipboard()
	{
		Lock lock(mData->ClipboardMutex);
		return mData->CachedClipboardData;
	}

	String Platform::KeyCodeToUnicode(u32 buttonCode)
	{
		if(buttonCode >= sizeof(ButtonCodeToKeyCode) / sizeof(ButtonCodeToKeyCode[0]))
			return "";

		u32 keyCode = ButtonCodeToKeyCode[buttonCode];

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

		u32 keysDown = 0;
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
		String utf8String = UTF8::FromUtF16(u16String);

		return utf8String;
	}

	void Platform::OpenFolder(const Path& path)
	{
		String pathString = path.ToString();

		NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:pathString.c_str()]];
		[mData->PlatformManager
			performSelectorOnMainThread:@selector(openFolder:)
			withObject:url
			waitUntilDone:NO];
	}

	void Platform::StartUp()
	{
		InitKeyCodeMapping();

		mData->AppDelegate = [[BSAppDelegate alloc] init];
		mData->CursorManager = [[BSCursor alloc] initWithPlatformData:mData];
		mData->PlatformManager = [[BSFPlatform alloc] initWithPlatformData:mData];
		[BSApplication sharedApplication];

		[NSApp setDelegate:mData->AppDelegate];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

		CreateApplicationMenu();

		[NSApp finishLaunching];
	}

	void Platform::Update()
	{
		CocoaDragAndDrop::Update();

		{
			Lock lock(mData->CursorMutex);
			mData->CursorPos = [mData->CursorManager getPosition];
		}

		i32 changeCount = [mData->PlatformManager getClipboardChangeCount];
		if(mData->ClipboardChangeCount != changeCount)
		{
			NSString* string = [mData->PlatformManager getClipboardText];
			String utf8String;

			if(string)
				utf8String = [string UTF8String];

			{
				Lock lock(mData->ClipboardMutex);
				mData->CachedClipboardData = utf8String;
			}

			mData->ClipboardChangeCount = changeCount;
		}

		MessagePump();
	}

	void Platform::ShutDown()
	{
		// Do nothing
	}

	void Platform::MessagePump()
	{ @autoreleasepool {
		while(true)
		{
			if(!mData->ModalWindows.empty())
			{
				NSModalSession session = mData->ModalWindows.back().Session;
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

	void MacOSPlatform::RegisterWindow(CocoaWindow* window)
	{
		// First window is assumed to be main
		if(!mData->MainWindow)
			mData->MainWindow = window;

		CocoaWindow::Pimpl* windowData = window->GetPrivateDataInternal();
		if(windowData->IsModal)
		{
			ModalWindowInfo info = { window->GetWindowIdInternal(), windowData->ModalSession };
			mData->ModalWindows.push_back(info);
		}

		Lock lock(mData->WindowMutex);
		mData->AllWindows[window->GetWindowIdInternal()] = window;
	}

	void MacOSPlatform::UnregisterWindow(CocoaWindow* window)
	{
		CocoaWindow::Pimpl* windowData = window->GetPrivateDataInternal();
		if(windowData->IsModal)
		{
			u32 windowId = window->GetWindowIdInternal();
			auto iterFind = std::find_if(mData->ModalWindows.begin(), mData->ModalWindows.end(),
										 [windowId](const ModalWindowInfo& x)
										 {
											 return x.WindowId == windowId;
										 });

			if(iterFind != mData->ModalWindows.end())
				mData->ModalWindows.erase(iterFind);
		}

		Lock lock(mData->WindowMutex);
		mData->AllWindows.erase(window->GetWindowIdInternal());

		[mData->CursorManager unregisterWindow:windowData->Window];

		// Shut down app when the main window is closed
		if(mData->MainWindow == window)
		{
			GetApplication().NotifyQuitRequested();
			mData->MainWindow = nullptr;
		}
	}

	void MacOSPlatform::LockWindows()
	{
		mData->WindowMutex.lock();
	}

	void MacOSPlatform::UnlockWindows()
	{
		mData->WindowMutex.unlock();
	}

	CocoaWindow* MacOSPlatform::GetWindow(u32 windowId)
	{
		auto iterFind = mData->AllWindows.find(windowId);
		if(iterFind == mData->AllWindows.end())
			return nullptr;

		return iterFind->second;
	}

	NSImage* MacOSPlatform::CreateNSImage(const PixelData& data)
	{
		// Premultiply alpha
		Vector<Color> colors = data.GetColors();
		for(auto& color : colors)
		{
			color.R *= color.A;
			color.G *= color.A;
			color.B *= color.A;
		}

		// Convert to RGBA
		TShared<PixelData> rgbaData = PixelData::Create(data.GetWidth(), data.GetHeight(), 1, PF_RGBA8);
		rgbaData->SetColors(colors);
		@autoreleasepool
		{
			i32 pitch = data.GetWidth() * sizeof(u32);
			NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc]
										  initWithBitmapDataPlanes:nullptr
										  pixelsWide:data.GetWidth()
										  pixelsHigh:data.GetHeight()
										  bitsPerSample:8
										  samplesPerPixel:4
										  hasAlpha:YES
										  isPlanar:NO
										  colorSpaceName:NSDeviceRGBColorSpace
										  bytesPerRow:pitch
										  bitsPerPixel:32];

			unsigned char* pixels = [imageRep bitmapData];
			memcpy(pixels, rgbaData->GetData(), data.GetHeight() * pitch);

			NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(data.GetWidth(), data.GetHeight())];
			[image addRepresentation:imageRep];

			return image;
		}
	}

	void MacOSPlatform::SendInputCommandEvent(InputCommandType inputCommand)
	{
		OnInputCommand(inputCommand);
	}

	void MacOSPlatform::SendCharInputEvent(u32 character)
	{
		OnCharInput(character);
	}

	void MacOSPlatform::SendPointerButtonPressedEvent(
		const Vector2I& pos,
		OSMouseButton button,
		const OSPointerButtonStates& buttonStates)
	{
		OnPointerButtonPressed(pos, button, buttonStates);
	}

	void MacOSPlatform::SendPointerButtonReleasedEvent(
		const Vector2I& pos,
		OSMouseButton button,
		const OSPointerButtonStates& buttonStates)
	{
		OnPointerButtonReleased(pos, button, buttonStates);
	}

	void MacOSPlatform::SendPointerDoubleClickEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates)
	{
		OnPointerDoubleClick(pos, buttonStates);
	}

	void MacOSPlatform::SendPointerMovedEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates)
	{
		OnPointerMoved(pos, buttonStates);
	}

	void MacOSPlatform::SendMouseWheelScrollEvent(float delta)
	{
		OnMouseWheelScrolled(delta);
	}

	void MacOSPlatform::NotifyWindowEvent(WindowEventType type, u32 windowId)
	{
		CocoaWindow* window = nullptr;
		{
			auto iterFind = mData->AllWindows.find(windowId);
			if(iterFind == mData->AllWindows.end())
				return;

			window = iterFind->second;
		}

		auto renderWindow = (RenderWindow*)window->GetUserDataInternal();
		if(renderWindow == nullptr)
		{
			// If it's a render window we allow the client code to handle the message, otherwise we just destroy it
			if(type == WindowEventType::CloseRequested)
				window->DestroyInternal();

			return;
		}

		renderWindow->NotifyWindowEvent(type);
	}

	NSCursor* MacOSPlatform::GetCurrentCursorInternal()
	{
		return [mData->CursorManager currentCursor];
	}

	bool MacOSPlatform::ClipCursorInternal(Vector2I& pos)
	{
		return [mData->CursorManager clipCursor:pos];
	}

	void MacOSPlatform::UpdateClipBoundsInternal(NSWindow* window)
	{
		[mData->CursorManager updateClipBounds:window];
	}

	void MacOSPlatform::SetCursorPositionInternal(const Vector2I& position)
	{
		[mData->CursorManager setPosition:position];
	}
}
