//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DProfilerOverlay.h"

#include "B3DGUIUtility.h"
#include "Scene/B3DSceneObject.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUILayout.h"
#include "GUI/B3DGUILayoutX.h"
#include "GUI/B3DGUILayoutY.h"
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUILabel.h"
#include "GUI/B3DGUISpace.h"
#include "RenderAPI/B3DViewport.h"
#include "Utility/B3DTime.h"
#include "Resources/B3DBuiltinResources.h"
#include "Profiling/B3DProfilingManager.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "Components/B3DCamera.h"
#include "Localization/B3DHEString.h"

#define BS_SHOW_PRECISE_PROFILING 0

using namespace b3d;

constexpr u32 kMaxDepth = 4;

class BasicRowFiller
{
public:
	u32 CurIdx;
	GUILayout& LabelLayout;
	GUILayout& ContentLayout;
	GUIWidget& Widget;
	Vector<ProfilerOverlay::BasicRow>& Rows;

	BasicRowFiller(Vector<ProfilerOverlay::BasicRow>& _rows, GUILayout& _labelLayout, GUILayout& _contentLayout, GUIWidget& _widget)
		: CurIdx(0), LabelLayout(_labelLayout), ContentLayout(_contentLayout), Widget(_widget), Rows(_rows)
	{}

	~BasicRowFiller()
	{
		u32 excessEntries = (u32)Rows.size() - CurIdx;
		for(u32 i = 0; i < excessEntries; i++)
		{
			ProfilerOverlay::BasicRow& row = Rows[CurIdx + i];

			if(!row.Disabled)
			{
				row.LabelLayout->SetHidden(true);
				row.ContentLayout->SetHidden(true);
				row.Disabled = true;
			}
		}

		Rows.resize(CurIdx);
	}

	void AddData(u32 depth, const String& name, float pctOfParent, u32 numCalls, u64 numAllocs, u64 numFrees, double avgTime, double totalTime, double avgSelfTime, double totalSelfTime)
	{
		if(CurIdx >= Rows.size())
		{
			Rows.push_back(ProfilerOverlay::BasicRow());

			ProfilerOverlay::BasicRow& newRow = Rows.back();

			newRow.Disabled = false;
			newRow.Name = HEString(u8"{0}");
			newRow.PctOfParent = HEString(u8"{0} %");
			newRow.NumCalls = HEString(u8"{0}");
			newRow.NumAllocs = HEString(u8"{0}");
			newRow.NumFrees = HEString(u8"{0}");
			newRow.AvgTime = HEString(u8"{0}");
			newRow.TotalTime = HEString(u8"{0}");
			newRow.AvgTimeSelf = HEString(u8"{0}");
			newRow.TotalTimeSelf = HEString(u8"{0}");

			newRow.LabelLayout = LabelLayout.InsertNewElement<GUILayoutX>(LabelLayout.GetChildCount() - 1); // Insert before flexible space
			newRow.ContentLayout = ContentLayout.InsertNewElement<GUILayoutX>(ContentLayout.GetChildCount() - 1); // Insert before flexible space

			newRow.LabelSpace = newRow.LabelLayout->AddNewElement<GUIFixedSpace>(0);
			newRow.GuiName = newRow.LabelLayout->AddNewElement<GUILabel>(newRow.Name, GUIOptions(GUIOption::FixedWidth(200)));

			newRow.GuiPctOfParent = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.PctOfParent, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiNumCalls = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.NumCalls, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiNumAllocs = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.NumAllocs, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiNumFrees = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.NumFrees, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiAvgTime = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.AvgTime, GUIOptions(GUIOption::FixedWidth(60)));
			newRow.GuiTotalTime = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.TotalTime, GUIOptions(GUIOption::FixedWidth(60)));
			newRow.GuiAvgTimeSelf = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.AvgTimeSelf, GUIOptions(GUIOption::FixedWidth(100)));
			newRow.GuiTotalTimeSelf = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.TotalTimeSelf, GUIOptions(GUIOption::FixedWidth(100)));
		}

		ProfilerOverlay::BasicRow& row = Rows[CurIdx];

		row.LabelSpace->SetSize(GUILogicalUnit(20) * (i32)depth);
		row.Name.SetParameter(0, name);
		row.PctOfParent.SetParameter(0, ToString(pctOfParent * 100.0f, 2, 0, ' ', std::ios::fixed));
		row.NumCalls.SetParameter(0, ToString(numCalls));
		row.NumAllocs.SetParameter(0, ToString(numAllocs));
		row.NumFrees.SetParameter(0, ToString(numFrees));
		row.AvgTime.SetParameter(0, ToString(avgTime, 2, 0, ' ', std::ios::fixed));
		row.TotalTime.SetParameter(0, ToString(totalTime, 2, 0, ' ', std::ios::fixed));
		row.AvgTimeSelf.SetParameter(0, ToString(avgSelfTime, 2, 0, ' ', std::ios::fixed));
		row.TotalTimeSelf.SetParameter(0, ToString(totalSelfTime, 2, 0, ' ', std::ios::fixed));

		row.GuiName->SetContent(row.Name);
		row.GuiPctOfParent->SetContent(row.PctOfParent);
		row.GuiNumCalls->SetContent(row.NumCalls);
		row.GuiNumAllocs->SetContent(row.NumAllocs);
		row.GuiNumFrees->SetContent(row.NumFrees);
		row.GuiAvgTime->SetContent(row.AvgTime);
		row.GuiTotalTime->SetContent(row.TotalTime);
		row.GuiAvgTimeSelf->SetContent(row.AvgTimeSelf);
		row.GuiTotalTimeSelf->SetContent(row.TotalTimeSelf);

		if(row.Disabled)
		{
			row.LabelLayout->SetHidden(false);
			row.ContentLayout->SetHidden(false);
			row.Disabled = false;
		}

		CurIdx++;
	}
};

