//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**
	 * Interface that provides a method for finding a shader include resource based on the name of the include that was 
	 * provided in a shader file.
	 */
	class BS_CORE_EXPORT IShaderIncludeHandler
	{
	public:
		virtual ~IShaderIncludeHandler() = default;

		/** Attempts to find a shader include resource based on its name. */
		virtual HShaderInclude findInclude(const String& name) const = 0;

		/** Registers a path in which to look for shader include files, along the default places. */
		virtual void addSearchPath(const Path& path) { }
	};

	/**
	 * Implements shader include finding by converting the shader include name into a path that the resource will be loaded 
	 * from.
	 */
	class BS_CORE_EXPORT DefaultShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		/** @copydoc IShaderIncludeHandler::findInclude */
		HShaderInclude findInclude(const String& name) const override;
	};

	/**	A global manager that handles various shader specific operations. */
	class BS_CORE_EXPORT ShaderManager : public Module <ShaderManager>
	{
	public:
		ShaderManager(const SPtr<IShaderIncludeHandler>& handler) { mIncludeHandler = handler; }

		/**
		 * Attempts to find a shader include based on the include name.
		 *
		 * @note	
		 * The name is usually a path to the resource relative to the working folder, but can be other things depending on 
		 * active handler.
		 */
		HShaderInclude findInclude(const String& name) const;

		/** Changes the active include handler that determines how is a shader include name mapped to the actual resource. */
		void setIncludeHandler(const SPtr<IShaderIncludeHandler>& handler) { mIncludeHandler = handler; }

		/** Registers a path in which to look for shader include files, along the default places. */
		void addSearchPath(const Path& path);
	private:
		SPtr<IShaderIncludeHandler> mIncludeHandler;
	};

	/** @} */
}