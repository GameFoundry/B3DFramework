//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace render
	{
		class RendererMaterialBase;
		struct RendererMaterialMetaData;
	} // namespace render

	/**	Initializes and handles all renderer materials. */
	class B3D_EXPORT RendererMaterialManager : public Module<RendererMaterialManager>
	{
		/**	Information used for initializing a renderer material managed by this module. */
		struct RendererMaterialData
		{
			render::RendererMaterialMetaData* MetaData;
			const Path ShaderPath;
		};

	public:
		RendererMaterialManager();
		~RendererMaterialManager();

		/**	Registers a new material that should be initialized on module start-up. */
		static void RegisterMaterial(render::RendererMaterialMetaData* metaData, const char* shaderPath);

		/** Returns a set of defines to be used when importing the shader. */
		static ShaderDefines GetDefinesInternal(const Path& shaderPath);

	private:
		template <class T>
		friend class RendererMaterial;
		friend class render::RendererMaterialBase;

		/**	Initializes the manager on the render thread. */
		static void InitOnRenderThread();

		/**	Destroys all materials on the render thread. */
		static void DestroyOnRenderThread();

		/**	Returns a list in which are all materials managed by this module. */
		static Vector<RendererMaterialData>& GetMaterials();

		/**	Returns a mutex used for inter-thread access to the materials list. */
		static Mutex& GetMutex();
	};

	/** @} */
} // namespace b3d