class PreciseRowFiller
{
public:
	u32 CurIdx;
	GUILayout& LabelLayout;
	GUILayout& ContentLayout;
	GUIWidget& Widget;
	Vector<ProfilerOverlay::PreciseRow>& Rows;

	PreciseRowFiller(Vector<ProfilerOverlay::PreciseRow>& _rows, GUILayout& _labelLayout, GUILayout& _contentLayout, GUIWidget& _widget)
		: CurIdx(0), LabelLayout(_labelLayout), ContentLayout(_contentLayout), Widget(_widget), Rows(_rows)
	{}

	~PreciseRowFiller()
	{
		u32 excessEntries = (u32)Rows.size() - CurIdx;
		for(u32 i = 0; i < excessEntries; i++)
		{
			ProfilerOverlay::PreciseRow& row = Rows[CurIdx + i];

			if(!row.Disabled)
			{
				row.LabelLayout->SetHidden(true);
				row.ContentLayout->SetHidden(true);
				row.Disabled = true;
			}
		}

		Rows.resize(CurIdx);
	}

	void AddData(u32 depth, const String& name, float pctOfParent, u32 numCalls, u64 numAllocs, u64 numFrees, u64 avgCycles, u64 totalCycles, u64 avgSelfCycles, u64 totalSelfCycles)
	{
		if(CurIdx >= Rows.size())
		{
			Rows.push_back(ProfilerOverlay::PreciseRow());

			ProfilerOverlay::PreciseRow& newRow = Rows.back();

			newRow.Disabled = false;
			newRow.Name = HEString(u8"{0}");
			newRow.PctOfParent = HEString(u8"{0}");
			newRow.NumCalls = HEString(u8"{0}");
			newRow.NumAllocs = HEString(u8"{0}");
			newRow.NumFrees = HEString(u8"{0}");
			newRow.AvgCycles = HEString(u8"{0}");
			newRow.TotalCycles = HEString(u8"{0}");
			newRow.AvgCyclesSelf = HEString(u8"{0}");
			newRow.TotalCyclesSelf = HEString(u8"{0}");

			newRow.LabelLayout = LabelLayout.InsertNewElement<GUILayoutX>(LabelLayout.GetChildCount() - 1); // Insert before flexible space
			newRow.ContentLayout = ContentLayout.InsertNewElement<GUILayoutX>(ContentLayout.GetChildCount() - 1); // Insert before flexible space

			newRow.LabelSpace = newRow.LabelLayout->AddNewElement<GUIFixedSpace>(0);
			newRow.GuiName = newRow.LabelLayout->AddNewElement<GUILabel>(newRow.Name, GUIOptions(GUIOption::FixedWidth(200)));

			newRow.GuiPctOfParent = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.PctOfParent, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiNumCalls = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.NumCalls, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiNumAllocs = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.NumAllocs, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiNumFrees = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.NumFrees, GUIOptions(GUIOption::FixedWidth(50)));
			newRow.GuiAvgCycles = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.AvgCycles, GUIOptions(GUIOption::FixedWidth(60)));
			newRow.GuiTotalCycles = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.TotalCycles, GUIOptions(GUIOption::FixedWidth(60)));
			newRow.GuiAvgCyclesSelf = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.AvgCyclesSelf, GUIOptions(GUIOption::FixedWidth(100)));
			newRow.GuiTotalCyclesSelf = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.TotalCyclesSelf, GUIOptions(GUIOption::FixedWidth(100)));
		}

		ProfilerOverlay::PreciseRow& row = Rows[CurIdx];

		row.LabelSpace->SetSize(GUILogicalUnit(20) * (i32)depth);
		row.Name.SetParameter(0, name);
		row.PctOfParent.SetParameter(0, ToString(pctOfParent * 100.0f, 2, 0, ' ', std::ios::fixed));
		row.NumCalls.SetParameter(0, ToString(numCalls));
		row.NumAllocs.SetParameter(0, ToString(numAllocs));
		row.NumFrees.SetParameter(0, ToString(numFrees));
		row.AvgCycles.SetParameter(0, ToString(avgCycles));
		row.TotalCycles.SetParameter(0, ToString(totalCycles));
		row.AvgCyclesSelf.SetParameter(0, ToString(avgSelfCycles));
		row.TotalCyclesSelf.SetParameter(0, ToString(totalSelfCycles));

		row.GuiName->SetContent(row.Name);
		row.GuiPctOfParent->SetContent(row.PctOfParent);
		row.GuiNumCalls->SetContent(row.NumCalls);
		row.GuiNumAllocs->SetContent(row.NumAllocs);
		row.GuiNumFrees->SetContent(row.NumFrees);
		row.GuiAvgCycles->SetContent(row.AvgCycles);
		row.GuiTotalCycles->SetContent(row.TotalCycles);
		row.GuiAvgCyclesSelf->SetContent(row.AvgCyclesSelf);
		row.GuiTotalCyclesSelf->SetContent(row.TotalCyclesSelf);

		if(row.Disabled)
		{
			row.LabelLayout->SetHidden(false);
			row.ContentLayout->SetHidden(false);
			row.Disabled = false;
		}

		CurIdx++;
	}
};

