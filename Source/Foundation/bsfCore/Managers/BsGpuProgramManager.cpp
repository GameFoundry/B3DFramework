//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsGpuProgramManager.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	SPtr<GpuProgram> GpuProgramManager::Create(const GPU_PROGRAM_DESC& desc)
	{
		GpuProgram* program = new (bs_alloc<GpuProgram>()) GpuProgram(desc);
		SPtr<GpuProgram> ret = bs_core_ptr<GpuProgram>(program);
		ret->_setThisPtr(ret);
		ret->Initialize();

		return ret;
	}

	SPtr<GpuProgram> GpuProgramManager::CreateEmpty(const String& language, GpuProgramType type)
	{
		GPU_PROGRAM_DESC desc;
		desc.language = language;
		desc.type = type;

		GpuProgram* program = new (bs_alloc<GpuProgram>()) GpuProgram(desc);
		SPtr<GpuProgram> ret = bs_core_ptr<GpuProgram>(program);
		ret->_setThisPtr(ret);

		return ret;
	}

	namespace ct
	{
	String sNullLang = "null";

	/** Null GPU program used in place of GPU programs we cannot create. Null programs don't do anything. */
	class NullProgram final : public GpuProgram
	{
	public:
		NullProgram()
			:GpuProgram(GPU_PROGRAM_DESC(), GDF_DEFAULT)
		{ }

		~NullProgram() = default;

		bool IsSupported() const override { return false; }
	};

	SPtr<GpuProgram> NullProgramFactory::Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		SPtr<NullProgram> ret = bs_shared_ptr_new<NullProgram>();
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuProgram> NullProgramFactory::Create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		SPtr<NullProgram> ret = bs_shared_ptr_new<NullProgram>();
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuProgramBytecode> NullProgramFactory::CompileBytecode(const GPU_PROGRAM_DESC& desc)
	{
		auto bytecode = bs_shared_ptr_new<GpuProgramBytecode>();
		bytecode->compilerId = "Null";

		return bytecode;
	}

	GpuProgramManager::GpuProgramManager()
	{
		mNullFactory = bs_new<NullProgramFactory>();
		addFactory(sNullLang, mNullFactory);
	}

	GpuProgramManager::~GpuProgramManager()
	{
		bs_delete((NullProgramFactory*)mNullFactory);
	}

	void GpuProgramManager::AddFactory(const String& language, GpuProgramFactory* factory)
	{
		Lock Lock(mMutex);

		mFactories[language] = factory;
	}

	void GpuProgramManager::RemoveFactory(const String& language)
	{
		Lock Lock(mMutex);

		auto iter = mFactories.Find(language);
		if (iter != mFactories.End())
			mFactories.Erase(iter);
	}

	GpuProgramFactory* GpuProgramManager::getFactory(const String& language)
	{
		auto iter = mFactories.Find(language);
		if (iter == mFactories.End())
			iter = mFactories.Find(sNullLang);

		return iter->second;
	}

	bool GpuProgramManager::IsLanguageSupported(const String& lang)
	{
		Lock Lock(mMutex);

		auto iter = mFactories.Find(lang);
		return iter != mFactories.End();
	}

	SPtr<GpuProgram> GpuProgramManager::Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuProgram> ret = createInternal(desc, deviceMask);
		ret->Initialize();

		return ret;
	}

	SPtr<GpuProgram> GpuProgramManager::CreateInternal(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		GpuProgramFactory* factory = getFactory(desc.language);
		SPtr<GpuProgram> ret = factory->Create(desc, deviceMask);

		return ret;
	}

	SPtr<GpuProgramBytecode> GpuProgramManager::CompileBytecode(const GPU_PROGRAM_DESC& desc)
	{
		GpuProgramFactory* factory = getFactory(desc.language);
		return factory->CompileBytecode(desc);
	}
	}
}
