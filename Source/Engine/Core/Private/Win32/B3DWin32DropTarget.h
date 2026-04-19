//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include <windows.h>
#include <oleidl.h>

namespace b3d
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/**
	 * Called by the OS when various drag and drop actions are performed over a window this control is registered for.
	 *
	 * @note
	 * This class queues all messages receives by the OS (from the render thread), and then executes the queue on main thread.
	 * You should be wary of which methods are allowed to be called from which thread.
	 */
	class Win32DropTarget : public IDropTarget
	{
		/** Type of drag and drop event. */
		enum class DropOpType
		{
			DragOver,
			Drop,
			Leave
		};

		/** Type of data that a drag and drop operation contains. */
		enum class DropOpDataType
		{
			FileList,
			None
		};

		/**	Structure describing a drag and drop operation. */
		struct DropTargetOp
		{
			DropTargetOp(DropOpType _type, const Vector2I& _pos)
				: Type(_type), Position(_pos), DataType(DropOpDataType::None)
			{}

			DropOpType Type;
			Vector2I Position;

			DropOpDataType DataType;
			Vector<Path>* MFileList;
		};

	public:
		Win32DropTarget(HWND hWnd);
		~Win32DropTarget();

		void RegisterWithOs();
		void UnregisterWithOs();

		HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override;

		/**	COM requirement. Increments objects reference count. */
		ULONG __stdcall AddRef() override;

		/** COM requirement. Decreases the objects reference count and deletes the object if its zero. */
		ULONG __stdcall Release() override;

		/**
		 * Called by the OS when user enters the drop target area while dragging an object.
		 *
		 * @note	Called on render thread.
		 */
		HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		/**
		 * Called by the OS while user continues to drag an object over the drop target.
		 *
		 * @note	Called on render thread.
		 */
		HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		/**
		 * Called by the OS when user leaves the drop target.
		 *
		 * @note	Called on render thread.
		 */
		HRESULT __stdcall DragLeave() override;

		/**
		 * Called by the OS when the user ends the drag operation while over the drop target.
		 *
		 * @note	Called on render thread.
		 */
		HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		/**
		 * Registers a new drop target to monitor.
		 *
		 * @note	Main thread only.
		 */
		void RegisterDropTarget(DropTarget* dropTarget);

		/**
		 * Unregisters an existing drop target and stops monitoring it.
		 *
		 * @note	Main thread only.
		 */
		void UnregisterDropTarget(DropTarget* dropTarget);

		/**
		 * Gets the total number of monitored drop targets.
		 *
		 * @note	Main thread only.
		 */
		unsigned int GetNumDropTargets() const;

		/** Called every frame by the main thread. Internal method. */
		void Update();

	private:
		/**	Check if we support the data in the provided drag and drop data object. */
		bool IsDataValid(IDataObject* data);

		/**	Gets a file list from data. Caller must ensure that the data actually contains a file list. */
		Vector<Path>* GetFileListFromData(IDataObject* data);

	private:
		Vector<DropTarget*> mDropTargets;

		LONG mRefCount;
		HWND mHWnd;
		bool mAcceptDrag;

		Vector<DropTargetOp> mQueuedDropOps;
		Vector<Vector<Path>*> mFileLists;

		Mutex mSync;
	};

	/** @} */
} // namespace b3d