class GPUSampleRowFiller
{
public:
	u32 CurIdx;
	GUILayout& LabelLayout;
	GUILayout& ContentLayout;
	GUIWidget& Widget;
	Vector<ProfilerOverlay::GPUSampleRow>& Rows;

	GPUSampleRowFiller(Vector<ProfilerOverlay::GPUSampleRow>& rows, GUILayout& labelLayout, GUILayout& contentLayout, GUIWidget& _widget)
		: CurIdx(0), LabelLayout(labelLayout), ContentLayout(contentLayout), Widget(_widget), Rows(rows)
	{}

	~GPUSampleRowFiller()
	{
		u32 excessEntries = (u32)Rows.size() - CurIdx;
		for(u32 i = 0; i < excessEntries; i++)
		{
			ProfilerOverlay::GPUSampleRow& row = Rows[CurIdx + i];

			if(!row.Disabled)
			{
				row.LabelLayout->SetHidden(true);
				row.ContentLayout->SetHidden(true);
				row.Disabled = true;
			}
		}

		Rows.resize(CurIdx);
	}

	void AddData(u32 depth, const String& name, float timeMs)
	{
		if(CurIdx >= Rows.size())
		{
			Rows.push_back(ProfilerOverlay::GPUSampleRow());

			ProfilerOverlay::GPUSampleRow& newRow = Rows.back();

			newRow.Disabled = false;
			newRow.Name = HEString(u8"{1}");
			newRow.Time = HEString(u8"{0}");

			newRow.LabelLayout = LabelLayout.InsertNewElement<GUILayoutX>(LabelLayout.GetChildCount() - 1); // Insert before flexible space
			newRow.ContentLayout = ContentLayout.InsertNewElement<GUILayoutX>(ContentLayout.GetChildCount() - 1); // Insert before flexible space

			newRow.LabelSpace = newRow.LabelLayout->AddNewElement<GUIFixedSpace>(0);
			newRow.GuiName = newRow.LabelLayout->AddNewElement<GUILabel>(newRow.Name, GUIOptions(GUIOption::FixedWidth(200)));

			newRow.GuiTime = newRow.ContentLayout->AddNewElement<GUILabel>(newRow.Time, GUIOptions(GUIOption::FixedWidth(100)));
		}

		ProfilerOverlay::GPUSampleRow& row = Rows[CurIdx];

		row.LabelSpace->SetSize(GUILogicalUnit(20) * (i32)depth);
		row.Name.SetParameter(0, name);
		row.Time.SetParameter(0, ToString(timeMs));

		row.GuiName->SetContent(row.Name);
		row.GuiTime->SetContent(row.Time);

		if(row.Disabled)
		{
			row.LabelLayout->SetHidden(true);
			row.ContentLayout->SetHidden(true);
			row.Disabled = false;
		}

		CurIdx++;
	}
};

ProfilerOverlay::ProfilerOverlay(const HCamera& camera)
	: mType(ProfilerOverlayType::CPUSamples), mIsShown(true)
{
	SetTarget(camera);
}

ProfilerOverlay::~ProfilerOverlay()
{
	if(mTarget != nullptr)
		mTargetResizedConn.Disconnect();

	if(mWidgetSO)
		mWidgetSO->Destroy();
}

