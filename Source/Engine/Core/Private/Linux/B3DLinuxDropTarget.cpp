//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "String/B3DUnicode.h"
#include "Platform/B3DPlatform.h"
#include "Platform/B3DDropTarget.h"
#include "RenderAPI/B3DRenderWindow.h"
#include "Math/B3DRect2I.h"
#include "Private/Linux/B3DLinuxDropTarget.h"
#include "Private/Linux/B3DLinuxWindow.h"
#include "Private/Linux/B3DLinuxPlatform.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#undef None

using namespace b3d;

::Display* LinuxDragAndDrop::sXDisplay = nullptr;
bool LinuxDragAndDrop::sDragActive = false;
Vector<LinuxDragAndDrop::DropArea> LinuxDragAndDrop::sDropAreas;
Mutex LinuxDragAndDrop::sMutex;
i32 LinuxDragAndDrop::sDNDVersion = 0;
Atom LinuxDragAndDrop::sDNDType = 0;
::Window LinuxDragAndDrop::sDNDSource = 0;
Vector2I LinuxDragAndDrop::sDragPosition;
Vector<LinuxDragAndDrop::DragAndDropOp> LinuxDragAndDrop::sQueuedOperations;
Vector<LinuxDragAndDrop::DropAreaOp> LinuxDragAndDrop::sQueuedAreaOperations;

Atom LinuxDragAndDrop::sXdndAware;
Atom LinuxDragAndDrop::sXdndSelection;
Atom LinuxDragAndDrop::sXdndEnter;
Atom LinuxDragAndDrop::sXdndLeave;
Atom LinuxDragAndDrop::sXdndPosition;
Atom LinuxDragAndDrop::sXdndStatus;
Atom LinuxDragAndDrop::sXdndDrop;
Atom LinuxDragAndDrop::sXdndFinished;
Atom LinuxDragAndDrop::sXdndActionCopy;
Atom LinuxDragAndDrop::sXdndTypeList;
Atom LinuxDragAndDrop::sPRIMARY;

struct X11Property
{
	u8* data;
	i32 format;
	u32 count;
	Atom type;
};

// Results must be freed using XFree
X11Property readX11Property(::Display* display, ::Window window, Atom type)
{
	X11Property output;
	output.data = nullptr;

	unsigned long bytesLeft;
	int bytesToFetch = 0;

	do
	{
		if(output.data != nullptr)
			XFree(output.data);

		XGetWindowProperty(display, window, type, 0, bytesToFetch, False, AnyPropertyType, &output.type, &output.format, (unsigned long*)&output.count, &bytesLeft, &output.data);
		bytesToFetch += bytesLeft;
	}
	while(bytesLeft != 0);

	return output;
}

/**
 * Decodes percent (%) encoded characters in an URI to actual characters. Characters are decoded into the input string.
 */
void decodeURI(char* uri)
{
	if(uri == nullptr)
		return;

	u32 length = (u32)strlen(uri);
	u8 decodedChar = '\0';
	u32 writeIdx = 0;
	u32 decodeState = 0; // 0 - Not decoding, 1 - found a % char, 2- found a % and following char

	for(u32 i = 0; i < length; i++)
	{
		// Not currently decoding, check for % or write as-is
		if(decodeState == 0)
		{
			// Potentially an encoded char, start decode
			if(uri[i] == '%')
			{
				decodedChar = '\0';
				decodeState = 1;
			}
			else // Normal char, write as-is
			{
				uri[writeIdx] = uri[i];
				writeIdx++;
			}
		}
		else // Currently decoding, check if following chars are valid
		{
			char isa = uri[i] >= 'a' && uri[i] <= 'f';
			char isA = uri[i] >= 'A' && uri[i] <= 'F';
			char isn = uri[i] >= '0' && uri[i] <= '9';

			bool isHex = isa || isA || isn;
			if(!isHex)
			{
				// If not a hex, this can't be an encoded character. Write the last "decodeState" characters as normal
				for(u32 j = decodeState; j > 0; j--)
				{
					uri[writeIdx] = uri[i - j];
					writeIdx++;
				}

				decodeState = 0;
			}
			else
			{
				// Decode the hex character into a number
				char offset = '\0';
				if(isn)
					offset = 0 - '0';
				else if(isa)
					offset = 10 - 'a';
				else if(isA)
					offset = 10 - 'A';

				decodedChar |= (uri[i] + offset) << ((2 - decodeState) * 4);

				// Check if done decoding and write
				if(decodeState == 2)
				{
					uri[writeIdx] = decodedChar;
					writeIdx++;
					decodeState = 0;
				}
				else // decodeState == 1
					decodeState = 2;
			}
		}
	}

	uri[writeIdx] = '\0';
}

