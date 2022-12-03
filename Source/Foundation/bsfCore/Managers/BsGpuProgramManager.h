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
	class B3D_CORE_EXPORT GpuProgramManager : public Module<GpuProgramManager>
	{
	public:
		/** @copydoc GpuProgram::Create */
		SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc);

		/**
		 * Creates a completely empty and uninitialized GpuProgram. Should only be used for specific purposes, like
		 * deserialization, as it requires additional manual initialization that is not required normally.
		 */
		SPtr<GpuProgram> CreateEmpty(const String& language, GpuProgramType type);
	};

	namespace ct
	{
		/** Factory responsible for creating GPU programs of a certain type. */
		class B3D_CORE_EXPORT GpuProgramFactory
		{
		public:
			GpuProgramFactory() = default;
			virtual ~GpuProgramFactory() = default;

			/** @copydoc GpuProgram::Create */
			virtual SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc bs::GpuProgramManager::CreateEmpty */
			virtual SPtr<GpuProgram> Create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc GpuProgram::CompileBytecode */
			virtual SPtr<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& desc) = 0;
		};

		/**
		 * Manager responsible for creating GPU programs. It will automatically	try to find the appropriate handler for a
		 * specific GPU program language and create the program if possible.
		 *
		 * @note	Core thread only unless otherwise specified.
		 */
		class B3D_CORE_EXPORT GpuProgramManager : public Module<GpuProgramManager>
		{
		public:
			GpuProgramManager();
			virtual ~GpuProgramManager();

			/**
			 * Registers a new factory that is able to create GPU programs for a certain language. If any other factory for the
			 * same language exists, it will overwrite it.
			 */
			void AddFactory(const String& language, GpuProgramFactory* factory);

			/**
			 * Unregisters a GPU program factory, essentially making it not possible to create GPU programs using the language
			 * the factory supported.
			 */
			void RemoveFactory(const String& language);

			/** Query if a GPU program language is supported (for example "hlsl", "glsl"). Thread safe. */
			bool IsLanguageSupported(const String& language);

			/** @copydoc GpuProgram::Create */
			SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuProgram::CompileBytecode */
			SPtr<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& desc);

		protected:
			friend class bs::GpuProgram;

			/**
			 * Creates a GPU program without initializing it.
			 *
			 * @see		create
			 */
			SPtr<GpuProgram> CreateInternal(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** Attempts to find a factory for the specified language. Returns null if it cannot find one. */
			GpuProgramFactory* GetFactory(const String& language);

		protected:
			Mutex mMutex;

			UnorderedMap<String, GpuProgramFactory*> mFactories;
			GpuProgramFactory* mNullFactory; /**< Factory for dealing with GPU programs that can't be created. */
		};

		/**	Factory that creates null GPU programs.  */
		class B3D_CORE_EXPORT NullProgramFactory : public GpuProgramFactory
		{
		public:
			NullProgramFactory() = default;
			~NullProgramFactory() = default;

			SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask) override;
			SPtr<GpuProgram> Create(GpuProgramType type, GpuDeviceFlags deviceMask) override;
			SPtr<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& desc) override;
		};
	} // namespace ct

	/** @} */
} // namespace bs
