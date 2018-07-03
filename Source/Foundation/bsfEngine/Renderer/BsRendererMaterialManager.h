//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	namespace ct
	{
		class RendererMaterialBase;
		struct RendererMaterialMetaData;
	}

	/**	Initializes and handles all renderer materials. */
	class BS_EXPORT RendererMaterialManager : public Module<RendererMaterialManager>
	{
		/**	Information used for initializing a renderer material managed by this module. */	
		struct RendererMaterialData
		{
			ct::RendererMaterialMetaData* metaData;
			const Path shaderPath;
		};

	public:
		RendererMaterialManager();
		~RendererMaterialManager();

		/**	Registers a new material that should be initialized on module start-up. */
		static void _registerMaterial(ct::RendererMaterialMetaData* metaData, const char* shaderPath);

		/** Returns a set of defines to be used when importing the shader. */
		static ShaderDefines _getDefines(const Path& shaderPath);
	private:
		template<class T>
		friend class RendererMaterial;
		friend class ct::RendererMaterialBase;

		/**	Initializes all materials on the core thread. */
		static void initOnCore(const Vector<SPtr<ct::Shader>>& shaders);

		/**	Destroys all materials on the core thread. */
		static void destroyOnCore();

		/**	Returns a list in which are all materials managed by this module. */
		static Vector<RendererMaterialData>& getMaterials();

		/**	Returns a mutex used for inter-thread access to the materials list. */
		static Mutex& getMutex();
	};

	/** @} */
}