//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Scene/BsComponent.h"
#include "Profiling/BsProfilerGPU.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	class ProfilerOverlay;

	/**	Handles rendering of Profiler information as an overlay in a viewport. */
	class BS_EXPORT ProfilerOverlay
	{
	public:
		/**	Holds data about GUI elements in a single row of a "CPU basic" sample. */
		struct BasicRow
		{
			GUILayout* labelLayout;
			GUILayout* contentLayout;
			GUIFixedSpace* labelSpace;

			GUILabel* guiName;
			GUILabel* guiPctOfParent;
			GUILabel* guiNumCalls;
			GUILabel* guiNumAllocs;
			GUILabel* guiNumFrees;
			GUILabel* guiAvgTime;
			GUILabel* guiTotalTime;
			GUILabel* guiAvgTimeSelf;
			GUILabel* guiTotalTimeSelf;

			HString name;
			HString pctOfParent;
			HString numCalls;
			HString numAllocs;
			HString numFrees;
			HString avgTime;
			HString totalTime;
			HString avgTimeSelf;
			HString totalTimeSelf;

			bool disabled;
		};

		/**	Holds data about GUI elements in a single row of a "CPU precise" sample. */
		struct PreciseRow
		{
			GUILayout* labelLayout;
			GUILayout* contentLayout;
			GUIFixedSpace* labelSpace;

			GUILabel* guiName;
			GUILabel* guiPctOfParent;
			GUILabel* guiNumCalls;
			GUILabel* guiNumAllocs;
			GUILabel* guiNumFrees;
			GUILabel* guiAvgCycles;
			GUILabel* guiTotalCycles;
			GUILabel* guiAvgCyclesSelf;
			GUILabel* guiTotalCyclesSelf;

			HString name;
			HString pctOfParent;
			HString numCalls;
			HString numAllocs;
			HString numFrees;
			HString avgCycles;
			HString totalCycles;
			HString avgCyclesSelf;
			HString totalCyclesSelf;

			bool disabled;
		};
		
		/**	Holds data about GUI elements in a single row of a GPU sample. */
		struct GPUSampleRow
		{
			GUILayout* labelLayout;
			GUILayout* contentLayout;
			GUIFixedSpace* labelSpace;

			GUILabel* guiName;
			GUILabel* guiTime;

			HString name;
			HString time;

			bool disabled;
		};

	public:
		/**	Constructs a new overlay attached to the specified parent and displayed on the provided camera. */
		ProfilerOverlay(const SPtr<Camera>& camera);
		~ProfilerOverlay();

		/**	Changes the camera to display the overlay on. */
		void setTarget(const SPtr<Camera>& camera);

		/**	Shows the overlay of the specified type. */
		void show(ProfilerOverlayType type);

		/**	Hides the overlay. */
		void hide();

		/**	Updates overlay contents. Should be called once per frame. */
		void update();
	private:
		/**	Called whenever the viewport resizes in order to rearrange the GUI elements. */
		void targetResized();

		/** Updates sizes of GUI areas used for displaying CPU sample data. To be called after viewport change or resize. */
		void updateCPUSampleAreaSizes();

		/** Updates sizes of GUI areas used for displaying GPU sample data. To be called after viewport change or resize. */
		void updateGPUSampleAreaSizes();

		/**
		 * Updates CPU GUI elements from the data in the provided profiler reports. To be called whenever a new report is 
		 * received.
		 */
		void updateCPUSampleContents(const ProfilerReport& simReport, const ProfilerReport& coreReport);

		/**
		 * Updates GPU GUI elemnts from the data in the provided profiler report. To be called whenever a new report is
		 * received.
		 */
		void updateGPUSampleContents(const GPUProfilerReport& gpuReport);

		static constexpr UINT32 GPU_NUM_SAMPLE_COLUMNS = 3;

		ProfilerOverlayType mType;
		SPtr<Viewport> mTarget;

		HSceneObject mWidgetSO;
		HGUIWidget mWidget;

		GUILayout* mBasicLayoutLabels = nullptr;
		GUILayout* mPreciseLayoutLabels = nullptr;
		GUILayout* mBasicLayoutContents = nullptr;
		GUILayout* mPreciseLayoutContents = nullptr;

		GUIElement* mTitleBasicName = nullptr;
		GUIElement* mTitleBasicPctOfParent = nullptr;
		GUIElement* mTitleBasicNumCalls = nullptr;
		GUIElement* mTitleBasicNumAllocs = nullptr;
		GUIElement* mTitleBasicNumFrees = nullptr;
		GUIElement* mTitleBasicAvgTime = nullptr;
		GUIElement* mTitleBasicTotalTime = nullptr;
		GUIElement* mTitleBasicAvgTitleSelf = nullptr;
		GUIElement* mTitleBasicTotalTimeSelf = nullptr;

		GUIElement* mTitlePreciseName = nullptr;
		GUIElement* mTitlePrecisePctOfParent = nullptr;
		GUIElement* mTitlePreciseNumCalls = nullptr;
		GUIElement* mTitlePreciseNumAllocs = nullptr;
		GUIElement* mTitlePreciseNumFrees = nullptr;
		GUIElement* mTitlePreciseAvgCycles = nullptr;
		GUIElement* mTitlePreciseTotalCycles = nullptr;
		GUIElement* mTitlePreciseAvgCyclesSelf = nullptr;
		GUIElement* mTitlePreciseTotalCyclesSelf = nullptr;

		GUILayout* mGPULayoutFrameContents = nullptr;
		GUILayout* mGPULayoutFrameContentsLeft = nullptr;
		GUILayout* mGPULayoutFrameContentsRight = nullptr;
		GUILayout* mGPULayoutSamples = nullptr;
		GUILayout* mGPULayoutSampleLabels[GPU_NUM_SAMPLE_COLUMNS] = { };
		GUILayout* mGPULayoutSampleContents[GPU_NUM_SAMPLE_COLUMNS] = { };

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
		Vector<GPUSampleRow> mGPUSampleRows[GPU_NUM_SAMPLE_COLUMNS];

		HEvent mTargetResizedConn;
		bool mIsShown;
		UINT32 mNumGPUSamplesPerColumn = 20;
	};

	/** @} */
}