char* convertURIToLocalPath(char* uri)
{
	if(memcmp(uri, "file:/", 6) == 0)
		uri += 6;
	else if(strstr(uri, ":/") != nullptr)
		return nullptr;

	bool isLocal = uri[0] != '/' || (uri[0] != '\0' && uri[1] == '/');

	// Ignore hostname
	if(!isLocal && uri[0] == '/' && uri[2] != '/')
	{
		char* hostnameEnd = strchr(uri + 1, '/');
		if(hostnameEnd != nullptr)
		{
			char hostname[257];
			if(gethostname(hostname, 255) == 0)
			{
				hostname[256] = '\0';
				if(memcmp(uri + 1, hostname, hostnameEnd - (uri + 1)) == 0)
				{
					uri = hostnameEnd + 1;
					isLocal = true;
				}
			}
		}
	}

	if(isLocal)
	{
		decodeURI(uri);
		if(uri[1] == '/')
			uri++;
		else
			uri--;

		return uri;
	}

	return nullptr;
}

DropTarget::DropTarget(const RenderWindow* ownerWindow, const Rect2I& area)
	: mArea(area), mActive(false), mOwnerWindow(ownerWindow), mDropType(DropTargetType::None)
{
	LinuxDragAndDrop::registerDropTarget(this);
}

DropTarget::~DropTarget()
{
	LinuxDragAndDrop::unregisterDropTarget(this);

	ClearInternal();
}

void DropTarget::setArea(const Rect2I& area)
{
	mArea = area;

	LinuxDragAndDrop::updateDropTarget(this);
}

void LinuxDragAndDrop::StartUp(::Display* xDisplay)
{
	sXDisplay = xDisplay;

#define INIT_ATOM(name) s##name = XInternAtom(xDisplay, #name, False);

	INIT_ATOM(XdndAware)
	INIT_ATOM(XdndSelection)
	INIT_ATOM(XdndEnter)
	INIT_ATOM(XdndLeave)
	INIT_ATOM(XdndPosition)
	INIT_ATOM(XdndStatus)
	INIT_ATOM(XdndDrop)
	INIT_ATOM(XdndFinished)
	INIT_ATOM(XdndActionCopy)
	INIT_ATOM(XdndTypeList)
	INIT_ATOM(PRIMARY)

#undef INIT_ATOM
}

void LinuxDragAndDrop::ShutDown()
{
	sXDisplay = nullptr;
}

void LinuxDragAndDrop::makeDNDAware(::Window xWindow)
{
	u32 dndVersion = 5;
	XChangeProperty(sXDisplay, xWindow, sXdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&dndVersion, 1);
}

void LinuxDragAndDrop::registerDropTarget(DropTarget* target)
{
	Lock lock(sMutex);
	sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Register, target->GetArea()));
}

void LinuxDragAndDrop::unregisterDropTarget(DropTarget* target)
{
	Lock lock(sMutex);
	sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Unregister));
}

void LinuxDragAndDrop::updateDropTarget(DropTarget* target)
{
	Lock lock(sMutex);
	sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Update, target->GetArea()));
}

