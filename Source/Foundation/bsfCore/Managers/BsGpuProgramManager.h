//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Error/BsException.h"
#include "RenderAPI/BsGpuProgram.h"

namespace bs 
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Manager responsible for creating GPU programs. It will automatically try to find the appropriate handler for a 
	 * specific GPU program language and create the program if possible.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuProgramManager : public Module<GpuProgramManager>
	{
	public:
		/** @copydoc GpuProgram::create */
		SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc);

		/**
		 * Creates a completely empty and uninitialized GpuProgram. Should only be used for specific purposes, like 
		 * deserialization, as it requires additional manual initialization that is not required normally.
		 */
		SPtr<GpuProgram> createEmpty(const String& language, GpuProgramType type);
	};

	namespace ct
	{
	/** Factory responsible for creating GPU programs of a certain type. */
	class BS_CORE_EXPORT GpuProgramFactory
	{
	public:
		GpuProgramFactory() {}
		virtual ~GpuProgramFactory() { }

		/** @copydoc GpuProgram::create */
		virtual SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc bs::GpuProgramManager::createEmpty */
		virtual SPtr<GpuProgram> create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc GpuProgram::compileBytecode */
		virtual SPtr<GpuProgramBytecode> compileBytecode(const GPU_PROGRAM_DESC& desc) = 0;
	};

	/**
	 * Manager responsible for creating GPU programs. It will automatically	try to find the appropriate handler for a 
	 * specific GPU program language and create the program if possible.
	 *
	 * @note	Core thread only unless otherwise specified.
	 */
	class BS_CORE_EXPORT GpuProgramManager : public Module<GpuProgramManager>
	{
	public:
		GpuProgramManager();
		virtual ~GpuProgramManager();

		/**
		 * Registers a new factory that is able to create GPU programs for a certain language. If any other factory for the
		 * same language exists, it will overwrite it.
		 */
		void addFactory(const String& language, GpuProgramFactory* factory);

		/**
		 * Unregisters a GPU program factory, essentially making it not possible to create GPU programs using the language 
		 * the factory supported.
		 */
		void removeFactory(const String& language);

		/** Query if a GPU program language is supported (for example "hlsl", "glsl"). Thread safe. */
		bool isLanguageSupported(const String& language);

		/** @copydoc GpuProgram::create */
		SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** @copydoc GpuProgram::compileBytecode */
		SPtr<GpuProgramBytecode> compileBytecode(const GPU_PROGRAM_DESC& desc);

	protected:
		friend class bs::GpuProgram;

		/**
		 * Creates a GPU program without initializing it.
		 *
		 * @see		create
		 */
		SPtr<GpuProgram> createInternal(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** Attempts to find a factory for the specified language. Returns null if it cannot find one. */
		GpuProgramFactory* getFactory(const String& language);

	protected:
		Mutex mMutex;

		UnorderedMap<String, GpuProgramFactory*> mFactories;
		GpuProgramFactory* mNullFactory; /**< Factory for dealing with GPU programs that can't be created. */
	};
	}
	/** @} */
}