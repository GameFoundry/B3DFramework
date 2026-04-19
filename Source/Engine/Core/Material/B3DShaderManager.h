//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**
	 * Interface that provides a method for finding a shader include resource based on the name of the include that was
	 * provided in a shader file.
	 */
	class B3D_EXPORT IShaderIncludeHandler
	{
	public:
		virtual ~IShaderIncludeHandler() = default;

		/**
		 * Attempts to find a shader include resource based on its name.
		 *
		 * @note
		 * The name is usually a path to the resource relative to the working folder, but can be other things depending on
		 * active handler.
		 */
		virtual HShaderInclude FindInclude(const String& name) const = 0;

		/** Attempts to find a shader include based on the include name and returns the include source code if found. */
		virtual TOptional<String> FindIncludeSource(const String& name) const = 0;

		/** Registers a path in which to look for shader include files, along the default places. */
		virtual void AddSearchPath(const Path& path) {}
	};

	/**
	 * Implements shader include finding by converting the shader include name into a path that the resource will be loaded
	 * from.
	 */
	class B3D_EXPORT DefaultShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		HShaderInclude FindInclude(const String& name) const override;
		TOptional<String> FindIncludeSource(const String& name) const override;
	};

	/**	A global manager that handles various shader specific operations. */
	class B3D_EXPORT ShaderManager : public Module<ShaderManager>
	{
	public:
		ShaderManager(const SPtr<IShaderIncludeHandler>& handler) { mIncludeHandler = handler; }

		/** @copydoc IShaderIncludeHandler::FindInclude */
		HShaderInclude FindInclude(const String& name) const;

		/** @copydoc IShaderIncludeHandler::FindIncludeSource */
		TOptional<String> FindIncludeSource(const String& name) const;

		/** Changes the active include handler that determines how is a shader include name mapped to the actual resource. */
		void SetIncludeHandler(const SPtr<IShaderIncludeHandler>& handler) { mIncludeHandler = handler; }

		/** Registers a path in which to look for shader include files, along the default places. */
		void AddSearchPath(const Path& path);

	private:
		SPtr<IShaderIncludeHandler> mIncludeHandler;
	};

	/** @} */
} // namespace b3d
