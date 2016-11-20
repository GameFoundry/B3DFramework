//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPlatform.h"
#include "Win32/BsWin32Defs.h"
#include "Win32/BsWin32DropTarget.h"

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Encapsulate native cursor data so we can avoid including windows.h as it pollutes the global namespace. */
	struct BS_CORE_EXPORT NativeCursorData
	{
		HCURSOR cursor;
	};

	/**	Encapsulate drop target data so we can avoid including windows.h as it pollutes the global namespace. */
	struct BS_CORE_EXPORT NativeDropTargetData
	{
		Map<const RenderWindow*, Win32DropTarget*> dropTargetsPerWindow;
		Vector<Win32DropTarget*> dropTargetsToInitialize;
		Vector<Win32DropTarget*> dropTargetsToDestroy;
	};

	struct Platform::Pimpl
	{
		bool mIsCursorHidden = false;
		NativeCursorData mCursor;
		bool mUsingCustomCursor = false;
		Map<const RenderWindowCore*, WindowNonClientAreaData> mNonClientAreas;

		bool mIsTrackingMouse = false;
		NativeDropTargetData mDropTargets;

		bool mRequiresStartUp = false;
		bool mRequiresShutDown = false;

		Mutex mSync;
	};

	/** @} */
}