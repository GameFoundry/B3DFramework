//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>
	/// Keeps track of all active SceneObject%s and their components. Keeps track of component state and triggers their 
	/// events. Updates the transforms of objects as SceneObject%s move.
	/// </summary>
	[ShowInInspector]
	public partial class SceneManager : ScriptObject
	{
		private SceneManager(bool __dummy0) { }
		protected SceneManager() { }

		/// <summary>Returns the object that represents the main scene.</summary>
		[NativeWrapper]
		public static SceneInstance MainScene
		{
			get { return Internal_GetMainScene(); }
		}

		/// <summary>Called when a new main scene has been loaded and is active.</summary>
		public static event Action<UUID> OnMainSceneLoaded;

		/// <summary>Called when the main scene has been cleared or unloaded.</summary>
		public static event Action<UUID> OnMainSceneUnloaded;

		/// <summary>
		/// Instantiates a new scene and makes it active. All non-persistent objects that are part of the current scene will be 
		/// destroyed.
		/// </summary>
		public static void LoadMainScene(Prefab scene)
		{
			Internal_LoadMainScene(scene);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneInstance Internal_GetMainScene();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClearMainScene(bool forceAll);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_LoadMainScene(Prefab scene);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject Internal_GetMainCameraSceneObject();
		private static void Internal_OnMainSceneLoaded(ref UUID p0)
		{
			OnMainSceneLoaded?.Invoke(p0);
		}
		private static void Internal_OnMainSceneUnloaded(ref UUID p0)
		{
			OnMainSceneUnloaded?.Invoke(p0);
		}
	}
}