void ProfilerOverlay::SetTarget(const HCamera& camera)
{
	if(mTarget != nullptr)
		mTargetResizedConn.Disconnect();

	mTarget = camera->GetViewport();

	mTargetResizedConn = mTarget->GetTarget()->OnResized.Connect(std::bind(&ProfilerOverlay::TargetResized, this));

	if(mWidgetSO)
		mWidgetSO->Destroy();

	mWidgetSO = SceneObject::Create("ProfilerOverlay", SceneObjectFlag::Internal | SceneObjectFlag::RuntimePersistent);
	mWidget = mWidgetSO->AddComponent<GUIWidget>(camera);
	mWidget->SetDepth(127);

	// Set up CPU sample areas
	mBasicLayoutLabels = mWidget->GetPanel()->AddNewElement<GUILayoutY>();
	mPreciseLayoutLabels = mWidget->GetPanel()->AddNewElement<GUILayoutY>();
	mBasicLayoutContents = mWidget->GetPanel()->AddNewElement<GUILayoutY>();
	mPreciseLayoutContents = mWidget->GetPanel()->AddNewElement<GUILayoutY>();

	// Set up CPU sample title bars
	mTitleBasicName = GUILabel::Create(GUIContent(HEString(u8"Name")), GUIOptions(GUIOption::FixedWidth(200)));
	mTitleBasicPctOfParent = GUILabel::Create(GUIContent(HEString(u8"% parent")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitleBasicNumCalls = GUILabel::Create(GUIContent(HEString(u8"# calls")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitleBasicNumAllocs = GUILabel::Create(GUIContent(HEString(u8"# allocs")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitleBasicNumFrees = GUILabel::Create(GUIContent(HEString(u8"# frees")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitleBasicAvgTime = GUILabel::Create(GUIContent(HEString(u8"Avg. time")), GUIOptions(GUIOption::FixedWidth(60)));
	mTitleBasicTotalTime = GUILabel::Create(GUIContent(HEString(u8"Total time")), GUIOptions(GUIOption::FixedWidth(60)));
	mTitleBasicAvgTitleSelf = GUILabel::Create(GUIContent(HEString(u8"Avg. self time")), GUIOptions(GUIOption::FixedWidth(100)));
	mTitleBasicTotalTimeSelf = GUILabel::Create(GUIContent(HEString(u8"Total self time")), GUIOptions(GUIOption::FixedWidth(100)));

	mTitlePreciseName = GUILabel::Create(GUIContent(HEString(u8"Name")), GUIOptions(GUIOption::FixedWidth(200)));
	mTitlePrecisePctOfParent = GUILabel::Create(GUIContent(HEString(u8"% parent")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitlePreciseNumCalls = GUILabel::Create(GUIContent(HEString(u8"# calls")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitlePreciseNumAllocs = GUILabel::Create(GUIContent(HEString(u8"# allocs")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitlePreciseNumFrees = GUILabel::Create(GUIContent(HEString(u8"# frees")), GUIOptions(GUIOption::FixedWidth(50)));
	mTitlePreciseAvgCycles = GUILabel::Create(GUIContent(HEString(u8"Avg. cycles")), GUIOptions(GUIOption::FixedWidth(60)));
	mTitlePreciseTotalCycles = GUILabel::Create(GUIContent(HEString(u8"Total cycles")), GUIOptions(GUIOption::FixedWidth(60)));
	mTitlePreciseAvgCyclesSelf = GUILabel::Create(GUIContent(HEString(u8"Avg. self cycles")), GUIOptions(GUIOption::FixedWidth(100)));
	mTitlePreciseTotalCyclesSelf = GUILabel::Create(GUIContent(HEString(u8"Total self cycles")), GUIOptions(GUIOption::FixedWidth(100)));

	GUILayout* basicTitleLabelLayout = mBasicLayoutLabels->AddNewElement<GUILayoutX>();
	GUILayout* preciseTitleLabelLayout = mPreciseLayoutLabels->AddNewElement<GUILayoutX>();
	GUILayout* basicTitleContentLayout = mBasicLayoutContents->AddNewElement<GUILayoutX>();
	GUILayout* preciseTitleContentLayout = mPreciseLayoutContents->AddNewElement<GUILayoutX>();

	basicTitleLabelLayout->AddElement(mTitleBasicName);
	basicTitleContentLayout->AddElement(mTitleBasicPctOfParent);
	basicTitleContentLayout->AddElement(mTitleBasicNumCalls);
	basicTitleContentLayout->AddElement(mTitleBasicNumAllocs);
	basicTitleContentLayout->AddElement(mTitleBasicNumFrees);
	basicTitleContentLayout->AddElement(mTitleBasicAvgTime);
	basicTitleContentLayout->AddElement(mTitleBasicTotalTime);
	basicTitleContentLayout->AddElement(mTitleBasicAvgTitleSelf);
	basicTitleContentLayout->AddElement(mTitleBasicTotalTimeSelf);

	preciseTitleLabelLayout->AddElement(mTitlePreciseName);
	preciseTitleContentLayout->AddElement(mTitlePrecisePctOfParent);
	preciseTitleContentLayout->AddElement(mTitlePreciseNumCalls);
	preciseTitleContentLayout->AddElement(mTitlePreciseNumAllocs);
	preciseTitleContentLayout->AddElement(mTitlePreciseNumFrees);
	preciseTitleContentLayout->AddElement(mTitlePreciseAvgCycles);
	preciseTitleContentLayout->AddElement(mTitlePreciseTotalCycles);
	preciseTitleContentLayout->AddElement(mTitlePreciseAvgCyclesSelf);
	preciseTitleContentLayout->AddElement(mTitlePreciseTotalCyclesSelf);

	mBasicLayoutLabels->AddNewElement<GUIFlexibleSpace>();
	mPreciseLayoutLabels->AddNewElement<GUIFlexibleSpace>();
	mBasicLayoutContents->AddNewElement<GUIFlexibleSpace>();
	mPreciseLayoutContents->AddNewElement<GUIFlexibleSpace>();

#if BS_SHOW_PRECISE_PROFILING == 0
	mPreciseLayoutLabels->SetActive(false);
	mPreciseLayoutContents->SetActive(false);
#endif

	// Set up GPU sample areas
	mGPULayoutFrameContents = mWidget->GetPanel()->AddNewElement<GUILayoutX>();
	mGPULayoutFrameContentsLeft = mGPULayoutFrameContents->AddNewElement<GUILayoutY>();
	mGPULayoutFrameContentsRight = mGPULayoutFrameContents->AddNewElement<GUILayoutY>();

	mGPULayoutSamples = mWidget->GetPanel()->AddNewElement<GUIPanel>();

	HString gpuSamplesStr(u8"__ProfOvGPUSamples", u8"Samples");
	mGPULayoutSamples->AddNewElement<GUILabel>(gpuSamplesStr);

	for(u32 i = 0; i < kGpuNumSampleColumns; i++)
	{
		mGPULayoutSampleLabels[i] = mGPULayoutSamples->AddNewElement<GUILayoutY>();
		mGPULayoutSampleContents[i] = mGPULayoutSamples->AddNewElement<GUILayoutY>();

		HString gpuSamplesNameStr(u8"__ProfOvGPUSampName", u8"Name");
		HString gpuSamplesTimeStr(u8"__ProfOvGPUSampTime", u8"Time");
		mGPULayoutSampleLabels[i]->AddElement(GUILabel::Create(gpuSamplesNameStr, GUIOptions(GUIOption::FixedWidth(200))));
		mGPULayoutSampleContents[i]->AddElement(GUILabel::Create(gpuSamplesTimeStr, GUIOptions(GUIOption::FixedWidth(100))));

		mGPULayoutSampleLabels[i]->AddNewElement<GUIFlexibleSpace>();
		mGPULayoutSampleContents[i]->AddNewElement<GUIFlexibleSpace>();
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

	mGPUFrameNumLbl = GUILabel::Create(mGPUFrameNumStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUTimeLbl = GUILabel::Create(mGPUTimeStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUDrawCallsLbl = GUILabel::Create(mGPUDrawCallsStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPURenTargetChangesLbl = GUILabel::Create(mGPURenTargetChangesStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUPresentsLbl = GUILabel::Create(mGPUPresentsStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUClearsLbl = GUILabel::Create(mGPUClearsStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUVerticesLbl = GUILabel::Create(mGPUVerticesStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUPrimitivesLbl = GUILabel::Create(mGPUPrimitivesStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUSamplesLbl = GUILabel::Create(mGPUSamplesStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUPipelineStateChangesLbl = GUILabel::Create(mGPUPipelineStateChangesStr, GUIOptions(GUIOption::FixedWidth(200)));

	mGPUObjectsCreatedLbl = GUILabel::Create(mGPUObjectsCreatedStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUObjectsDestroyedLbl = GUILabel::Create(mGPUObjectsDestroyedStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUResourceWritesLbl = GUILabel::Create(mGPUResourceWritesStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUResourceReadsLbl = GUILabel::Create(mGPUResourceReadsStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUParamBindsLbl = GUILabel::Create(mGPUParamBindsStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUVertexBufferBindsLbl = GUILabel::Create(mGPUVertexBufferBindsStr, GUIOptions(GUIOption::FixedWidth(200)));
	mGPUIndexBufferBindsLbl = GUILabel::Create(mGPUIndexBufferBindsStr, GUIOptions(GUIOption::FixedWidth(200)));

	mGPULayoutFrameContentsLeft->AddElement(mGPUFrameNumLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUTimeLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUDrawCallsLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPURenTargetChangesLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUPresentsLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUClearsLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUVerticesLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUPrimitivesLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUSamplesLbl);
	mGPULayoutFrameContentsLeft->AddElement(mGPUPipelineStateChangesLbl);
	mGPULayoutFrameContentsLeft->AddNewElement<GUIFlexibleSpace>();

	mGPULayoutFrameContentsRight->AddElement(mGPUObjectsCreatedLbl);
	mGPULayoutFrameContentsRight->AddElement(mGPUObjectsDestroyedLbl);
	mGPULayoutFrameContentsRight->AddElement(mGPUResourceWritesLbl);
	mGPULayoutFrameContentsRight->AddElement(mGPUResourceReadsLbl);
	mGPULayoutFrameContentsRight->AddElement(mGPUParamBindsLbl);
	mGPULayoutFrameContentsRight->AddElement(mGPUVertexBufferBindsLbl);
	mGPULayoutFrameContentsRight->AddElement(mGPUIndexBufferBindsLbl);
	mGPULayoutFrameContentsRight->AddNewElement<GUIFlexibleSpace>();

	UpdateCpuSampleAreaSizes();
	UpdateGpuSampleAreaSizes();

	if(!mIsShown)
		Hide();
	else
	{
		if(mType == ProfilerOverlayType::CPUSamples)
			Show(ProfilerOverlayType::CPUSamples);
		else
			Show(ProfilerOverlayType::GPUSamples);
	}
}

void ProfilerOverlay::Show(ProfilerOverlayType type)
{
	if(type == ProfilerOverlayType::CPUSamples)
	{
		mBasicLayoutLabels->SetHidden(false);
		mPreciseLayoutLabels->SetHidden(false);
		mBasicLayoutContents->SetHidden(false);
		mPreciseLayoutContents->SetHidden(false);
		mGPULayoutFrameContents->SetHidden(true);
		mGPULayoutSamples->SetHidden(true);
	}
	else
	{
		mGPULayoutFrameContents->SetHidden(false);
		mGPULayoutSamples->SetHidden(false);
		mBasicLayoutLabels->SetHidden(true);
		mPreciseLayoutLabels->SetHidden(true);
		mBasicLayoutContents->SetHidden(true);
		mPreciseLayoutContents->SetHidden(true);
	}

	mType = type;
	mIsShown = true;
}

void ProfilerOverlay::Hide()
{
	mBasicLayoutLabels->SetHidden(true);
	mPreciseLayoutLabels->SetHidden(true);
	mBasicLayoutContents->SetHidden(true);
	mPreciseLayoutContents->SetHidden(true);
	mGPULayoutFrameContents->SetHidden(true);
	mGPULayoutSamples->SetHidden(true);
	mIsShown = false;
}

void ProfilerOverlay::Update()
{
	const ProfilerReport& latestSimReport = ProfilingManager::Instance().GetReport(ProfiledThread::Main);
	const ProfilerReport& latestCoreReport = ProfilingManager::Instance().GetReport(ProfiledThread::Render);

	UpdateCpuSampleContents(latestSimReport, latestCoreReport);

	TOptional<GpuProfilerResults> lastProfilerResults = GetGpuProfiler().GetResults("RenderScene"); // Note: Must match name in the renderer
	if(lastProfilerResults.has_value())
	{
		mLastProfilerResults = std::move(*lastProfilerResults);

		if(!mLastProfilerResults.Samples.Empty())
			UpdateGpuSampleContents(mLastProfilerResults.Samples[0]);
	}
}

void ProfilerOverlay::TargetResized()
{
	UpdateCpuSampleAreaSizes();
	UpdateGpuSampleAreaSizes();
}

void ProfilerOverlay::UpdateCpuSampleAreaSizes()
{
	static const GUILogicalUnit kPadding = 10;
	static const float kLabelsContentRatio = 0.3f;

	const Area2I pixelArea = mTarget->GetPixelArea();
	const GUIPhysicalSize physicalSize(Math::Max(0, (i32)pixelArea.Width), Math::Max(0, (i32)pixelArea.Height));
	const GUILogicalSize logicalSize = GUIUtility::PhysicalToLogical(physicalSize, mDPIScale);

	const GUILogicalUnit width = Math::Max(GUILogicalUnit(0), logicalSize.Width - kPadding * 2);
	const GUILogicalUnit height = Math::Max(GUILogicalUnit(0), logicalSize.Height - kPadding * 3);

	const GUILogicalUnit labelsWidth = Math::CeilToInt((float)width.To<float>() * kLabelsContentRatio);
	const GUILogicalUnit contentWidth = width - labelsWidth;

	mBasicLayoutLabels->SetPosition(kPadding, kPadding);
	mBasicLayoutLabels->SetWidth(labelsWidth);
	mBasicLayoutLabels->SetHeight(height);

	mPreciseLayoutLabels->SetPosition(kPadding, height + kPadding * 2);
	mPreciseLayoutLabels->SetWidth(labelsWidth);
	mPreciseLayoutLabels->SetHeight(height);

	mBasicLayoutContents->SetPosition(kPadding + labelsWidth, kPadding);
	mBasicLayoutContents->SetWidth(contentWidth);
	mBasicLayoutContents->SetHeight(height);

	mPreciseLayoutContents->SetPosition(kPadding + labelsWidth, height + kPadding * 2);
	mPreciseLayoutContents->SetWidth(contentWidth);
	mPreciseLayoutContents->SetHeight(height);
}

void ProfilerOverlay::UpdateGpuSampleAreaSizes()
{
	static const GUILogicalUnit kPadding = 10;
	static const float kSamplesFrameRatio = 0.25f;
	static const GUILogicalUnit kHeaderHeight = 20;
	static const i32 kNumColumns = 3;
	static const i32 kHeightPerEntry = 15;

	const Area2I pixelArea = mTarget->GetPixelArea();
	const GUIPhysicalSize physicalSize(Math::Max(0, (i32)pixelArea.Width), Math::Max(0, (i32)pixelArea.Height));
	const GUILogicalSize logicalSize = GUIUtility::PhysicalToLogical(physicalSize, mDPIScale);

	const GUILogicalUnit width = Math::Max(GUILogicalUnit(0), logicalSize.Width - kPadding * 2);
	const GUILogicalUnit height = Math::Max(GUILogicalUnit(0), logicalSize.Height - kPadding * 3);

	const GUILogicalUnit frameHeight = Math::CeilToInt((float)height.To<float>() * kSamplesFrameRatio);
	const GUILogicalUnit samplesHeight = height - frameHeight;

	mGPULayoutFrameContents->SetPosition(kPadding, kPadding);
	mGPULayoutFrameContents->SetWidth(width);
	mGPULayoutFrameContents->SetHeight(frameHeight);

	mGPULayoutSamples->SetPosition(kPadding, kPadding + frameHeight + kPadding);
	mGPULayoutSamples->SetWidth(width);
	mGPULayoutSamples->SetHeight(samplesHeight);

	GUILogicalUnit columnWidth = width / kNumColumns;
	GUILogicalUnit columnHeight = samplesHeight - kHeaderHeight;
	for(u32 i = 0; i < kNumColumns; i++)
	{
		mGPULayoutSampleLabels[i]->SetPosition(columnWidth * i, kHeaderHeight);
		mGPULayoutSampleLabels[i]->SetWidth(columnWidth / 2);
		mGPULayoutSampleLabels[i]->SetHeight(columnHeight);

		mGPULayoutSampleContents[i]->SetPosition(columnWidth * i + columnWidth / 2, kHeaderHeight);
		mGPULayoutSampleContents[i]->SetWidth(columnWidth / 2);
		mGPULayoutSampleContents[i]->SetHeight(columnHeight);
	}

	mNumGPUSamplesPerColumn = (u32)(i32)columnHeight / kHeightPerEntry;
}

void ProfilerOverlay::UpdateCpuSampleContents(const ProfilerReport& mainThreadReport, const ProfilerReport& renderThreadReport)
{
	static const u32 kNumRootEntries = 2;

	const CPUProfilerBasicSamplingEntry& mainThreadBasicRootEntry = mainThreadReport.CpuReport.GetBasicSamplingData();
	const CPUProfilerPreciseSamplingEntry& mainThreadPreciseRootEntry = mainThreadReport.CpuReport.GetPreciseSamplingData();

	const CPUProfilerBasicSamplingEntry& renderThreadBasicRootEntry = renderThreadReport.CpuReport.GetBasicSamplingData();
	const CPUProfilerPreciseSamplingEntry& renderThreadPreciseRootEntry = renderThreadReport.CpuReport.GetPreciseSamplingData();

	struct TodoBasic
	{
		TodoBasic(const CPUProfilerBasicSamplingEntry& _entry, u32 _depth)
			: Entry(_entry), Depth(_depth)
		{}

		const CPUProfilerBasicSamplingEntry& Entry;
		u32 Depth;
	};

	struct TodoPrecise
	{
		TodoPrecise(const CPUProfilerPreciseSamplingEntry& _entry, u32 _depth)
			: Entry(_entry), Depth(_depth)
		{}

		const CPUProfilerPreciseSamplingEntry& Entry;
		u32 Depth;
	};

	BasicRowFiller basicRowFiller(mBasicRows, *mBasicLayoutLabels, *mBasicLayoutContents, *mWidget);
	Stack<TodoBasic> todoBasic;

	const CPUProfilerBasicSamplingEntry* basicRootEntries[kNumRootEntries];
	basicRootEntries[0] = &mainThreadBasicRootEntry;
	basicRootEntries[1] = &renderThreadBasicRootEntry;

	for(u32 i = 0; i < kNumRootEntries; i++)
	{
		todoBasic.push(TodoBasic(*basicRootEntries[i], 0));

		while(!todoBasic.empty())
		{
			TodoBasic curEntry = todoBasic.top();
			todoBasic.pop();

			const struct CPUProfilerBasicSamplingEntry::Data& data = curEntry.Entry.Data;
			basicRowFiller.AddData(curEntry.Depth, data.Name, data.PctOfParent, data.NumCalls, data.MemAllocs, data.MemFrees, data.AvgTimeMs, data.TotalTimeMs, data.AvgSelfTimeMs, data.TotalSelfTimeMs);

			if(curEntry.Depth <= kMaxDepth)
			{
				for(auto iter = curEntry.Entry.ChildEntries.rbegin(); iter != curEntry.Entry.ChildEntries.rend(); ++iter)
				{
					todoBasic.push(TodoBasic(*iter, curEntry.Depth + 1));
				}
			}
		}
	}

	PreciseRowFiller preciseRowFiller(mPreciseRows, *mBasicLayoutLabels, *mBasicLayoutContents, *mWidget);
	Stack<TodoPrecise> todoPrecise;

	const CPUProfilerPreciseSamplingEntry* preciseRootEntries[kNumRootEntries];
	preciseRootEntries[0] = &mainThreadPreciseRootEntry;
	preciseRootEntries[1] = &renderThreadPreciseRootEntry;

	for(u32 i = 0; i < kNumRootEntries; i++)
	{
		todoPrecise.push(TodoPrecise(*preciseRootEntries[i], 0));

		while(!todoBasic.empty())
		{
			TodoPrecise curEntry = todoPrecise.top();
			todoPrecise.pop();

			const struct CPUProfilerPreciseSamplingEntry::Data& data = curEntry.Entry.Data;
			preciseRowFiller.AddData(curEntry.Depth, data.Name, data.PctOfParent, data.NumCalls, data.MemAllocs, data.MemFrees, data.AvgCycles, data.TotalCycles, data.AvgSelfCycles, data.TotalSelfCycles);

			if(curEntry.Depth <= kMaxDepth)
			{
				for(auto iter = curEntry.Entry.ChildEntries.rbegin(); iter != curEntry.Entry.ChildEntries.rend(); ++iter)
				{
					todoPrecise.push(TodoPrecise(*iter, curEntry.Depth + 1));
				}
			}
		}
	}
}

void ProfilerOverlay::UpdateGpuSampleContents(const GpuProfilerSample& frameSample)
{
	mGPUFrameNumStr.SetParameter(0, ToString((u64)GetTime().GetCurrentFrameIndex()));
	mGPUTimeStr.SetParameter(0, ToString(frameSample.TimeMs));
	mGPUDrawCallsStr.SetParameter(0, ToString(frameSample.DrawCallCount));
	mGPURenTargetChangesStr.SetParameter(0, ToString(frameSample.RenderTargetChangesCount));
	mGPUPresentsStr.SetParameter(0, ToString(frameSample.PresentCount));
	mGPUClearsStr.SetParameter(0, ToString(frameSample.ClearCount));
	mGPUVerticesStr.SetParameter(0, ToString(frameSample.VerticesDrawn));
	mGPUPrimitivesStr.SetParameter(0, ToString(frameSample.PrimitivesDrawn));
	mGPUSamplesStr.SetParameter(0, ToString(frameSample.SamplesDrawn));
	mGPUPipelineStateChangesStr.SetParameter(0, ToString(frameSample.PipelineStateChangeCount));

	mGPUObjectsCreatedStr.SetParameter(0, ToString(frameSample.ObjectsCreatedCount));
	mGPUObjectsDestroyedStr.SetParameter(0, ToString(frameSample.ObjectsDestroyedCount));
	mGPUResourceWritesStr.SetParameter(0, ToString(frameSample.ResourceWriteCount));
	mGPUResourceReadsStr.SetParameter(0, ToString(frameSample.ResourceReadCount));
	mGPUParamBindsStr.SetParameter(0, ToString(frameSample.GpuParameterBindCount));
	mGPUVertexBufferBindsStr.SetParameter(0, ToString(frameSample.VertexBufferBindCount));
	mGPUIndexBufferBindsStr.SetParameter(0, ToString(frameSample.IndexBufferBindCount));

	mGPUFrameNumLbl->SetContent(mGPUFrameNumStr);
	mGPUTimeLbl->SetContent(mGPUTimeStr);
	mGPUDrawCallsLbl->SetContent(mGPUDrawCallsStr);
	mGPURenTargetChangesLbl->SetContent(mGPURenTargetChangesStr);
	mGPUPresentsLbl->SetContent(mGPUPresentsStr);
	mGPUClearsLbl->SetContent(mGPUClearsStr);
	mGPUVerticesLbl->SetContent(mGPUVerticesStr);
	mGPUPrimitivesLbl->SetContent(mGPUPrimitivesStr);
	mGPUSamplesLbl->SetContent(mGPUSamplesStr);
	mGPUPipelineStateChangesLbl->SetContent(mGPUPipelineStateChangesStr);

	mGPUObjectsCreatedLbl->SetContent(mGPUObjectsCreatedStr);
	mGPUObjectsDestroyedLbl->SetContent(mGPUObjectsDestroyedStr);
	mGPUResourceWritesLbl->SetContent(mGPUResourceWritesStr);
	mGPUResourceReadsLbl->SetContent(mGPUResourceReadsStr);
	mGPUParamBindsLbl->SetContent(mGPUParamBindsStr);
	mGPUVertexBufferBindsLbl->SetContent(mGPUVertexBufferBindsStr);
	mGPUIndexBufferBindsLbl->SetContent(mGPUIndexBufferBindsStr);

	GPUSampleRowFiller sampleRowFillers[kGpuNumSampleColumns] = {
		GPUSampleRowFiller(mGPUSampleRows[0], *mGPULayoutSampleLabels[0], *mGPULayoutSampleContents[0], *mWidget),
		GPUSampleRowFiller(mGPUSampleRows[1], *mGPULayoutSampleLabels[1], *mGPULayoutSampleContents[1], *mWidget),
		GPUSampleRowFiller(mGPUSampleRows[2], *mGPULayoutSampleLabels[2], *mGPULayoutSampleContents[2], *mWidget)
	};

	struct Todo
	{
		Todo(const GpuProfilerSample& entry, u32 depth)
			: Entry(entry), Depth(depth)
		{}

		const GpuProfilerSample& Entry;
		u32 Depth;
	};

	u32 column = 0;
	u32 currentCount = 0;

	Stack<Todo> todo;
	todo.push(Todo(frameSample, 0));

	while(!todo.empty())
	{
		Todo curEntry = todo.top();
		todo.pop();

		const GpuProfilerSample& data = curEntry.Entry;

		if(column < kGpuNumSampleColumns)
			sampleRowFillers[column].AddData(curEntry.Depth, data.Name, data.TimeMs);

		currentCount++;
		if(currentCount % mNumGPUSamplesPerColumn == 0)
			column++;

		if(curEntry.Depth <= kMaxDepth)
		{
			for(auto iter = curEntry.Entry.ChildSamples.rbegin(); iter != curEntry.Entry.ChildSamples.rend(); ++iter)
				todo.push(Todo(*iter, curEntry.Depth + 1));
		}
	}
}
