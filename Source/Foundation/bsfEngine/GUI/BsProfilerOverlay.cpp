//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsProfilerOverlay.h"
#include "Scene/BsSceneObject.h"
#include "GUI/BsCGUIWidget.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUISpace.h"
#include "RenderAPI/BsViewport.h"
#include "Utility/BsTime.h"
#include "Resources/BsBuiltinResources.h"
#include "Profiling/BsProfilingManager.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Renderer/BsCamera.h"
#include "Localization/BsHEString.h"

#define BS_SHOW_PRECISE_PROFILING 0

namespace bs
{
	constexpr UINT32 MAX_DEPTH = 4;

	class BasicRowFiller
	{
	public:
		UINT32 curIdx;
		GUILayout& labelLayout;
		GUILayout& contentLayout;
		GUIWidget& widget;
		Vector<ProfilerOverlay::BasicRow>& rows;

		BasicRowFiller(Vector<ProfilerOverlay::BasicRow>& _rows, GUILayout& _labelLayout, GUILayout& _contentLayout, GUIWidget& _widget)
			:curIdx(0), labelLayout(_labelLayout), contentLayout(_contentLayout), widget(_widget), rows(_rows)
		{ }

		~BasicRowFiller()
		{
			UINT32 excessEntries = (UINT32)rows.size() - curIdx;
			for(UINT32 i = 0; i < excessEntries; i++)
			{
				ProfilerOverlay::BasicRow& row = rows[curIdx + i];

				if (!row.disabled)
				{
					row.labelLayout->setVisible(false);
					row.contentLayout->setVisible(false);
					row.disabled = true;
				}
			}

			rows.resize(curIdx);
		}

