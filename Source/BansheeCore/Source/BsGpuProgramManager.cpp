//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuProgramManager.h"
#include "BsRenderAPI.h"

namespace bs 
{
	SPtr<GpuProgram> GpuProgramManager::create(const GPU_PROGRAM_DESC& desc)
	{
		GpuProgram* program = new (bs_alloc<GpuProgram>()) GpuProgram(desc);
		SPtr<GpuProgram> ret = bs_core_ptr<GpuProgram>(program);
		ret->_setThisPtr(ret);
		ret->initialize();

		return ret;
	}

	SPtr<GpuProgram> GpuProgramManager::createEmpty(const String& language, GpuProgramType type)
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
	class NullProgramCore : public GpuProgramCore
	{
	public:
		NullProgramCore()
			:GpuProgramCore(GPU_PROGRAM_DESC(), GDF_DEFAULT)
		{ }

		~NullProgramCore() { }

		bool isSupported() const { return false; }
		const String& getLanguage() const { return sNullLang; }

	protected:
		void loadFromSource() {}

		void buildConstantDefinitions() const { }
	};

	/**	Factory that creates null GPU programs.  */
	class NullProgramFactory : public GpuProgramFactory
	{
	public:
		NullProgramFactory() {}
		~NullProgramFactory() {}

		const String& getLanguage() const override
		{ 
			return sNullLang;
		}

		SPtr<GpuProgramCore> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask) override
		{
			SPtr<NullProgramCore> ret = bs_shared_ptr_new<NullProgramCore>();
			ret->_setThisPtr(ret);

			return ret;
		}

		SPtr<GpuProgramCore> create(GpuProgramType type, GpuDeviceFlags deviceMask) override
		{
			SPtr<NullProgramCore> ret = bs_shared_ptr_new<NullProgramCore>();
			ret->_setThisPtr(ret);

			return ret;
		}
	};

	GpuProgramManager::GpuProgramManager()
	{
		mNullFactory = bs_new<NullProgramFactory>();
		addFactory(mNullFactory);
	}

	GpuProgramManager::~GpuProgramManager()
	{
		bs_delete((NullProgramFactory*)mNullFactory);
	}

	void GpuProgramManager::addFactory(GpuProgramFactory* factory)
	{
		mFactories[factory->getLanguage()] = factory;
	}

	void GpuProgramManager::removeFactory(GpuProgramFactory* factory)
    {
        FactoryMap::iterator it = mFactories.find(factory->getLanguage());
        if (it != mFactories.end() && it->second == factory)
        {
            mFactories.erase(it);
        }
    }

	GpuProgramFactory* GpuProgramManager::getFactory(const String& language)
	{
		FactoryMap::iterator i = mFactories.find(language);

		if (i == mFactories.end())
			i = mFactories.find(sNullLang);

		return i->second;
	}

	bool GpuProgramManager::isLanguageSupported(const String& lang)
	{
		FactoryMap::iterator i = mFactories.find(lang);

		return i != mFactories.end();
	}

	SPtr<GpuProgramCore> GpuProgramManager::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
    {
		SPtr<GpuProgramCore> ret = createInternal(desc, deviceMask);
		ret->initialize();

        return ret;
    }

	SPtr<GpuProgramCore> GpuProgramManager::createInternal(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		GpuProgramFactory* factory = getFactory(desc.language);
		SPtr<GpuProgramCore> ret = factory->create(desc, deviceMask);

		return ret;
	}
	}
}
