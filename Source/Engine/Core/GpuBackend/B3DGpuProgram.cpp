//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "RTTI/B3DGpuProgramRTTI.h"

using namespace b3d;

RTTIType* GpuProgramCreateInformation::GetRttiStatic()
{
	return GpuProgramCreateInformationRTTI::Instance();
}

RTTIType* GpuProgramCreateInformation::GetRtti() const
{
	return GetRttiStatic();
}

GpuProgramBytecode::~GpuProgramBytecode()
{
	if(Instructions.Data)
		B3DFree(Instructions.Data);
}

RTTIType* GpuProgramBytecode::GetRttiStatic()
{
	return GpuProgramBytecodeRTTI::Instance();
}

RTTIType* GpuProgramBytecode::GetRtti() const
{
	return GpuProgramBytecode::GetRttiStatic();
}

TArrayView<const GpuDescriptorTableEntry> GpuResourceTableLayout::GetEntries(const GpuDescriptorTable& table) const
{
	if(table.EntryCount == 0)
		return TArrayView<const GpuDescriptorTableEntry>();

	return TArrayView<const GpuDescriptorTableEntry>(Entries.data() + table.FirstEntry, table.EntryCount);
}

u32 GpuResourceTableLayout::FindSetTableIndex(u32 set) const
{
	if(IsEmpty())
		return ~0u;

	for(const GpuDescriptorTableEntry& entry : GetEntries(GetRootTable()))
	{
		if(entry.Kind != GpuDescriptorEntryKind::SubTable)
			continue;

		if(Tables[entry.TableIndex].Set == set)
			return entry.TableIndex;
	}

	return ~0u;
}

const GpuDescriptorTableEntry* GpuResourceTableLayout::FindResourceEntry(const GpuDescriptorTable& table, GpuParameterType type, u32 slot) const
{
	for(const GpuDescriptorTableEntry& entry : GetEntries(table))
	{
		if(entry.Kind == GpuDescriptorEntryKind::Resource && entry.Type == type && entry.Slot == slot)
			return &entry;
	}

	return nullptr;
}

const GpuDescriptorTableEntry* GpuResourceTableLayout::FindRootResourceEntry(GpuParameterType type, u32 set, u32 slot) const
{
	if(IsEmpty())
		return nullptr;

	for(const GpuDescriptorTableEntry& entry : GetEntries(GetRootTable()))
	{
		if(entry.Kind == GpuDescriptorEntryKind::Resource && entry.Type == type && entry.Set == set && entry.Slot == slot)
			return &entry;
	}

	return nullptr;
}

RTTIType* GpuResourceTableLayout::GetRttiStatic()
{
	return GpuResourceTableLayoutRTTI::Instance();
}

RTTIType* GpuResourceTableLayout::GetRtti() const
{
	return GpuResourceTableLayout::GetRttiStatic();
}

GpuProgram::GpuProgram(const GpuProgramCreateInformation& createInformation)
	: mNeedsAdjacencyInfo(createInformation.RequiresAdjacency), mLanguage(createInformation.Language), mName(createInformation.Name), mType(createInformation.Type), mEntryPoint(createInformation.EntryPoint), mSource(createInformation.Source), mBytecode(createInformation.Bytecode)
{
	mParametersDescription = B3DMakeShared<GpuProgramParameterDescription>();
}

GpuProgram::~GpuProgram()
{}

bool GpuProgram::IsSupported() const
{
	return true;
}
/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* GpuProgram::GetRttiStatic()
{
	return GpuProgramRTTI::Instance();
}

RTTIType* GpuProgram::GetRtti() const
{
	return GpuProgram::GetRttiStatic();
}
