//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsCoreApplication.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/** @addtogroup Application-Engine
	 *  @{
	 */

	/**	Types of available render systems. */
	enum class RenderAPIPlugin
	{
		DX11,
		DX9,
		OpenGL
	};

	/**	Types of available renderers. */
	enum class RendererPlugin
	{
		Default
	};

	/**	Types of available audio systems. */
	enum class AudioPlugin
	{
		OpenAudio, /**< Open-source audio implementation using OpenAL. */
		FMOD /**< Audio system implementation using FMOD. */
	};

	/**	Primary entry point for Banshee engine. Handles startup and shutdown. */
	class BS_EXPORT Application : public CoreApplication
	{
	public:
		Application(RENDER_WINDOW_DESC primaryWindowDesc, RenderAPIPlugin renderAPI, RendererPlugin renderer, 
			AudioPlugin audio, const Vector<String>& importers);
		virtual ~Application();

		/**
		 * Starts the Banshee engine.
		 * 
		 * @param[in]	primaryWindowDesc	Description of the primary render window that will be created on startup.
		 * @param[in]	renderAPI			Render API plugin to use.
		 * @param[in]	renderer			Renderer plugin to use.
		 * @param[in]	audio				Audio plugin to use.
		 * @param[in]	importers			A list of importer plugins to load on startup.
		 */
		static void startUp(RENDER_WINDOW_DESC& primaryWindowDesc, RenderAPIPlugin renderAPI, 
			RendererPlugin renderer = RendererPlugin::Default, AudioPlugin audio = AudioPlugin::OpenAudio, 
			const Vector<String>& importers = Vector<String>());

		/**	Returns the absolute path to the builtin managed engine assembly file. */
		Path getEngineAssemblyPath() const;

		/**	Returns the absolute path to the game managed assembly file. */
		Path getGameAssemblyPath() const;

		/**	Returns the absolute path to the folder where script assemblies are located in. */
		virtual Path getScriptAssemblyFolder() const;

	protected:
		/** @copydoc Module::onStartUp */
		virtual void onStartUp() override;

		/** @copydoc Module::onShutDown */
		virtual void onShutDown() override;

		/** @copydoc CoreApplication::preUpdate */
		virtual void preUpdate() override;

		/** @copydoc CoreApplication::postUpdate */
		virtual void postUpdate() override;

		/** @copydoc CoreApplication::startUpRenderer */
		virtual void startUpRenderer() override;

		/** @copydoc CoreApplication::getShaderIncludeHandler */
		SPtr<IShaderIncludeHandler> getShaderIncludeHandler() const override;

		/**	Loads the script system and all script libraries. */
		virtual void loadScriptSystem();

		/**	Unloads script libraries and shuts down the script system. */
		virtual void unloadScriptSystem();

		/**	Returns the absolute path to the folder where built-in assemblies are located in. */
		virtual Path getBuiltinAssemblyFolder() const;

		/**	Translates render system type into library name. */
		static String getLibNameForRenderAPI(RenderAPIPlugin plugin);

		/**	Translates renderer type into library name. */
		static String getLibNameForRenderer(RendererPlugin plugin);

		/**	Translates audio system type into library name. */
		static String getLibNameForAudio(AudioPlugin plugin);

		DynLib* mMonoPlugin;
		DynLib* mSBansheeEnginePlugin;
	};

	/** Easy way to access Application. */
	BS_EXPORT Application& gApplication();

	/** @} */
}