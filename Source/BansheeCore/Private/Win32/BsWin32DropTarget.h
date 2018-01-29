//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsVector2I.h"
#include <windows.h>
#include <oleidl.h>

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/**
	* Called by the OS when various drag and drop actions are performed over a window this control is registered for.
	* 			
	* @note		
	* This class queues all messages receives by the OS (from the core thread), and then executes the queue on sim thread.
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
				:type(_type), position(_pos), dataType(DropOpDataType::None)
			{ }

			DropOpType type;
			Vector2I position;

			DropOpDataType dataType;
			Vector<Path>* mFileList;
		};

	public:
		Win32DropTarget(HWND hWnd);
		~Win32DropTarget();

		void registerWithOS();
		void unregisterWithOS();

		HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override;

		/**	COM requirement. Increments objects reference count. */
		ULONG __stdcall AddRef() override;

		/** COM requirement. Decreases the objects reference count and deletes the object if its zero. */
		ULONG __stdcall Release() override;

		/**
		 * Called by the OS when user enters the drop target area while dragging an object.
		 * 			
		 * @note	Called on core thread.
		 */
		HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		/**
		 * Called by the OS while user continues to drag an object over the drop target.
		 * 			
		 * @note	Called on core thread.
		 */
		HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		/**
		 * Called by the OS when user leaves the drop target.
		 * 			
		 * @note	Called on core thread.
		 */
		HRESULT __stdcall DragLeave() override;

		/**
		 * Called by the OS when the user ends the drag operation while over the drop target.
		 * 			
		 * @note	Called on core thread.
		 */
		HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		/**
		 * Registers a new drop target to monitor.
		 *
		 * @note	Sim thread only.
		 */
		void registerDropTarget(DropTarget* dropTarget);

		/**
		 * Unregisters an existing drop target and stops monitoring it.
		 *
		 * @note	Sim thread only.
		 */
		void unregisterDropTarget(DropTarget* dropTarget);

		/**
		 * Gets the total number of monitored drop targets.
		 * 			
		 * @note	Sim thread only.
		 */
		unsigned int getNumDropTargets() const;

		/** Called every frame by the sim thread. Internal method. */
		void update();
	private:
		/**	Check if we support the data in the provided drag and drop data object. */
		bool isDataValid(IDataObject* data);

		/**	Gets a file list from data. Caller must ensure that the data actually contains a file list. */
		Vector<Path>* getFileListFromData(IDataObject* data);

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
}