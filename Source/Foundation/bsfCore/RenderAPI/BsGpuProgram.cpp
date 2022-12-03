//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsRenderAPICapabilities.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsGpuProgramManager.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

using namespace bs;

GpuProgramBytecode::~GpuProgramBytecode()
{
	if(Instructions.Data)
		B3DFree(Instructions.Data);
}

RTTITypeBase* GpuProgramBytecode::GetRttiStatic()
{
	return GpuProgramBytecodeRTTI::Instance();
}

RTTITypeBase* GpuProgramBytecode::GetRtti() const
{
	return GpuProgramBytecode::GetRttiStatic();
}

GpuProgram::GpuProgram(const GpuProgramCreateInformation& createInformation)
	: mNeedsAdjacencyInfo(createInformation.RequiresAdjacency), mLanguage(createInformation.Language), mName(createInformation.Name), mType(createInformation.Type), mEntryPoint(createInformation.EntryPoint), mSource(createInformation.Source)
{
}

bool GpuProgram::IsCompiled() const
{
	return GetCore()->IsCompiled();
}

String GpuProgram::GetCompileErrorMessage() const
{
	return GetCore()->GetCompileErrorMessage();
}

SPtr<GpuParamDesc> GpuProgram::GetParamDesc() const
{
	return GetCore()->GetParamDesc();
}

SPtr<ct::GpuProgram> GpuProgram::GetCore() const
{
	return std::static_pointer_cast<ct::GpuProgram>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuProgram::CreateCore() const
{
	GpuProgramCreateInformation createInformation;
	createInformation.Name = mName;
	createInformation.Source = mSource;
	createInformation.EntryPoint = mEntryPoint;
	createInformation.Language = mLanguage;
	createInformation.Type = mType;
	createInformation.RequiresAdjacency = mNeedsAdjacencyInfo;
	createInformation.Bytecode = mBytecode;

	return ct::GpuProgramManager::Instance().CreateInternal(createInformation);
}

SPtr<GpuProgram> GpuProgram::Create(const GpuProgramCreateInformation& desc)
{
	return GpuProgramManager::Instance().Create(desc);
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* GpuProgram::GetRttiStatic()
{
	return GpuProgramRTTI::Instance();
}

RTTITypeBase* GpuProgram::GetRtti() const
{
	return GpuProgram::GetRttiStatic();
}

namespace bs { namespace ct
{
GpuProgram::GpuProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: mNeedsAdjacencyInfo(desc.RequiresAdjacency), mName(desc.Name), mType(desc.Type), mEntryPoint(desc.EntryPoint), mSource(desc.Source), mBytecode(desc.Bytecode)
{
	mParametersDesc = B3DMakeShared<GpuParamDesc>();
}

GpuProgram::~GpuProgram()
{}

bool GpuProgram::IsSupported() const
{
	return true;
}

SPtr<GpuProgram> GpuProgram::Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	return GpuProgramManager::Instance().Create(desc, deviceMask);
}

SPtr<GpuProgramBytecode> GpuProgram::CompileBytecode(const GpuProgramCreateInformation& desc)
{
	return GpuProgramManager::Instance().CompileBytecode(desc);
}
}}
