//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsGpuProgramManager.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

SPtr<GpuProgram> GpuProgramManager::Create(const GPU_PROGRAM_DESC& desc)
{
	GpuProgram* program = new(B3DAllocate<GpuProgram>()) GpuProgram(desc);
	SPtr<GpuProgram> ret = B3DMakeCoreFromExisting<GpuProgram>(program);
	ret->SetThisPtrInternal(ret);
	ret->Initialize();

	return ret;
}

SPtr<GpuProgram> GpuProgramManager::CreateEmpty(const String& language, GpuProgramType type)
{
	GPU_PROGRAM_DESC desc;
	desc.Language = language;
	desc.Type = type;

	GpuProgram* program = new(B3DAllocate<GpuProgram>()) GpuProgram(desc);
	SPtr<GpuProgram> ret = B3DMakeCoreFromExisting<GpuProgram>(program);
	ret->SetThisPtrInternal(ret);

	return ret;
}

namespace bs { namespace ct
{
String sNullLang = "null";

/** Null GPU program used in place of GPU programs we cannot create. Null programs don't do anything. */
class NullProgram final : public GpuProgram
{
public:
	NullProgram()
		: GpuProgram(GPU_PROGRAM_DESC(), GDF_DEFAULT)
	{}

	~NullProgram() = default;

	bool IsSupported() const { return false; }
};

SPtr<GpuProgram> NullProgramFactory::Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<NullProgram> ret = B3DMakeShared<NullProgram>();
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<GpuProgram> NullProgramFactory::Create(GpuProgramType type, GpuDeviceFlags deviceMask)
{
	SPtr<NullProgram> ret = B3DMakeShared<NullProgram>();
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<GpuProgramBytecode> NullProgramFactory::CompileBytecode(const GPU_PROGRAM_DESC& desc)
{
	auto bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = "Null";

	return bytecode;
}

GpuProgramManager::GpuProgramManager()
{
	mNullFactory = B3DNew<NullProgramFactory>();
	AddFactory(sNullLang, mNullFactory);
}

GpuProgramManager::~GpuProgramManager()
{
	B3DDelete((NullProgramFactory*)mNullFactory);
}

void GpuProgramManager::AddFactory(const String& language, GpuProgramFactory* factory)
{
	Lock lock(mMutex);

	mFactories[language] = factory;
}

void GpuProgramManager::RemoveFactory(const String& language)
{
	Lock lock(mMutex);

	auto iter = mFactories.find(language);
	if(iter != mFactories.end())
		mFactories.erase(iter);
}

GpuProgramFactory* GpuProgramManager::GetFactory(const String& language)
{
	auto iter = mFactories.find(language);
	if(iter == mFactories.end())
		iter = mFactories.find(sNullLang);

	return iter->second;
}

bool GpuProgramManager::IsLanguageSupported(const String& lang)
{
	Lock lock(mMutex);

	auto iter = mFactories.find(lang);
	return iter != mFactories.end();
}

SPtr<GpuProgram> GpuProgramManager::Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<GpuProgram> ret = CreateInternal(desc, deviceMask);
	ret->Initialize();

	return ret;
}

SPtr<GpuProgram> GpuProgramManager::CreateInternal(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
{
	GpuProgramFactory* factory = GetFactory(desc.Language);
	SPtr<GpuProgram> ret = factory->Create(desc, deviceMask);

	return ret;
}

SPtr<GpuProgramBytecode> GpuProgramManager::CompileBytecode(const GPU_PROGRAM_DESC& desc)
{
	GpuProgramFactory* factory = GetFactory(desc.Language);
	return factory->CompileBytecode(desc);
}
}}
