//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsRendererMaterial.h"

namespace bs
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	class RendererMaterialBase;
	struct RendererMaterialMetaData;

	/**	Initializes and handles all renderer materials. */
	class BS_EXPORT RendererMaterialManager : public Module<RendererMaterialManager>
	{
		/**	Information used for initializing a renderer material managed by this module. */	
		struct RendererMaterialData
		{
			RendererMaterialMetaData* metaData;
			Path shaderPath;
			Path resourcePath;
		};

	public:
		RendererMaterialManager();
		~RendererMaterialManager();

		/**	Registers a new material that should be initialized on module start-up. */
		static void _registerMaterial(RendererMaterialMetaData* metaData, const Path& shaderPath);

		/** Returns all available variations (specified by pre-processor defines) of a shader at the provided path. */
		static Vector<ShaderDefines> _getVariations(const Path& shaderPath);

		/** Generates a new unique shader path for a specific shader variation. */
		static Path _getVariationPath(const Path& shaderPath, UINT32 variationIdx);

	private:
		template<class T>
		friend class RendererMaterial;

		/**	Initializes all materials on the core thread. */
		static void initOnCore(const Vector<SPtr<ShaderCore>>& shaders);

		/**	Destroys all materials on the core thread. */
		static void destroyOnCore();

		/**	Returns a list in which are all materials managed by this module. */
		static Vector<RendererMaterialData>& getMaterials();

		/**	Returns a mutex used for inter-thread access to the materials list. */
		static Mutex& getMutex();
	};

	/** @} */
}