//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsProfilerOverlay.h"
#include "BsSceneObject.h"
#include "BsCGUIWidget.h"
#include "BsGUILayout.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUIPanel.h"
#include "BsGUIElement.h"
#include "BsGUILabel.h"
#include "BsGUISpace.h"
#include "BsViewport.h"
#include "BsTime.h"
#include "BsBuiltinResources.h"
#include "BsProfilingManager.h"
#include "BsRenderTarget.h"
#include "BsProfilerOverlayRTTI.h"
#include "BsCamera.h"
#include <BsHEString.h>

#define BS_SHOW_PRECISE_PROFILING 0

namespace bs
{
	class BasicRowFiller
	{
	public:
		UINT32 curIdx;
		GUILayout& labelLayout;
		GUILayout& contentLayout;
		GUIWidget& widget;
		Vector<ProfilerOverlayInternal::BasicRow>& rows;

		BasicRowFiller(Vector<ProfilerOverlayInternal::BasicRow>& _rows, GUILayout& _labelLayout, GUILayout& _contentLayout, GUIWidget& _widget)
			:curIdx(0), labelLayout(_labelLayout), contentLayout(_contentLayout), widget(_widget), rows(_rows)
		{ }

		~BasicRowFiller()
		{
			UINT32 excessEntries = (UINT32)rows.size() - curIdx;
			for(UINT32 i = 0; i < excessEntries; i++)
			{
				ProfilerOverlayInternal::BasicRow& row = rows[curIdx + i];

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
				rows.push_back(ProfilerOverlayInternal::BasicRow());

				ProfilerOverlayInternal::BasicRow& newRow = rows.back();

				newRow.disabled = false;
				newRow.name = HEString(L"{0}");
				newRow.pctOfParent = HEString(L"{0} %");
				newRow.numCalls = HEString(L"{0}");
				newRow.numAllocs = HEString(L"{0}");
				newRow.numFrees = HEString(L"{0}");
				newRow.avgTime = HEString(L"{0}");
				newRow.totalTime = HEString(L"{0}");
				newRow.avgTimeSelf = HEString(L"{0}");
				newRow.totalTimeSelf = HEString(L"{0}");

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
			
			ProfilerOverlayInternal::BasicRow& row = rows[curIdx];

			row.labelSpace->setSize(depth * 20);
			row.name.setParameter(0, toWString(name));
			row.pctOfParent.setParameter(0, toWString(pctOfParent * 100.0f, 2, 0, ' ', std::ios::fixed));
			row.numCalls.setParameter(0, toWString(numCalls));
			row.numAllocs.setParameter(0, toWString(numAllocs));
			row.numFrees.setParameter(0, toWString(numFrees));
			row.avgTime.setParameter(0, toWString(avgTime, 2, 0, ' ', std::ios::fixed));
			row.totalTime.setParameter(0, toWString(totalTime, 2, 0, ' ', std::ios::fixed));
			row.avgTimeSelf.setParameter(0, toWString(avgSelfTime, 2, 0, ' ', std::ios::fixed));
			row.totalTimeSelf.setParameter(0, toWString(totalSelfTime, 2, 0, ' ', std::ios::fixed));

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
		Vector<ProfilerOverlayInternal::PreciseRow>& rows;

		PreciseRowFiller(Vector<ProfilerOverlayInternal::PreciseRow>& _rows, GUILayout& _labelLayout, GUILayout& _contentLayout, GUIWidget& _widget)
			:curIdx(0), labelLayout(_labelLayout), contentLayout(_contentLayout), widget(_widget), rows(_rows)
		{ }

		~PreciseRowFiller()
		{
			UINT32 excessEntries = (UINT32)rows.size() - curIdx;
			for(UINT32 i = 0; i < excessEntries; i++)
			{
				ProfilerOverlayInternal::PreciseRow& row = rows[curIdx + i];

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
				rows.push_back(ProfilerOverlayInternal::PreciseRow());

				ProfilerOverlayInternal::PreciseRow& newRow = rows.back();

				newRow.disabled = false;
				newRow.name = HEString(L"{0}");
				newRow.pctOfParent = HEString(L"{0}");
				newRow.numCalls = HEString(L"{0}");
				newRow.numAllocs = HEString(L"{0}");
				newRow.numFrees = HEString(L"{0}");
				newRow.avgCycles = HEString(L"{0}");
				newRow.totalCycles = HEString(L"{0}");
				newRow.avgCyclesSelf = HEString(L"{0}");
				newRow.totalCyclesSelf = HEString(L"{0}");

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

			ProfilerOverlayInternal::PreciseRow& row = rows[curIdx];

			row.labelSpace->setSize(depth * 20);
			row.name.setParameter(0, toWString(name));
			row.pctOfParent.setParameter(0, toWString(pctOfParent * 100.0f, 2, 0, ' ', std::ios::fixed));
			row.numCalls.setParameter(0, toWString(numCalls));
			row.numAllocs.setParameter(0, toWString(numAllocs));
			row.numFrees.setParameter(0, toWString(numFrees));
			row.avgCycles.setParameter(0, toWString(avgCycles));
			row.totalCycles.setParameter(0, toWString(totalCycles));
			row.avgCyclesSelf.setParameter(0, toWString(avgSelfCycles));
			row.totalCyclesSelf.setParameter(0, toWString(totalSelfCycles));

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
		GUILayout& layout;
		GUIWidget& widget;
		Vector<ProfilerOverlayInternal::GPUSampleRow>& rows;

		GPUSampleRowFiller(Vector<ProfilerOverlayInternal::GPUSampleRow>& _rows, GUILayout& _layout, GUIWidget& _widget)
			:curIdx(0), layout(_layout), widget(_widget), rows(_rows)
		{ }

		~GPUSampleRowFiller()
		{
			UINT32 excessEntries = (UINT32)rows.size() - curIdx;
			for (UINT32 i = 0; i < excessEntries; i++)
			{
				ProfilerOverlayInternal::GPUSampleRow& row = rows[curIdx + i];

				if (!row.disabled)
				{
					row.layout->setVisible(false);
					row.disabled = true;
				}
			}

			rows.resize(curIdx);
		}

		void addData(const String& name, float timeMs)
		{
			if (curIdx >= rows.size())
			{
				rows.push_back(ProfilerOverlayInternal::GPUSampleRow());

				ProfilerOverlayInternal::GPUSampleRow& newRow = rows.back();

				newRow.disabled = false;
				newRow.name = HEString(L"{1}");
				newRow.time = HEString(L"{0}");

				newRow.layout = layout.insertNewElement<GUILayoutX>(layout.getNumChildren());

				newRow.guiName = newRow.layout->addNewElement<GUILabel>(newRow.name, GUIOptions(GUIOption::fixedWidth(100)));
				newRow.guiTime = newRow.layout->addNewElement<GUILabel>(newRow.time, GUIOptions(GUIOption::fixedWidth(100)));
			}

			ProfilerOverlayInternal::GPUSampleRow& row = rows[curIdx];
			row.name.setParameter(0, toWString(name));
			row.time.setParameter(0, toWString(timeMs));

			row.guiName->setContent(row.name);
			row.guiTime->setContent(row.time);

			if (row.disabled)
			{
				row.layout->setVisible(true);
				row.disabled = false;
			}

			curIdx++;
		}
	};

	const UINT32 ProfilerOverlayInternal::MAX_DEPTH = 4;

	ProfilerOverlay::ProfilerOverlay(const HSceneObject& parent, const SPtr<Camera>& target)
		:Component(parent), mInternal(nullptr)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		mInternal = bs_new<ProfilerOverlayInternal>(target);
	}

	ProfilerOverlay::~ProfilerOverlay()
	{
		bs_delete(mInternal);
	}

	void ProfilerOverlay::setTarget(const SPtr<Camera>& target)
	{
		mInternal->setTarget(target);
	}

	void ProfilerOverlay::show(ProfilerOverlayType type)
	{
		mInternal->show(type);
	}

	void ProfilerOverlay::hide()
	{
		mInternal->hide();
	}

	void ProfilerOverlay::update()
	{
		mInternal->update();
	}

	RTTITypeBase* ProfilerOverlay::getRTTIStatic()
	{
		return ProfilerOverlayRTTI::instance();
	}

	RTTITypeBase* ProfilerOverlay::getRTTI() const
	{
		return ProfilerOverlay::getRTTIStatic();
	}

	ProfilerOverlayInternal::ProfilerOverlayInternal(const SPtr<Camera>& camera)
		:mType(ProfilerOverlayType::CPUSamples), mIsShown(true)
	{
		setTarget(camera);
	}

	ProfilerOverlayInternal::~ProfilerOverlayInternal()
	{
		if(mTarget != nullptr)
			mTargetResizedConn.disconnect();

		if(mWidgetSO)
			mWidgetSO->destroy();
	}

	void ProfilerOverlayInternal::setTarget(const SPtr<Camera>& camera)
	{
		if(mTarget != nullptr)
			mTargetResizedConn.disconnect();

		mTarget = camera->getViewport();

		mTargetResizedConn = mTarget->getTarget()->onResized.connect(std::bind(&ProfilerOverlayInternal::targetResized, this));

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
		mTitleBasicName = GUILabel::create(HEString(L"Name"), GUIOptions(GUIOption::fixedWidth(200)));
		mTitleBasicPctOfParent = GUILabel::create(HEString(L"% parent"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicNumCalls = GUILabel::create(HEString(L"# calls"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicNumAllocs = GUILabel::create(HEString(L"# allocs"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicNumFrees = GUILabel::create(HEString(L"# frees"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitleBasicAvgTime = GUILabel::create(HEString(L"Avg. time"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitleBasicTotalTime = GUILabel::create(HEString(L"Total time"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitleBasicAvgTitleSelf = GUILabel::create(HEString(L"Avg. self time"), GUIOptions(GUIOption::fixedWidth(100)));
		mTitleBasicTotalTimeSelf = GUILabel::create(HEString(L"Total self time"), GUIOptions(GUIOption::fixedWidth(100)));

		mTitlePreciseName = GUILabel::create(HEString(L"Name"), GUIOptions(GUIOption::fixedWidth(200)));
		mTitlePrecisePctOfParent = GUILabel::create(HEString(L"% parent"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseNumCalls = GUILabel::create(HEString(L"# calls"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseNumAllocs = GUILabel::create(HEString(L"# allocs"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseNumFrees = GUILabel::create(HEString(L"# frees"), GUIOptions(GUIOption::fixedWidth(50)));
		mTitlePreciseAvgCycles = GUILabel::create(HEString(L"Avg. cycles"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitlePreciseTotalCycles = GUILabel::create(HEString(L"Total cycles"), GUIOptions(GUIOption::fixedWidth(60)));
		mTitlePreciseAvgCyclesSelf = GUILabel::create(HEString(L"Avg. self cycles"), GUIOptions(GUIOption::fixedWidth(100)));
		mTitlePreciseTotalCyclesSelf = GUILabel::create(HEString(L"Total self cycles"), GUIOptions(GUIOption::fixedWidth(100)));

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

		mGPULayoutSamples = mWidget->getPanel()->addNewElement<GUILayoutY>();

		GUILayout* gpuSampleTitle = mGPULayoutSamples->addNewElement<GUILayoutY>();
		mGPULayoutSampleContents = mGPULayoutSamples->addNewElement<GUILayoutY>();
		mGPULayoutSamples->addNewElement<GUIFlexibleSpace>();

		HString gpuSamplesStr(L"__ProfOvGPUSamples", L"Samples");
		gpuSampleTitle->addElement(GUILabel::create(gpuSamplesStr));
		gpuSampleTitle->addNewElement<GUIFixedSpace>(20);

		GUILayout* gpuSampleTitleRow = gpuSampleTitle->addNewElement<GUILayoutX>();

		HString gpuSamplesNameStr(L"__ProfOvGPUSampName", L"Name");
		HString gpuSamplesTimeStr(L"__ProfOvGPUSampTime", L"Time");
		gpuSampleTitleRow->addElement(GUILabel::create(gpuSamplesNameStr, GUIOptions(GUIOption::fixedWidth(100))));
		gpuSampleTitleRow->addElement(GUILabel::create(gpuSamplesTimeStr, GUIOptions(GUIOption::fixedWidth(100))));

		mGPUFrameNumStr = HEString(L"__ProfOvFrame", L"Frame #{0}");
		mGPUTimeStr = HEString(L"__ProfOvTime", L"Time: {0}ms");
		mGPUDrawCallsStr = HEString(L"__ProfOvDrawCalls", L"Draw calls: {0}");
		mGPURenTargetChangesStr = HEString(L"__ProfOvRTChanges", L"Render target changes: {0}");
		mGPUPresentsStr = HEString(L"__ProfOvPresents", L"Presents: {0}");
		mGPUClearsStr = HEString(L"__ProfOvClears", L"Clears: {0}");
		mGPUVerticesStr = HEString(L"__ProfOvVertices", L"Num. vertices: {0}");
		mGPUPrimitivesStr = HEString(L"__ProfOvPrimitives", L"Num. primitives: {0}");
		mGPUSamplesStr = HEString(L"__ProfOvSamples", L"Samples drawn: {0}");
		mGPUPipelineStateChangesStr = HEString(L"__ProfOvPSChanges", L"Pipeline state changes: {0}");

		mGPUObjectsCreatedStr = HEString(L"__ProfOvObjsCreated", L"Objects created: {0}");
		mGPUObjectsDestroyedStr = HEString(L"__ProfOvObjsDestroyed", L"Objects destroyed: {0}");
		mGPUResourceWritesStr = HEString(L"__ProfOvResWrites", L"Resource writes: {0}");
		mGPUResourceReadsStr = HEString(L"__ProfOvResReads", L"Resource reads: {0}");
		mGPUParamBindsStr = HEString(L"__ProfOvGpuParamBinds", L"GPU parameter binds: {0}");
		mGPUVertexBufferBindsStr = HEString(L"__ProfOvVBBinds", L"VB binds: {0}");
		mGPUIndexBufferBindsStr = HEString(L"__ProfOvIBBinds", L"IB binds: {0}");

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

	void ProfilerOverlayInternal::show(ProfilerOverlayType type)
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

	void ProfilerOverlayInternal::hide()
	{
		mBasicLayoutLabels->setVisible(false);
		mPreciseLayoutLabels->setVisible(false);
		mBasicLayoutContents->setVisible(false);
		mPreciseLayoutContents->setVisible(false);
		mGPULayoutFrameContents->setVisible(false);
		mGPULayoutSamples->setVisible(false);
		mIsShown = false;
	}

	void ProfilerOverlayInternal::update()
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

	void ProfilerOverlayInternal::targetResized()
	{
		updateCPUSampleAreaSizes();
		updateGPUSampleAreaSizes();
	}

	void ProfilerOverlayInternal::updateCPUSampleAreaSizes()
	{
		static const INT32 PADDING = 10;
		static const float LABELS_CONTENT_RATIO = 0.3f;

		UINT32 width = (UINT32)std::max(0, (INT32)mTarget->getWidth() - PADDING * 2);
		UINT32 height = (UINT32)std::max(0, (INT32)(mTarget->getHeight() - PADDING * 3)/2);

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

	void ProfilerOverlayInternal::updateGPUSampleAreaSizes()
	{
		static const INT32 PADDING = 10;
		static const float SAMPLES_FRAME_RATIO = 0.5f;

		UINT32 width = (UINT32)std::max(0, (INT32)mTarget->getWidth() - PADDING * 2);
		UINT32 height = (UINT32)std::max(0, (INT32)(mTarget->getHeight() - PADDING * 3));

		UINT32 frameHeight = Math::ceilToInt(height * SAMPLES_FRAME_RATIO);
		UINT32 samplesHeight = height - frameHeight;

		mGPULayoutFrameContents->setPosition(PADDING, PADDING);
		mGPULayoutFrameContents->setWidth(width);
		mGPULayoutFrameContents->setHeight(frameHeight);

		mGPULayoutSamples->setPosition(PADDING, PADDING + frameHeight + PADDING);
		mGPULayoutSamples->setWidth(width);
		mGPULayoutSamples->setHeight(samplesHeight);
	}

	void ProfilerOverlayInternal::updateCPUSampleContents(const ProfilerReport& simReport, const ProfilerReport& coreReport)
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

	void ProfilerOverlayInternal::updateGPUSampleContents(const GPUProfilerReport& gpuReport)
	{
		mGPUFrameNumStr.setParameter(0, toWString((UINT64)gTime().getFrameIdx()));
		mGPUTimeStr.setParameter(0, toWString(gpuReport.frameSample.timeMs));
		mGPUDrawCallsStr.setParameter(0, toWString(gpuReport.frameSample.numDrawCalls));
		mGPURenTargetChangesStr.setParameter(0, toWString(gpuReport.frameSample.numRenderTargetChanges));
		mGPUPresentsStr.setParameter(0, toWString(gpuReport.frameSample.numPresents));
		mGPUClearsStr.setParameter(0, toWString(gpuReport.frameSample.numClears));
		mGPUVerticesStr.setParameter(0, toWString(gpuReport.frameSample.numVertices));
		mGPUPrimitivesStr.setParameter(0, toWString(gpuReport.frameSample.numPrimitives));
		mGPUSamplesStr.setParameter(0, toWString(gpuReport.frameSample.numDrawnSamples));
		mGPUPipelineStateChangesStr.setParameter(0, toWString(gpuReport.frameSample.numPipelineStateChanges));

		mGPUObjectsCreatedStr.setParameter(0, toWString(gpuReport.frameSample.numObjectsCreated));
		mGPUObjectsDestroyedStr.setParameter(0, toWString(gpuReport.frameSample.numObjectsDestroyed));
		mGPUResourceWritesStr.setParameter(0, toWString(gpuReport.frameSample.numResourceWrites));
		mGPUResourceReadsStr.setParameter(0, toWString(gpuReport.frameSample.numResourceReads));
		mGPUParamBindsStr.setParameter(0, toWString(gpuReport.frameSample.numGpuParamBinds));
		mGPUVertexBufferBindsStr.setParameter(0, toWString(gpuReport.frameSample.numVertexBufferBinds));
		mGPUIndexBufferBindsStr.setParameter(0, toWString(gpuReport.frameSample.numIndexBufferBinds));

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

		GPUSampleRowFiller sampleRowFiller(mGPUSampleRows, *mGPULayoutSampleContents, *mWidget->_getInternal());
		for (auto& sample : gpuReport.samples)
		{
			sampleRowFiller.addData(sample.name, sample.timeMs);
		}
	}
}