//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Plugin/B3DPluginLoader.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/** Factory class registered when a plugin supporting a new GpuBackend is loaded. */
	class GpuBackendFactory
	{
	public:
		virtual ~GpuBackendFactory() = default;

		/**	Creates and starts up the GPU backend managed by this factory.  */
		virtual void Create() = 0;

		/**	Returns the name of the GPU backend this factory creates. */
		virtual const char* Name() const = 0;
	};

	/** Manager that handles GPU backend initialization. */
	class B3D_EXPORT GpuBackendManager : public Module<GpuBackendManager>
	{
	public:
		GpuBackendManager() = default;
		~GpuBackendManager();

		/** Loads a plugin with the provided name and initializes the GpuBackend from the plugin. */
		void Initialize(const String& pluginFilename);

	private:
		LoadedPlugin mPlugin;
		GpuBackendFactory* mFactory = nullptr;
		bool mRenderAPIInitialized = false;
	};

	/** @} */
} // namespace b3d
