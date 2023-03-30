//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuProgram.h"

#include "BsCoreApplication.h"
#include "RenderAPI/BsGpuDeviceCapabilities.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

using namespace bs;

RTTITypeBase* GpuProgramCreateInformation::GetRttiStatic()
{
	return GpuProgramCreateInformationRTTI::Instance();
}

RTTITypeBase* GpuProgramCreateInformation::GetRtti() const
{
	return GetRttiStatic();
}

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

SPtr<GpuProgramParameterDescription> GpuProgram::GetParameterDescription() const
{
	return GetCore()->GetParameterDescription();
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

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	return gpuDevice->CreateGpuProgram(createInformation, true);
}

SPtr<GpuProgram> GpuProgram::Create(const GpuProgramCreateInformation& createInformation)
{
	GpuProgram* const program = new(B3DAllocate<GpuProgram>()) GpuProgram(createInformation);
	SPtr<GpuProgram> shared = B3DMakeCoreFromExisting<GpuProgram>(program);
	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

SPtr<GpuProgram> GpuProgram::CreateEmpty()
{
	GpuProgram* program = new(B3DAllocate<GpuProgram>()) GpuProgram(GpuProgramCreateInformation());
	SPtr<GpuProgram> shared = B3DMakeCoreFromExisting<GpuProgram>(program);
	shared->SetShared(shared);

	return shared;
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
GpuProgram::GpuProgram(const GpuProgramCreateInformation& createInformation)
	: mNeedsAdjacencyInfo(createInformation.RequiresAdjacency), mName(createInformation.Name), mType(createInformation.Type), mEntryPoint(createInformation.EntryPoint), mSource(createInformation.Source), mBytecode(createInformation.Bytecode)
{
	mParametersDesc = B3DMakeShared<GpuProgramParameterDescription>();
}

GpuProgram::~GpuProgram()
{}

bool GpuProgram::IsSupported() const
{
	return true;
}
}}