bool LinuxDragAndDrop::handleClientMessage(XClientMessageEvent& event)
{
	// First handle any queued registration/unregistration
	{
		Lock lock(sMutex);

		for(auto& entry : sQueuedAreaOperations)
		{
			switch(entry.type)
			{
			case DropAreaOpType::Register:
				sDropAreas.push_back(DropArea(entry.target, entry.area));
				break;
			case DropAreaOpType::Unregister:
				// Remove any operations queued for this target
				for(auto iter = sQueuedOperations.begin(); iter != sQueuedOperations.end();)
				{
					if(iter->target == entry.target)
						iter = sQueuedOperations.erase(iter);
					else
						++iter;
				}

				// Remove the area
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&](const DropArea& area)
												 { return area.target == entry.target; });

					sDropAreas.erase(iterFind);
				}

				break;
			case DropAreaOpType::Update:
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&](const DropArea& area)
												 { return area.target == entry.target; });

					if(iterFind != sDropAreas.end())
						iterFind->area = entry.area;
				}
				break;
			}
		}

		sQueuedAreaOperations.clear();
	}

	// Source window notifies us a drag has just entered our window area
	if(event.message_type == sXdndEnter)
	{
		sDNDSource = (::Window)event.data.l[0];
		bool isList = (event.data.l[1] & 1) != 0;
		sDNDVersion = (i32)(event.data.l[1] >> 24);

		// Get a list of properties are determine if there are any relevant ones
		Atom* propertyList = nullptr;
		u32 numProperties = 0;

		// If more than 3 properties we need to read the list property to get them all
		if(isList)
		{
			X11Property property = readX11Property(sXDisplay, sDNDSource, sXdndTypeList);

			propertyList = (Atom*)property.data;
			numProperties = property.count;
		}
		else
		{
			propertyList = B3DStackAllocate<Atom>(3);

			for(int i = 0; i < 3; i++)
			{
				if(event.data.l[2 + i])
				{
					propertyList[i] = (Atom)event.data.l[2 + i];
					numProperties++;
				}
			}
		}

		// Scan the list for URI list (file list), which is the only one we support (currently)
		bool foundSupportedType = false;
		for(u32 i = 0; i < numProperties; ++i)
		{
			char* name = XGetAtomName(sXDisplay, propertyList[i]);
			if(strcmp(name, "text/uri-list") == 0)
			{
				sDNDType = propertyList[i];

				XFree(name);
				foundSupportedType = true;
				break;
			}

			XFree(name);
		}

		// Free the property list
		if(isList)
			XFree(propertyList);
		else
			B3DStackFree(propertyList);

		sDragActive = foundSupportedType;
	}
	// Cursor moved while drag is active (also includes the initial cursor activity when drag entered)
	else if(event.message_type == sXdndPosition)
	{
		::Window source = (::Window)event.data.l[0];

		sDragPosition.x = (i32)((event.data.l[2] >> 16) & 0xFFFF);
		sDragPosition.y = (i32)((event.data.l[2]) & 0xFFFF);

		// Respond with a status message, we either accept or reject the dnd
		XClientMessageEvent response;
		B3DZeroOut(response);

		response.type = ClientMessage;
		response.display = event.display;
		response.window = source;
		response.message_type = sXdndStatus;
		response.format = 32;
		response.data.l[0] = event.window;
		response.data.l[1] = 0; // Reject drop by default
		response.data.l[2] = 0; // Empty rectangle
		response.data.l[3] = 0; // Empty rectangle
		response.data.l[4] = sXdndActionCopy;

		if(sDragActive)
		{
			for(auto& dropArea : sDropAreas)
			{
				LinuxWindow* linuxWindow;
				dropArea.target->GetOwnerWindowInternal()->GetCustomAttribute("LINUX_WINDOW", &linuxWindow);
				::Window xWindow = linuxWindow->GetXWindowInternal();

				if(xWindow == event.window)
				{
					Vector2I windowPos = linuxWindow->screenToWindowPos(sDragPosition);
					if(dropArea.area.contains(windowPos))
					{
						// Accept drop
						response.data.l[1] = 1;

						if(dropArea.target->IsActiveInternal())
						{
							Lock lock(sMutex);
							sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::DragOver, dropArea.target, windowPos));
						}
						else
						{
							Lock lock(sMutex);
							sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, dropArea.target, windowPos));
						}

						dropArea.target->SetActiveInternal(true);
					}
					else
					{
						// Cursor left previously active target's area
						if(dropArea.target->IsActiveInternal())
						{
							{
								Lock lock(sMutex);
								sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, dropArea.target));
							}

							dropArea.target->SetActiveInternal(false);
						}
					}
				}
			}
		}

		XSendEvent(sXDisplay, source, False, NoEventMask, (XEvent*)&response);
		XFlush(sXDisplay);
	}
	// Cursor left the target window, or the drop was rejected
	else if(event.message_type == sXdndLeave)
	{
		for(auto& dropArea : sDropAreas)
		{
			if(dropArea.target->IsActiveInternal())
			{
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, dropArea.target));
				}

				dropArea.target->SetActiveInternal(false);
			}
		}

		sDragActive = false;
	}
	else if(event.message_type == sXdndDrop)
	{
		::Window source = (::Window)event.data.l[0];
		bool dropAccepted = false;

		if(sDragActive)
		{
			for(auto& dropArea : sDropAreas)
			{
				if(dropArea.target->IsActiveInternal())
					dropAccepted = true;
			}
		}

		if(dropAccepted)
		{
			::Time timestamp;
			if(sDNDVersion >= 1)
				timestamp = (::Time)event.data.l[2];
			else
				timestamp = CurrentTime;

			XConvertSelection(sXDisplay, sXdndSelection, sDNDType, sPRIMARY, LinuxPlatform::getMainXWindow(), timestamp);

			// Now we wait for SelectionNotify
		}
		else
		{
			// Respond with a status message that we reject the drop
			XClientMessageEvent response;
			B3DZeroOut(response);

			response.type = ClientMessage;
			response.display = event.display;
			response.window = source;
			response.message_type = sXdndFinished;
			response.format = 32;
			response.data.l[0] = LinuxPlatform::getMainXWindow();
			response.data.l[1] = 0;
			response.data.l[2] = 0;
			response.data.l[3] = 0;
			response.data.l[4] = 0;

			XSendEvent(sXDisplay, source, False, NoEventMask, (XEvent*)&response);
			XFlush(sXDisplay);

			sDragActive = false;
		}
	}
	else
		return false;

	return true;
}

