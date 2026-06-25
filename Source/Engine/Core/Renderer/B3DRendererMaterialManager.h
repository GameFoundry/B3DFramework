//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Material/B3DShaderVariation.h"

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
		/** A shader used by one or more registered renderer materials, together with the defines it is compiled with. */
		struct RendererMaterialShaderInfo
		{
			Path ShaderPath;
			ShaderDefines Defines;
		};

		RendererMaterialManager();
		~RendererMaterialManager();

		/**	Registers a new material that should be initialized on module start-up. */
		static void RegisterMaterial(render::RendererMaterialMetaData* metaData, const char* shaderPath);

		/**
		 * Enumerates the shaders used by all registered renderer materials, appending one entry per registered material
		 * to @p output. A shader shared by several materials (potentially with different defines) yields one entry per
		 * material. Safe to call without a render device, so the offline shader cook tool can determine which builtin
		 * shaders are renderer materials, and with which defines and cache prefix they must be compiled.
		 */
		static void GetRegisteredMaterialShaders(Vector<RendererMaterialShaderInfo>& output);

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