		void addData(UINT32 depth, const String& name, float pctOfParent, UINT32 numCalls, UINT64 numAllocs, 
			UINT64 numFrees, double avgTime, double totalTime, double avgSelfTime, double totalSelfTime)
		{
			if(curIdx >= rows.size())
			{
				rows.push_back(ProfilerOverlay::BasicRow());

				ProfilerOverlay::BasicRow& newRow = rows.back();

				newRow.disabled = false;
				newRow.name = HEString(u8"{0}");
				newRow.pctOfParent = HEString(u8"{0} %");
				newRow.numCalls = HEString(u8"{0}");
				newRow.numAllocs = HEString(u8"{0}");
				newRow.numFrees = HEString(u8"{0}");
				newRow.avgTime = HEString(u8"{0}");
				newRow.totalTime = HEString(u8"{0}");
				newRow.avgTimeSelf = HEString(u8"{0}");
				newRow.totalTimeSelf = HEString(u8"{0}");

				newRow.labelLayout = labelLayout.insertNewElement<GUILayoutX>(labelLayout.getNumChildren() - 1); // Insert before flexible space
				newRow.contentLayout = contentLayout.insertNewElement<GUILayoutX>(contentLayout.getNumChildren() - 1); // Insert before flexible space

				newRow.labelSpace = newRow.labelLayout->addNewElement<GUIFixedSpace>(0);
				newRow.guiName = newRow.labelLayout->addNewElement<GUILabel>(newRow.name, GUIOptions(GUIOption::fixedWidth(200)));

				newRow.guiPctOfParent = newRow.contentLayout->addNewElement<GUILabel>(newRow.pctOfParent, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiNumCalls = newRow.contentLayout->addNewElement<GUILabel>(newRow.numCalls, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiNumAllocs = newRow.contentLayout->addNewElement<GUILabel>(newRow.numAllocs, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiNumFrees = newRow.contentLayout->addNewElement<GUILabel>(newRow.numFrees, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiAvgTime = newRow.contentLayout->addNewElement<GUILabel>(newRow.avgTime, GUIOptions(GUIOption::fixedWidth(60)));
				newRow.guiTotalTime = newRow.contentLayout->addNewElement<GUILabel>(newRow.totalTime, GUIOptions(GUIOption::fixedWidth(60)));
				newRow.guiAvgTimeSelf = newRow.contentLayout->addNewElement<GUILabel>(newRow.avgTimeSelf, GUIOptions(GUIOption::fixedWidth(100)));
				newRow.guiTotalTimeSelf = newRow.contentLayout->addNewElement<GUILabel>(newRow.totalTimeSelf, GUIOptions(GUIOption::fixedWidth(100)));
			}
			
			ProfilerOverlay::BasicRow& row = rows[curIdx];

			row.labelSpace->setSize(depth * 20);
			row.name.setParameter(0, name);
			row.pctOfParent.setParameter(0, toString(pctOfParent * 100.0f, 2, 0, ' ', std::ios::fixed));
			row.numCalls.setParameter(0, toString(numCalls));
			row.numAllocs.setParameter(0, toString(numAllocs));
			row.numFrees.setParameter(0, toString(numFrees));
			row.avgTime.setParameter(0, toString(avgTime, 2, 0, ' ', std::ios::fixed));
			row.totalTime.setParameter(0, toString(totalTime, 2, 0, ' ', std::ios::fixed));
			row.avgTimeSelf.setParameter(0, toString(avgSelfTime, 2, 0, ' ', std::ios::fixed));
			row.totalTimeSelf.setParameter(0, toString(totalSelfTime, 2, 0, ' ', std::ios::fixed));

			row.guiName->setContent(row.name);
			row.guiPctOfParent->setContent(row.pctOfParent);
			row.guiNumCalls->setContent(row.numCalls);
			row.guiNumAllocs->setContent(row.numAllocs);
			row.guiNumFrees->setContent(row.numFrees);
			row.guiAvgTime->setContent(row.avgTime);
			row.guiTotalTime->setContent(row.totalTime);
			row.guiAvgTimeSelf->setContent(row.avgTimeSelf);
			row.guiTotalTimeSelf->setContent(row.totalTimeSelf);

			if (row.disabled)
			{
				row.labelLayout->setVisible(true);
				row.contentLayout->setVisible(true);
				row.disabled = false;
			}

			curIdx++;
		}
	};

	class PreciseRowFiller
	{
	public:
		UINT32 curIdx;
		GUILayout& labelLayout;
		GUILayout& contentLayout;
		GUIWidget& widget;
		Vector<ProfilerOverlay::PreciseRow>& rows;

		PreciseRowFiller(Vector<ProfilerOverlay::PreciseRow>& _rows, GUILayout& _labelLayout, GUILayout& _contentLayout, GUIWidget& _widget)
			:curIdx(0), labelLayout(_labelLayout), contentLayout(_contentLayout), widget(_widget), rows(_rows)
		{ }

		~PreciseRowFiller()
		{
			UINT32 excessEntries = (UINT32)rows.size() - curIdx;
			for(UINT32 i = 0; i < excessEntries; i++)
			{
				ProfilerOverlay::PreciseRow& row = rows[curIdx + i];

				if (!row.disabled)
				{
					row.labelLayout->setVisible(false);
					row.contentLayout->setVisible(false);
					row.disabled = true;
				}
			}

			rows.resize(curIdx);
		}

		void addData(UINT32 depth, const String& name, float pctOfParent, UINT32 numCalls, UINT64 numAllocs, 
			UINT64 numFrees, UINT64 avgCycles, UINT64 totalCycles, UINT64 avgSelfCycles, UINT64 totalSelfCycles)
		{
			if(curIdx >= rows.size())
			{
				rows.push_back(ProfilerOverlay::PreciseRow());

				ProfilerOverlay::PreciseRow& newRow = rows.back();

				newRow.disabled = false;
				newRow.name = HEString(u8"{0}");
				newRow.pctOfParent = HEString(u8"{0}");
				newRow.numCalls = HEString(u8"{0}");
				newRow.numAllocs = HEString(u8"{0}");
				newRow.numFrees = HEString(u8"{0}");
				newRow.avgCycles = HEString(u8"{0}");
				newRow.totalCycles = HEString(u8"{0}");
				newRow.avgCyclesSelf = HEString(u8"{0}");
				newRow.totalCyclesSelf = HEString(u8"{0}");

				newRow.labelLayout = labelLayout.insertNewElement<GUILayoutX>(labelLayout.getNumChildren() - 1); // Insert before flexible space
				newRow.contentLayout = contentLayout.insertNewElement<GUILayoutX>(contentLayout.getNumChildren() - 1); // Insert before flexible space

				newRow.labelSpace = newRow.labelLayout->addNewElement<GUIFixedSpace>(0);
				newRow.guiName = newRow.labelLayout->addNewElement<GUILabel>(newRow.name, GUIOptions(GUIOption::fixedWidth(200)));

				newRow.guiPctOfParent = newRow.contentLayout->addNewElement<GUILabel>(newRow.pctOfParent, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiNumCalls = newRow.contentLayout->addNewElement<GUILabel>(newRow.numCalls, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiNumAllocs = newRow.contentLayout->addNewElement<GUILabel>(newRow.numAllocs, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiNumFrees = newRow.contentLayout->addNewElement<GUILabel>(newRow.numFrees, GUIOptions(GUIOption::fixedWidth(50)));
				newRow.guiAvgCycles = newRow.contentLayout->addNewElement<GUILabel>(newRow.avgCycles, GUIOptions(GUIOption::fixedWidth(60)));
				newRow.guiTotalCycles = newRow.contentLayout->addNewElement<GUILabel>(newRow.totalCycles, GUIOptions(GUIOption::fixedWidth(60)));
				newRow.guiAvgCyclesSelf = newRow.contentLayout->addNewElement<GUILabel>(newRow.avgCyclesSelf, GUIOptions(GUIOption::fixedWidth(100)));
				newRow.guiTotalCyclesSelf = newRow.contentLayout->addNewElement<GUILabel>(newRow.totalCyclesSelf, GUIOptions(GUIOption::fixedWidth(100)));
			}

			ProfilerOverlay::PreciseRow& row = rows[curIdx];

			row.labelSpace->setSize(depth * 20);
			row.name.setParameter(0, name);
			row.pctOfParent.setParameter(0, toString(pctOfParent * 100.0f, 2, 0, ' ', std::ios::fixed));
			row.numCalls.setParameter(0, toString(numCalls));
			row.numAllocs.setParameter(0, toString(numAllocs));
			row.numFrees.setParameter(0, toString(numFrees));
			row.avgCycles.setParameter(0, toString(avgCycles));
			row.totalCycles.setParameter(0, toString(totalCycles));
			row.avgCyclesSelf.setParameter(0, toString(avgSelfCycles));
			row.totalCyclesSelf.setParameter(0, toString(totalSelfCycles));

			row.guiName->setContent(row.name);
			row.guiPctOfParent->setContent(row.pctOfParent);
			row.guiNumCalls->setContent(row.numCalls);
			row.guiNumAllocs->setContent(row.numAllocs);
			row.guiNumFrees->setContent(row.numFrees);
			row.guiAvgCycles->setContent(row.avgCycles);
			row.guiTotalCycles->setContent(row.totalCycles);
			row.guiAvgCyclesSelf->setContent(row.avgCyclesSelf);
			row.guiTotalCyclesSelf->setContent(row.totalCyclesSelf);

			if (row.disabled)
			{
				row.labelLayout->setVisible(true);
				row.contentLayout->setVisible(true);
				row.disabled = false;
			}

			curIdx++;
		}
	};

	class GPUSampleRowFiller
	{
	public:
		UINT32 curIdx;
		GUILayout& labelLayout;
		GUILayout& contentLayout;
		GUIWidget& widget;
		Vector<ProfilerOverlay::GPUSampleRow>& rows;

		GPUSampleRowFiller(Vector<ProfilerOverlay::GPUSampleRow>& rows, GUILayout& labelLayout, GUILayout& contentLayout, 
			GUIWidget& _widget)
			:curIdx(0), labelLayout(labelLayout), contentLayout(contentLayout), widget(_widget), rows(rows)
		{ }

		~GPUSampleRowFiller()
		{
			UINT32 excessEntries = (UINT32)rows.size() - curIdx;
			for (UINT32 i = 0; i < excessEntries; i++)
			{
				ProfilerOverlay::GPUSampleRow& row = rows[curIdx + i];

				if (!row.disabled)
				{
					row.labelLayout->setVisible(false);
					row.contentLayout->setVisible(false);
					row.disabled = true;
				}
			}

			rows.resize(curIdx);
		}

		void addData(UINT32 depth, const String& name, float timeMs)
		{
			if (curIdx >= rows.size())
			{
				rows.push_back(ProfilerOverlay::GPUSampleRow());

				ProfilerOverlay::GPUSampleRow& newRow = rows.back();

				newRow.disabled = false;
				newRow.name = HEString(u8"{1}");
				newRow.time = HEString(u8"{0}");

				newRow.labelLayout = labelLayout.insertNewElement<GUILayoutX>(labelLayout.getNumChildren() - 1); // Insert before flexible space
				newRow.contentLayout = contentLayout.insertNewElement<GUILayoutX>(contentLayout.getNumChildren() - 1); // Insert before flexible space

				newRow.labelSpace = newRow.labelLayout->addNewElement<GUIFixedSpace>(0);
				newRow.guiName = newRow.labelLayout->addNewElement<GUILabel>(newRow.name, GUIOptions(GUIOption::fixedWidth(200)));

				newRow.guiTime = newRow.contentLayout->addNewElement<GUILabel>(newRow.time, GUIOptions(GUIOption::fixedWidth(100)));
			}

			ProfilerOverlay::GPUSampleRow& row = rows[curIdx];

			row.labelSpace->setSize(depth * 20);
			row.name.setParameter(0, name);
			row.time.setParameter(0, toString(timeMs));

			row.guiName->setContent(row.name);
			row.guiTime->setContent(row.time);

			if (row.disabled)
			{
				row.labelLayout->setVisible(false);
				row.contentLayout->setVisible(false);
				row.disabled = false;
			}

			curIdx++;
		}
	};

	ProfilerOverlay::ProfilerOverlay(const SPtr<Camera>& camera)
		:mType(ProfilerOverlayType::CPUSamples), mIsShown(true)
	{
		setTarget(camera);
	}

	ProfilerOverlay::~ProfilerOverlay()
	{
		if(mTarget != nullptr)
			mTargetResizedConn.disconnect();

		if(mWidgetSO)
			mWidgetSO->destroy();
	}

	void ProfilerOverlay::setTarget(const SPtr<Camera>& camera)
	{
		if(mTarget != nullptr)
			mTargetResizedConn.disconnect();

		mTarget = camera->getViewport();

		mTargetResizedConn = mTarget->getTarget()->onResized.connect(std::bind(&ProfilerOverlay::targetResized, this));

		if(mWidgetSO)
			mWidgetSO->destroy();

		mWidgetSO = SceneObject::create("ProfilerOverlay", SOF_Internal | SOF_Persistent | SOF_DontSave);
		mWidget = mWidgetSO->addComponent<CGUIWidget>(camera);
		mWidget->setDepth(127);
		mWidget->setSkin(BuiltinResources::instance().getGUISkin());

		// Set up CPU sample areas
		mBasicLayoutLabels = mWidget->getPanel()->addNewElement<GUILayoutY>();
		mPreciseLayoutLabels = mWidget->getPanel()->addNewElement<GUILayoutY>();
		mBasicLayoutContents = mWidget->getPanel()->addNewElement<GUILayoutY>();
		mPreciseLayoutContents = mWidget->getPanel()->addNewElement<GUILayoutY>();

		// Set up CPU sample title bars
		mTitleBasicName = GUILabel::create(HEString(u8"Name"), GUIOptions(GUIOption::fixedWidth(200)));
		mTitleBasicPctOfParent = GUILabel::create(HEString(u8"% parent"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicNumCalls = GUILabel::create(HEString(u8"# calls"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicNumAllocs = GUILabel::create(HEString(u8"# allocs"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicNumFrees = GUILabel::create(HEString(u8"# frees"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicAvgTime = GUILabel::create(HEString(u8"Avg. time"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitleBasicTotalTime = GUILabel::create(HEString(u8"Total time"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitleBasicAvgTitleSelf = GUILabel::create(HEString(u8"Avg. self time"), GUIOptions(GUIOption::fixedWidth(100)));
		mTitleBasicTotalTimeSelf = GUILabel::create(HEString(u8"Total self time"), GUIOptions(GUIOption::fixedWidth(100)));

		mTitlePreciseName = GUILabel::create(HEString(u8"Name"), GUIOptions(GUIOption::fixedWidth(200)));
		mTitlePrecisePctOfParent = GUILabel::create(HEString(u8"% parent"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseNumCalls = GUILabel::create(HEString(u8"# calls"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseNumAllocs = GUILabel::create(HEString(u8"# allocs"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseNumFrees = GUILabel::create(HEString(u8"# frees"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseAvgCycles = GUILabel::create(HEString(u8"Avg. cycles"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitlePreciseTotalCycles = GUILabel::create(HEString(u8"Total cycles"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitlePreciseAvgCyclesSelf = GUILabel::create(HEString(u8"Avg. self cycles"), GUIOptions(GUIOption::fixedWidth(100)));
		mTitlePreciseTotalCyclesSelf = GUILabel::create(HEString(u8"Total self cycles"), GUIOptions(GUIOption::fixedWidth(100)));

		GUILayout* basicTitleLabelLayout = mBasicLayoutLabels->addNewElement<GUILayoutX>();
		GUILayout* preciseTitleLabelLayout = mPreciseLayoutLabels->addNewElement<GUILayoutX>();
		GUILayout* basicTitleContentLayout = mBasicLayoutContents->addNewElement<GUILayoutX>();
		GUILayout* preciseTitleContentLayout = mPreciseLayoutContents->addNewElement<GUILayoutX>();

		basicTitleLabelLayout->addElement(mTitleBasicName);
		basicTitleContentLayout->addElement(mTitleBasicPctOfParent);
		basicTitleContentLayout->addElement(mTitleBasicNumCalls);
		basicTitleContentLayout->addElement(mTitleBasicNumAllocs);
		basicTitleContentLayout->addElement(mTitleBasicNumFrees);
		basicTitleContentLayout->addElement(mTitleBasicAvgTime);
		basicTitleContentLayout->addElement(mTitleBasicTotalTime);
		basicTitleContentLayout->addElement(mTitleBasicAvgTitleSelf);
		basicTitleContentLayout->addElement(mTitleBasicTotalTimeSelf);

		preciseTitleLabelLayout->addElement(mTitlePreciseName);
		preciseTitleContentLayout->addElement(mTitlePrecisePctOfParent);
		preciseTitleContentLayout->addElement(mTitlePreciseNumCalls);
		preciseTitleContentLayout->addElement(mTitlePreciseNumAllocs);
		preciseTitleContentLayout->addElement(mTitlePreciseNumFrees);
		preciseTitleContentLayout->addElement(mTitlePreciseAvgCycles);
		preciseTitleContentLayout->addElement(mTitlePreciseTotalCycles);
		preciseTitleContentLayout->addElement(mTitlePreciseAvgCyclesSelf);
		preciseTitleContentLayout->addElement(mTitlePreciseTotalCyclesSelf);

		mBasicLayoutLabels->addNewElement<GUIFlexibleSpace>();
		mPreciseLayoutLabels->addNewElement<GUIFlexibleSpace>();
		mBasicLayoutContents->addNewElement<GUIFlexibleSpace>();
		mPreciseLayoutContents->addNewElement<GUIFlexibleSpace>();

#if BS_SHOW_PRECISE_PROFILING == 0
		mPreciseLayoutLabels->setActive(false);
		mPreciseLayoutContents->setActive(false);
#endif

		// Set up GPU sample areas
		mGPULayoutFrameContents = mWidget->getPanel()->addNewElement<GUILayoutX>();
		mGPULayoutFrameContentsLeft = mGPULayoutFrameContents->addNewElement<GUILayoutY>();
		mGPULayoutFrameContentsRight = mGPULayoutFrameContents->addNewElement<GUILayoutY>();

		mGPULayoutSamples = mWidget->getPanel()->addNewElement<GUIPanel>();

		HString gpuSamplesStr(u8"__ProfOvGPUSamples", u8"Samples");
		mGPULayoutSamples->addNewElement<GUILabel>(gpuSamplesStr);

		for(UINT32 i = 0; i < GPU_NUM_SAMPLE_COLUMNS; i++)
		{
			mGPULayoutSampleLabels[i] = mGPULayoutSamples->addNewElement<GUILayoutY>();
			mGPULayoutSampleContents[i] = mGPULayoutSamples->addNewElement<GUILayoutY>();

			HString gpuSamplesNameStr(u8"__ProfOvGPUSampName", u8"Name");
			HString gpuSamplesTimeStr(u8"__ProfOvGPUSampTime", u8"Time");
			mGPULayoutSampleLabels[i]->addElement(GUILabel::create(gpuSamplesNameStr, GUIOptions(GUIOption::fixedWidth(200))));
			mGPULayoutSampleContents[i]->addElement(GUILabel::create(gpuSamplesTimeStr, GUIOptions(GUIOption::fixedWidth(100))));

			mGPULayoutSampleLabels[i]->addNewElement<GUIFlexibleSpace>();
			mGPULayoutSampleContents[i]->addNewElement<GUIFlexibleSpace>();
		}

		mGPUFrameNumStr = HEString(u8"__ProfOvFrame", u8"Frame #{0}");
		mGPUTimeStr = HEString(u8"__ProfOvTime", u8"Time: {0}ms");
		mGPUDrawCallsStr = HEString(u8"__ProfOvDrawCalls", u8"Draw calls: {0}");
		mGPURenTargetChangesStr = HEString(u8"__ProfOvRTChanges", u8"Render target changes: {0}");
		mGPUPresentsStr = HEString(u8"__ProfOvPresents", u8"Presents: {0}");
		mGPUClearsStr = HEString(u8"__ProfOvClears", u8"Clears: {0}");
		mGPUVerticesStr = HEString(u8"__ProfOvVertices", u8"Num. vertices: {0}");
		mGPUPrimitivesStr = HEString(u8"__ProfOvPrimitives", u8"Num. primitives: {0}");
		mGPUSamplesStr = HEString(u8"__ProfOvSamples", u8"Samples drawn: {0}");
		mGPUPipelineStateChangesStr = HEString(u8"__ProfOvPSChanges", u8"Pipeline state changes: {0}");

		mGPUObjectsCreatedStr = HEString(u8"__ProfOvObjsCreated", u8"Objects created: {0}");
		mGPUObjectsDestroyedStr = HEString(u8"__ProfOvObjsDestroyed", u8"Objects destroyed: {0}");
		mGPUResourceWritesStr = HEString(u8"__ProfOvResWrites", u8"Resource writes: {0}");
		mGPUResourceReadsStr = HEString(u8"__ProfOvResReads", u8"Resource reads: {0}");
		mGPUParamBindsStr = HEString(u8"__ProfOvGpuParamBinds", u8"GPU parameter binds: {0}");
		mGPUVertexBufferBindsStr = HEString(u8"__ProfOvVBBinds", u8"VB binds: {0}");
		mGPUIndexBufferBindsStr = HEString(u8"__ProfOvIBBinds", u8"IB binds: {0}");

		mGPUFrameNumLbl = GUILabel::create(mGPUFrameNumStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUTimeLbl = GUILabel::create(mGPUTimeStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUDrawCallsLbl = GUILabel::create(mGPUDrawCallsStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPURenTargetChangesLbl = GUILabel::create(mGPURenTargetChangesStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUPresentsLbl = GUILabel::create(mGPUPresentsStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUClearsLbl = GUILabel::create(mGPUClearsStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUVerticesLbl = GUILabel::create(mGPUVerticesStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUPrimitivesLbl = GUILabel::create(mGPUPrimitivesStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUSamplesLbl = GUILabel::create(mGPUSamplesStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUPipelineStateChangesLbl = GUILabel::create(mGPUPipelineStateChangesStr, GUIOptions(GUIOption::fixedWidth(200)));

		mGPUObjectsCreatedLbl = GUILabel::create(mGPUObjectsCreatedStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUObjectsDestroyedLbl = GUILabel::create(mGPUObjectsDestroyedStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUResourceWritesLbl = GUILabel::create(mGPUResourceWritesStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUResourceReadsLbl = GUILabel::create(mGPUResourceReadsStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUParamBindsLbl = GUILabel::create(mGPUParamBindsStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUVertexBufferBindsLbl = GUILabel::create(mGPUVertexBufferBindsStr, GUIOptions(GUIOption::fixedWidth(200)));
		mGPUIndexBufferBindsLbl = GUILabel::create(mGPUIndexBufferBindsStr, GUIOptions(GUIOption::fixedWidth(200)));

		mGPULayoutFrameContentsLeft->addElement(mGPUFrameNumLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUTimeLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUDrawCallsLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPURenTargetChangesLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUPresentsLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUClearsLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUVerticesLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUPrimitivesLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUSamplesLbl);
		mGPULayoutFrameContentsLeft->addElement(mGPUPipelineStateChangesLbl);
		mGPULayoutFrameContentsLeft->addNewElement<GUIFlexibleSpace>();

		mGPULayoutFrameContentsRight->addElement(mGPUObjectsCreatedLbl);
		mGPULayoutFrameContentsRight->addElement(mGPUObjectsDestroyedLbl);
		mGPULayoutFrameContentsRight->addElement(mGPUResourceWritesLbl);
		mGPULayoutFrameContentsRight->addElement(mGPUResourceReadsLbl);
		mGPULayoutFrameContentsRight->addElement(mGPUParamBindsLbl);
		mGPULayoutFrameContentsRight->addElement(mGPUVertexBufferBindsLbl);
		mGPULayoutFrameContentsRight->addElement(mGPUIndexBufferBindsLbl);
		mGPULayoutFrameContentsRight->addNewElement<GUIFlexibleSpace>();

		updateCPUSampleAreaSizes();
		updateGPUSampleAreaSizes();

		if (!mIsShown)
			hide();
		else
		{
			if (mType == ProfilerOverlayType::CPUSamples)
				show(ProfilerOverlayType::CPUSamples);
			else
				show(ProfilerOverlayType::GPUSamples);
		}
	}

	void ProfilerOverlay::show(ProfilerOverlayType type)
	{
		if (type == ProfilerOverlayType::CPUSamples)
		{
			mBasicLayoutLabels->setVisible(true);
			mPreciseLayoutLabels->setVisible(true);
			mBasicLayoutContents->setVisible(true);
			mPreciseLayoutContents->setVisible(true);
			mGPULayoutFrameContents->setVisible(false);
			mGPULayoutSamples->setVisible(false);
		}
		else
		{
			mGPULayoutFrameContents->setVisible(true);
			mGPULayoutSamples->setVisible(true);
			mBasicLayoutLabels->setVisible(false);
			mPreciseLayoutLabels->setVisible(false);
			mBasicLayoutContents->setVisible(false);
			mPreciseLayoutContents->setVisible(false);
		}

		mType = type;
		mIsShown = true;
	}

	void ProfilerOverlay::hide()
	{
		mBasicLayoutLabels->setVisible(false);
		mPreciseLayoutLabels->setVisible(false);
		mBasicLayoutContents->setVisible(false);
		mPreciseLayoutContents->setVisible(false);
		mGPULayoutFrameContents->setVisible(false);
		mGPULayoutSamples->setVisible(false);
		mIsShown = false;
	}

	void ProfilerOverlay::update()
	{
		const ProfilerReport& latestSimReport = ProfilingManager::instance().getReport(ProfiledThread::Sim);
		const ProfilerReport& latestCoreReport = ProfilingManager::instance().getReport(ProfiledThread::Core);

		updateCPUSampleContents(latestSimReport, latestCoreReport);

		while (ProfilerGPU::instance().getNumAvailableReports() > 1)
			ProfilerGPU::instance().getNextReport(); // Drop any extra reports, we only want the latest

		if (ProfilerGPU::instance().getNumAvailableReports() > 0)
		{
			updateGPUSampleContents(ProfilerGPU::instance().getNextReport());
		}
	}

	void ProfilerOverlay::targetResized()
	{
		updateCPUSampleAreaSizes();
		updateGPUSampleAreaSizes();
	}

	void ProfilerOverlay::updateCPUSampleAreaSizes()
	{
		static const INT32 PADDING = 10;
		static const float LABELS_CONTENT_RATIO = 0.3f;

		UINT32 width = (UINT32)std::max(0, (INT32)mTarget->getPixelArea().width - PADDING * 2);
		UINT32 height = (UINT32)std::max(0, (INT32)(mTarget->getPixelArea().height - PADDING * 3));

		UINT32 labelsWidth = Math::ceilToInt(width * LABELS_CONTENT_RATIO);
		UINT32 contentWidth = width - labelsWidth;

		mBasicLayoutLabels->setPosition(PADDING, PADDING);
		mBasicLayoutLabels->setWidth(labelsWidth);
		mBasicLayoutLabels->setHeight(height);

		mPreciseLayoutLabels->setPosition(PADDING, height + PADDING * 2);
		mPreciseLayoutLabels->setWidth(labelsWidth);
		mPreciseLayoutLabels->setHeight(height);

		mBasicLayoutContents->setPosition(PADDING + labelsWidth, PADDING);
		mBasicLayoutContents->setWidth(contentWidth);
		mBasicLayoutContents->setHeight(height);

		mPreciseLayoutContents->setPosition(PADDING + labelsWidth, height + PADDING * 2);
		mPreciseLayoutContents->setWidth(contentWidth);
		mPreciseLayoutContents->setHeight(height);
	}

	void ProfilerOverlay::updateGPUSampleAreaSizes()
	{
		static const INT32 PADDING = 10;
		static const float SAMPLES_FRAME_RATIO = 0.25f;
		static const INT32 HEADER_HEIGHT = 20;
		static const INT32 NUM_COLUMNS = 3;
		static const INT32 HEIGHT_PER_ENTRY = 15;

		UINT32 width = (UINT32)std::max(0, (INT32)mTarget->getPixelArea().width - PADDING * 2);
		UINT32 height = (UINT32)std::max(0, (INT32)(mTarget->getPixelArea().height - PADDING * 3));

		UINT32 frameHeight = Math::ceilToInt(height * SAMPLES_FRAME_RATIO);
		UINT32 samplesHeight = height - frameHeight;

		mGPULayoutFrameContents->setPosition(PADDING, PADDING);
		mGPULayoutFrameContents->setWidth(width);
		mGPULayoutFrameContents->setHeight(frameHeight);

		mGPULayoutSamples->setPosition(PADDING, PADDING + frameHeight + PADDING);
		mGPULayoutSamples->setWidth(width);
		mGPULayoutSamples->setHeight(samplesHeight);

		UINT32 columnWidth = width / NUM_COLUMNS;
		UINT32 columnHeight = samplesHeight - HEADER_HEIGHT;
		for(UINT32 i = 0; i < NUM_COLUMNS; i++)
		{
			mGPULayoutSampleLabels[i]->setPosition(columnWidth * i, HEADER_HEIGHT);
			mGPULayoutSampleLabels[i]->setWidth(columnWidth / 2);
			mGPULayoutSampleLabels[i]->setHeight(columnHeight);

			mGPULayoutSampleContents[i]->setPosition(columnWidth * i + columnWidth / 2, HEADER_HEIGHT);
			mGPULayoutSampleContents[i]->setWidth(columnWidth / 2);
			mGPULayoutSampleContents[i]->setHeight(columnHeight);
		}

		mNumGPUSamplesPerColumn = columnHeight / HEIGHT_PER_ENTRY;
	}

	void ProfilerOverlay::updateCPUSampleContents(const ProfilerReport& simReport, const ProfilerReport& coreReport)
	{
		static const UINT32 NUM_ROOT_ENTRIES = 2;

		const CPUProfilerBasicSamplingEntry& simBasicRootEntry = simReport.cpuReport.getBasicSamplingData();
		const CPUProfilerPreciseSamplingEntry& simPreciseRootEntry = simReport.cpuReport.getPreciseSamplingData();

		const CPUProfilerBasicSamplingEntry& coreBasicRootEntry = coreReport.cpuReport.getBasicSamplingData();
		const CPUProfilerPreciseSamplingEntry& corePreciseRootEntry = coreReport.cpuReport.getPreciseSamplingData();

		struct TodoBasic
		{
			TodoBasic(const CPUProfilerBasicSamplingEntry& _entry, UINT32 _depth)
				:entry(_entry), depth(_depth)
			{ }

			const CPUProfilerBasicSamplingEntry& entry;
			UINT32 depth;
		};

		struct TodoPrecise
		{
			TodoPrecise(const CPUProfilerPreciseSamplingEntry& _entry, UINT32 _depth)
				:entry(_entry), depth(_depth)
			{ }

			const CPUProfilerPreciseSamplingEntry& entry;
			UINT32 depth;
		};

		BasicRowFiller basicRowFiller(mBasicRows, *mBasicLayoutLabels, *mBasicLayoutContents, *mWidget->_getInternal());
		Stack<TodoBasic> todoBasic;

		const CPUProfilerBasicSamplingEntry* basicRootEntries[NUM_ROOT_ENTRIES];
		basicRootEntries[0] = &simBasicRootEntry;
		basicRootEntries[1] = &coreBasicRootEntry;

		for(UINT32 i = 0; i < NUM_ROOT_ENTRIES; i++)
		{
			todoBasic.push(TodoBasic(*basicRootEntries[i], 0));

			while(!todoBasic.empty())
			{
				TodoBasic curEntry = todoBasic.top();
				todoBasic.pop();

				const CPUProfilerBasicSamplingEntry::Data& data = curEntry.entry.data;
				basicRowFiller.addData(curEntry.depth, data.name, data.pctOfParent, data.numCalls, data.memAllocs, data.memFrees, 
					data.avgTimeMs, data.totalTimeMs, data.avgSelfTimeMs, data.totalSelfTimeMs);

				if(curEntry.depth <= MAX_DEPTH)
				{
					for(auto iter = curEntry.entry.childEntries.rbegin(); iter != curEntry.entry.childEntries.rend(); ++iter)
					{
						todoBasic.push(TodoBasic(*iter, curEntry.depth + 1));
					}
				}
			}
		}

		PreciseRowFiller preciseRowFiller(mPreciseRows, *mBasicLayoutLabels, *mBasicLayoutContents, *mWidget->_getInternal());
		Stack<TodoPrecise> todoPrecise;

		const CPUProfilerPreciseSamplingEntry* preciseRootEntries[NUM_ROOT_ENTRIES];
		preciseRootEntries[0] = &simPreciseRootEntry;
		preciseRootEntries[1] = &corePreciseRootEntry;

		for(UINT32 i = 0; i < NUM_ROOT_ENTRIES; i++)
		{
			todoPrecise.push(TodoPrecise(*preciseRootEntries[i], 0));

			while(!todoBasic.empty())
			{
				TodoPrecise curEntry = todoPrecise.top();
				todoPrecise.pop();

				const CPUProfilerPreciseSamplingEntry::Data& data = curEntry.entry.data;
				preciseRowFiller.addData(curEntry.depth, data.name, data.pctOfParent, data.numCalls, data.memAllocs, data.memFrees, 
					data.avgCycles, data.totalCycles, data.avgSelfCycles, data.totalSelfCycles);

				if(curEntry.depth <= MAX_DEPTH)
				{
					for(auto iter = curEntry.entry.childEntries.rbegin(); iter != curEntry.entry.childEntries.rend(); ++iter)
					{
						todoPrecise.push(TodoPrecise(*iter, curEntry.depth + 1));
					}
				}
			}
		}
	}

	void ProfilerOverlay::updateGPUSampleContents(const GPUProfilerReport& gpuReport)
	{
		mGPUFrameNumStr.setParameter(0, toString((UINT64)gTime().getFrameIdx()));
		mGPUTimeStr.setParameter(0, toString(gpuReport.frameSample.timeMs));
		mGPUDrawCallsStr.setParameter(0, toString(gpuReport.frameSample.numDrawCalls));
		mGPURenTargetChangesStr.setParameter(0, toString(gpuReport.frameSample.numRenderTargetChanges));
		mGPUPresentsStr.setParameter(0, toString(gpuReport.frameSample.numPresents));
		mGPUClearsStr.setParameter(0, toString(gpuReport.frameSample.numClears));
		mGPUVerticesStr.setParameter(0, toString(gpuReport.frameSample.numVertices));
		mGPUPrimitivesStr.setParameter(0, toString(gpuReport.frameSample.numPrimitives));
		mGPUSamplesStr.setParameter(0, toString(gpuReport.frameSample.numDrawnSamples));
		mGPUPipelineStateChangesStr.setParameter(0, toString(gpuReport.frameSample.numPipelineStateChanges));

		mGPUObjectsCreatedStr.setParameter(0, toString(gpuReport.frameSample.numObjectsCreated));
		mGPUObjectsDestroyedStr.setParameter(0, toString(gpuReport.frameSample.numObjectsDestroyed));
		mGPUResourceWritesStr.setParameter(0, toString(gpuReport.frameSample.numResourceWrites));
		mGPUResourceReadsStr.setParameter(0, toString(gpuReport.frameSample.numResourceReads));
		mGPUParamBindsStr.setParameter(0, toString(gpuReport.frameSample.numGpuParamBinds));
		mGPUVertexBufferBindsStr.setParameter(0, toString(gpuReport.frameSample.numVertexBufferBinds));
		mGPUIndexBufferBindsStr.setParameter(0, toString(gpuReport.frameSample.numIndexBufferBinds));

		mGPUFrameNumLbl->setContent(mGPUFrameNumStr);
		mGPUTimeLbl->setContent(mGPUTimeStr);
		mGPUDrawCallsLbl->setContent(mGPUDrawCallsStr);
		mGPURenTargetChangesLbl->setContent(mGPURenTargetChangesStr);
		mGPUPresentsLbl->setContent(mGPUPresentsStr);
		mGPUClearsLbl->setContent(mGPUClearsStr);
		mGPUVerticesLbl->setContent(mGPUVerticesStr);
		mGPUPrimitivesLbl->setContent(mGPUPrimitivesStr);
		mGPUSamplesLbl->setContent(mGPUSamplesStr);
		mGPUPipelineStateChangesLbl->setContent(mGPUPipelineStateChangesStr);

		mGPUObjectsCreatedLbl->setContent(mGPUObjectsCreatedStr);
		mGPUObjectsDestroyedLbl->setContent(mGPUObjectsDestroyedStr);
		mGPUResourceWritesLbl->setContent(mGPUResourceWritesStr);
		mGPUResourceReadsLbl->setContent(mGPUResourceReadsStr);
		mGPUParamBindsLbl->setContent(mGPUParamBindsStr);
		mGPUVertexBufferBindsLbl->setContent(mGPUVertexBufferBindsStr);
		mGPUIndexBufferBindsLbl->setContent(mGPUIndexBufferBindsStr);

		GPUSampleRowFiller sampleRowFillers[GPU_NUM_SAMPLE_COLUMNS] = 
		{
			GPUSampleRowFiller(mGPUSampleRows[0], *mGPULayoutSampleLabels[0], *mGPULayoutSampleContents[0], *mWidget->_getInternal()),
			GPUSampleRowFiller(mGPUSampleRows[1], *mGPULayoutSampleLabels[1], *mGPULayoutSampleContents[1], *mWidget->_getInternal()),
			GPUSampleRowFiller(mGPUSampleRows[2], *mGPULayoutSampleLabels[2], *mGPULayoutSampleContents[2], *mWidget->_getInternal())
		};

		struct Todo
		{
			Todo(const GPUProfileSample& entry, UINT32 depth)
				:entry(entry), depth(depth)
			{ }

			const GPUProfileSample& entry;
			UINT32 depth;
		};

		UINT32 column = 0;
		UINT32 currentCount = 0;

		Stack<Todo> todo;
		todo.push(Todo(gpuReport.frameSample, 0));

		while (!todo.empty())
		{
			Todo curEntry = todo.top();
			todo.pop();

			const GPUProfileSample& data = curEntry.entry;

			if(column < GPU_NUM_SAMPLE_COLUMNS)
				sampleRowFillers[column].addData(curEntry.depth, data.name, data.timeMs);

			currentCount++;
			if (currentCount % mNumGPUSamplesPerColumn == 0)
				column++;

			if (curEntry.depth <= MAX_DEPTH)
			{
				for (auto iter = curEntry.entry.children.rbegin(); iter != curEntry.entry.children.rend(); ++iter)
					todo.push(Todo(*iter, curEntry.depth + 1));
			}
		}
	}
}