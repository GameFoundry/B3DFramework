//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Input/B3DInput.h"
#include "Utility/B3DEvent.h"
#include <atomic>

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Holds data returned by DragAndDropManager callbacks. */
	struct B3D_EXPORT DragCallbackInfo
	{
		bool Processed = false;
	};

	/** Base type that should be inherited to provide specific data relevant to a drag and drop operation. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() DragAndDropData : public IReflectable, public IScriptExportable
	{
	public:
		B3D_SCRIPT_EXPORT()
		DragAndDropData() = default;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class DragAndDropDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information about a single or multiple dragged scene objects. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() SceneObjectDragAndDropData : public DragAndDropData
	{
	public:
		B3D_SCRIPT_EXPORT()
		SceneObjectDragAndDropData() = default;

		B3D_SCRIPT_EXPORT()
		SceneObjectDragAndDropData(const HSceneObject& sceneObject)
			: SceneObjects { sceneObject }
		{ }

		B3D_SCRIPT_EXPORT()
		SceneObjectDragAndDropData(const Vector<HSceneObject>& sceneObjects)
			: SceneObjects(sceneObjects)
		{ }

		B3D_SCRIPT_EXPORT()
		Vector<HSceneObject> SceneObjects;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SceneObjectDragAndDropDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information about a single or multiple dragged resources, represented as paths. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() ResourceDragAndDropData : public DragAndDropData
	{
	public:
		B3D_SCRIPT_EXPORT()
		ResourceDragAndDropData() = default;

		B3D_SCRIPT_EXPORT()
		ResourceDragAndDropData(const Path& relativeResourcePath)
			: RelativeResourcePaths { relativeResourcePath }
		{ }

		B3D_SCRIPT_EXPORT()
		ResourceDragAndDropData(const Vector<Path>& relativeResourcePaths)
			: RelativeResourcePaths(relativeResourcePaths)
		{ }

		B3D_SCRIPT_EXPORT()
		Vector<Path> RelativeResourcePaths;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ResourceDragAndDropDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Handles GUI drag and drop operations. When active GUI elements will be notified of any drag events and will be able
	 * to retrieve dragged data. When cursor is released the data will be dropped and underlying GUI elements will be
	 * notified the data is dropped. The dropped data is also available for a single frame via GetDropData() method.
	 *
	 * @note	Main thread only.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() DragAndDrop : public Module<DragAndDrop>
	{
	public:
		DragAndDrop();
		~DragAndDrop();

		/**
		 * Starts a drag operation with the specified data. This means GUI elements will start receiving drag and drop
		 * related events and they may choose to handle them.
		 *
		 * @param	data					Some operation specific data that is just passed through to however needs it.
		 */
		B3D_SCRIPT_EXPORT()
		void StartDrag(const SPtr<DragAndDropData>& data) { StartDrag(data, nullptr); }

		/**
		 * Starts a drag operation with the specified data. This means GUI elements will start receiving drag and drop
		 * related events and they may choose to handle them.
		 *
		 * @param	data					Some operation specific data that is just passed through to however needs it.
		 * @param	dropCallback			The drop callback that gets triggered whenever mouse button is released and
		 *									drag operation ends. You should perform any cleanup here.
		 * @param	needsValidDropTarget	(optional) Determines whether the drop operation may happen anywhere or
		 *									does the GUI element need to specifically accept the drag of this type.
		 *									If false all GUI elements we mouse over will receive drag/drop events,
		 *									otherwise only those that specifically subscribe to the specified drag
		 *									operation of this typeId will.
		 * 									Additionally this will determine the cursor displayed (whether or not it
		 *									can have a "denied" state).
		 */
		void StartDrag(const SPtr<DragAndDropData>& data, Function<void(bool)>&& dropCallback, bool needsValidDropTarget = false);

		/**	Returns true if drag is currently in progress. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(IsDragInProgress))
		bool IsDragInProgress() const { return mDragData != nullptr; }

		/** Returns true if a drop operation happened this frame. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(IsDropInProgress))
		bool IsDropInProgress() const { return mDropData != nullptr && mIsDropActiveThisFrame; }

		/**	Get RTTI ID of the dragged data. Only valid if drag is in progress. */
		u32 GetDragTypeId() const { return mDragData != nullptr ? mDragData->GetTypeId() : ~0u; }

		/**	Get RTTI ID of the dropped data. Only valid if drop is in progress. */
		u32 GetDropTypeId() const { return mDropData != nullptr ? mDropData->GetTypeId() : ~0u; }

		/**	Gets drag specific data specified when the drag started. Only valid if drag is in progress. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(DragData))
		SPtr<DragAndDropData> GetDragData() const { return mDragData; }

		/**	Gets drag specific data specified when the drag started. Only valid if drop is in progress. This is only valid for a single frame when a drop happens. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(DropData))
		SPtr<DragAndDropData> GetDropData() const { return mDropData; }

		/**
		 * Determines whether the drop operation may happen anywhere or does the GUI element need to specifically accept the
		 * drag of this type. If false all GUI elements we mouse over will receive drag/drop events, otherwise only those
		 * that specifically subscribe to the specified drag operation of this typeId will.
		 *
		 * Additionally this will determine the cursor displayed (whether or not it can have a "denied" state).
		 */
		bool NeedsValidDropTarget() const { return mNeedsValidDropTarget; }

		/**
		 * Registers a new callback that will be triggered when dragged item is dropped. Provided parameter specifies if
		 * the drop operation was handled by anyone or not.
		 */
		void AddDropCallback(Function<void(bool)>&& dropCallback);

		/** Called once per frame. Checks if drag ended, drop started/ended or if window loses focus. */
		void Update();

		/**
		 * Triggers a callback when user releases the pointer and the drag operation ends. Provided parameters inform the
		 * subscriber where the pointer was released, and allows the subscriber to note whether the drag operation was
		 * processed or not.
		 *
		 * @note	Internal event. You should use addDropCallback for normal use.
		 */
		Event<void(const PointerEvent&, DragCallbackInfo&)> OnDragEnded;

	private:
		/**	Triggers any drop callbacks, clears callback data and starts drop operation. */
		void EndDrag(bool processed);

		/**
		 * Called by the render thread whenever mouse capture state changes. This can happen when window loses focus
		 * (for example alt+tab). In that case we want to end the drag even if the user is still holding the dragged item.
		 *
		 * @note	Render thread.
		 */
		void MouseCaptureChanged();

		/**	Called by the input system when pointer is released. */
		void CursorReleased(const PointerEvent& event);

	private:
		SPtr<DragAndDropData> mDragData;
		SPtr<DragAndDropData> mDropData;
		Vector<Function<void(bool)>> mDropCallbacks;
		bool mNeedsValidDropTarget = false;
		bool mIsDropActiveThisFrame = false;
		HEvent mMouseCaptureChangedConn;

		std::atomic<bool> mCaptureChanged{ false };
		std::atomic<int> mCaptureActive{ 0 };
		std::atomic<u64> mCaptureChangeFrame;
	};

	/** @} */
} // namespace b3d
