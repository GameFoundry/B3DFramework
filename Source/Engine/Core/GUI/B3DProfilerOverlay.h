//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "Profiling/B3DProfilerGPU.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup Profiling
	 *  @{
	 */

	class ProfilerOverlay;

	/**	Handles rendering of Profiler information as an overlay in a viewport. */
	class B3D_EXPORT ProfilerOverlay
	{
	public:
		/**	Holds data about GUI elements in a single row of a "CPU basic" sample. */
		struct BasicRow
		{
			GUILayout* LabelLayout;
			GUILayout* ContentLayout;
			GUIFixedSpace* LabelSpace;

			GUILabel* GuiName;
			GUILabel* GuiPctOfParent;
			GUILabel* GuiNumCalls;
			GUILabel* GuiNumAllocs;
			GUILabel* GuiNumFrees;
			GUILabel* GuiAvgTime;
			GUILabel* GuiTotalTime;
			GUILabel* GuiAvgTimeSelf;
			GUILabel* GuiTotalTimeSelf;

			HString Name;
			HString PctOfParent;
			HString NumCalls;
			HString NumAllocs;
			HString NumFrees;
			HString AvgTime;
			HString TotalTime;
			HString AvgTimeSelf;
			HString TotalTimeSelf;

			bool Disabled;
		};

		/**	Holds data about GUI elements in a single row of a "CPU precise" sample. */
		struct PreciseRow
		{
			GUILayout* LabelLayout;
			GUILayout* ContentLayout;
			GUIFixedSpace* LabelSpace;

			GUILabel* GuiName;
			GUILabel* GuiPctOfParent;
			GUILabel* GuiNumCalls;
			GUILabel* GuiNumAllocs;
			GUILabel* GuiNumFrees;
			GUILabel* GuiAvgCycles;
			GUILabel* GuiTotalCycles;
			GUILabel* GuiAvgCyclesSelf;
			GUILabel* GuiTotalCyclesSelf;

			HString Name;
			HString PctOfParent;
			HString NumCalls;
			HString NumAllocs;
			HString NumFrees;
			HString AvgCycles;
			HString TotalCycles;
			HString AvgCyclesSelf;
			HString TotalCyclesSelf;

			bool Disabled;
		};

		/**	Holds data about GUI elements in a single row of a GPU sample. */
		struct GPUSampleRow
		{
			GUILayout* LabelLayout;
			GUILayout* ContentLayout;
			GUIFixedSpace* LabelSpace;

			GUILabel* GuiName;
			GUILabel* GuiTime;

			HString Name;
			HString Time;

			bool Disabled;
		};

	public:
		/**	Constructs a new overlay attached to the specified parent and displayed on the provided camera. */
		ProfilerOverlay(const HCamera& camera);
		~ProfilerOverlay();

		/**	Changes the camera to display the overlay on. */
		void SetTarget(const HCamera& camera);

		/**	Shows the overlay of the specified type. */
		void Show(ProfilerOverlayType type);

		/**	Hides the overlay. */
		void Hide();

		/**	Updates overlay contents. Should be called once per frame. */
		void Update();

	private:
		/**	Called whenever the viewport resizes in order to rearrange the GUI elements. */
		void TargetResized();

		/** Updates sizes of GUI areas used for displaying CPU sample data. To be called after viewport change or resize. */
		void UpdateCpuSampleAreaSizes();

		/** Updates sizes of GUI areas used for displaying GPU sample data. To be called after viewport change or resize. */
		void UpdateGpuSampleAreaSizes();

		/**
		 * Updates CPU GUI elements from the data in the provided profiler reports. To be called whenever a new report is
		 * received.
		 */
		void UpdateCpuSampleContents(const ProfilerReport& mainThreadReport, const ProfilerReport& renderThreadReport);

		/**
		 * Updates GPU GUI elemnts from the data in the provided profiler report. To be called whenever a new report is
		 * received.
		 */
		void UpdateGpuSampleContents(const GpuProfilerSample& gpuReport);

		static constexpr u32 kGpuNumSampleColumns = 3;

		GpuProfilerResults mLastProfilerResults;

		ProfilerOverlayType mType;
		SPtr<Viewport> mTarget;

		HSceneObject mWidgetSO;
		HGUIWidget mWidget;

		GUILayout* mBasicLayoutLabels = nullptr;
		GUILayout* mPreciseLayoutLabels = nullptr;
		GUILayout* mBasicLayoutContents = nullptr;
		GUILayout* mPreciseLayoutContents = nullptr;

		GUIInteractable* mTitleBasicName = nullptr;
		GUIInteractable* mTitleBasicPctOfParent = nullptr;
		GUIInteractable* mTitleBasicNumCalls = nullptr;
		GUIInteractable* mTitleBasicNumAllocs = nullptr;
		GUIInteractable* mTitleBasicNumFrees = nullptr;
		GUIInteractable* mTitleBasicAvgTime = nullptr;
		GUIInteractable* mTitleBasicTotalTime = nullptr;
		GUIInteractable* mTitleBasicAvgTitleSelf = nullptr;
		GUIInteractable* mTitleBasicTotalTimeSelf = nullptr;

		GUIInteractable* mTitlePreciseName = nullptr;
		GUIInteractable* mTitlePrecisePctOfParent = nullptr;
		GUIInteractable* mTitlePreciseNumCalls = nullptr;
		GUIInteractable* mTitlePreciseNumAllocs = nullptr;
		GUIInteractable* mTitlePreciseNumFrees = nullptr;
		GUIInteractable* mTitlePreciseAvgCycles = nullptr;
		GUIInteractable* mTitlePreciseTotalCycles = nullptr;
		GUIInteractable* mTitlePreciseAvgCyclesSelf = nullptr;
		GUIInteractable* mTitlePreciseTotalCyclesSelf = nullptr;

		GUILayout* mGPULayoutFrameContents = nullptr;
		GUILayout* mGPULayoutFrameContentsLeft = nullptr;
		GUILayout* mGPULayoutFrameContentsRight = nullptr;
		GUILayout* mGPULayoutSamples = nullptr;
		GUILayout* mGPULayoutSampleLabels[kGpuNumSampleColumns] = {};
		GUILayout* mGPULayoutSampleContents[kGpuNumSampleColumns] = {};

		GUILabel* mGPUFrameNumLbl = nullptr;
		GUILabel* mGPUTimeLbl = nullptr;
		GUILabel* mGPUDrawCallsLbl = nullptr;
		GUILabel* mGPURenTargetChangesLbl = nullptr;
		GUILabel* mGPUPresentsLbl = nullptr;
		GUILabel* mGPUClearsLbl = nullptr;
		GUILabel* mGPUVerticesLbl = nullptr;
		GUILabel* mGPUPrimitivesLbl = nullptr;
		GUILabel* mGPUSamplesLbl = nullptr;
		GUILabel* mGPUPipelineStateChangesLbl = nullptr;

		GUILabel* mGPUObjectsCreatedLbl = nullptr;
		GUILabel* mGPUObjectsDestroyedLbl = nullptr;
		GUILabel* mGPUResourceWritesLbl = nullptr;
		GUILabel* mGPUResourceReadsLbl = nullptr;
		GUILabel* mGPUParamBindsLbl = nullptr;
		GUILabel* mGPUVertexBufferBindsLbl = nullptr;
		GUILabel* mGPUIndexBufferBindsLbl = nullptr;

		HString mGPUFrameNumStr;
		HString mGPUTimeStr;
		HString mGPUDrawCallsStr;
		HString mGPURenTargetChangesStr;
		HString mGPUPresentsStr;
		HString mGPUClearsStr;
		HString mGPUVerticesStr;
		HString mGPUPrimitivesStr;
		HString mGPUSamplesStr;
		HString mGPUPipelineStateChangesStr;

		HString mGPUObjectsCreatedStr;
		HString mGPUObjectsDestroyedStr;
		HString mGPUResourceWritesStr;
		HString mGPUResourceReadsStr;
		HString mGPUParamBindsStr;
		HString mGPUVertexBufferBindsStr;
		HString mGPUIndexBufferBindsStr;

		Vector<BasicRow> mBasicRows;
		Vector<PreciseRow> mPreciseRows;
		Vector<GPUSampleRow> mGPUSampleRows[kGpuNumSampleColumns];

		HEvent mTargetResizedConn;
		bool mIsShown;
		u32 mNumGPUSamplesPerColumn = 20;
		float mDPIScale = 1.0f; // TODO - Hook this up with actual window DPI
	};

	/** @} */
} // namespace b3d