bool LinuxDragAndDrop::handleSelectionNotify(XSelectionEvent& event)
{
	if(event.target != sDNDType)
		return false;

	if(!sDragActive)
		return true;

	// Read data
	X11Property property = readX11Property(sXDisplay, LinuxPlatform::getMainXWindow(), sPRIMARY);
	if(property.format == 8)
	{
		// Assuming this is a file list, since we rejected any other drop type
		Vector<Path> filePaths;

		char* token = strtok((char*)property.data, "\r\n");
		while(token != nullptr)
		{
			char* filePath = convertURIToLocalPath(token);
			if(filePath != nullptr)
				filePaths.push_back(String(filePath));

			token = strtok(nullptr, "\r\n");
		}

		for(auto& dropArea : sDropAreas)
		{
			if(!dropArea.target->IsActiveInternal())
				continue;

			LinuxWindow* linuxWindow;
			dropArea.target->GetOwnerWindowInternal()->GetCustomAttribute("LINUX_WINDOW", &linuxWindow);

			Vector2I windowPos = linuxWindow->screenToWindowPos(sDragPosition);

			Lock lock(sMutex);
			sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Drop, dropArea.target, windowPos, filePaths));

			dropArea.target->SetActiveInternal(false);
		}
	}

	XFree(property.data);

	// Respond with a status message that we accepted the drop
	XClientMessageEvent response;
	B3DZeroOut(response);

	response.type = ClientMessage;
	response.display = event.display;
	response.window = sDNDSource;
	response.message_type = sXdndFinished;
	response.format = 32;
	response.data.l[0] = LinuxPlatform::getMainXWindow();
	response.data.l[1] = 1;
	response.data.l[2] = sXdndActionCopy;
	response.data.l[3] = 0;
	response.data.l[4] = 0;

	XSendEvent(sXDisplay, sDNDSource, False, NoEventMask, (XEvent*)&response);
	XFlush(sXDisplay);

	sDragActive = false;

	return true;
}

void LinuxDragAndDrop::update()
{
	Vector<DragAndDropOp> operations;

	{
		Lock lock(sMutex);
		std::swap(operations, sQueuedOperations);
	}

	for(auto& op : operations)
	{
		switch(op.type)
		{
		case DragAndDropOpType::Enter:
			op.target->onEnter(op.position.x, op.position.y);
			break;
		case DragAndDropOpType::DragOver:
			op.target->onDragOver(op.position.x, op.position.y);
			break;
		case DragAndDropOpType::Drop:
			op.target->SetFileListInternal(op.fileList);
			op.target->onDrop(op.position.x, op.position.y);
			break;
		case DragAndDropOpType::Leave:
			op.target->ClearInternal();
			op.target->onLeave();
			break;
		}
	}
}
