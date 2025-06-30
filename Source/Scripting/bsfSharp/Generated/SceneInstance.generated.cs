//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Scene
	 *  @{
	 */

	/// <summary>Contains information about an instantiated scene.</summary>
	[ShowInInspector]
	public partial class SceneInstance : ScriptObject
	{
		private SceneInstance(bool __dummy0) { }
		protected SceneInstance() { }

		/// <summary>Creates a new empty scene instance.</summary>
		public SceneInstance(string name)
		{
			Internal_Create(this, name);
		}

		/// <summary>Creates a new scene instance with an existing hierarchy.</summary>
		public SceneInstance(string name, SceneObject root)
		{
			Internal_Create0(this, name, root);
		}

		/// <summary>Name of the scene.</summary>
		[NativeWrapper]
		public string Name
		{
			get { return Internal_GetName(mCachedPtr); }
		}

		/// <summary>Root object of the scene.</summary>
		[NativeWrapper]
		public SceneObject Root
		{
			get { return Internal_GetRoot(mCachedPtr); }
		}

		/// <summary>
		/// Checks is the scene currently active. IF inactive the scene properties aside from the name are undefined.
		/// </summary>
		[NativeWrapper]
		public bool IsActive
		{
			get { return Internal_IsActive(mCachedPtr); }
		}

		/// <summary>
		/// Representation of the scene used by the physics sub-system. Contains all the objects that can be physically 
		/// interacted with. Exact implementation depends on the physics plugin used.
		/// </summary>
		[NativeWrapper]
		public PhysicsScene Physics
		{
			get { return Internal_GetPhysicsScene(mCachedPtr); }
		}

		/// <summary>
		/// Returns the ID of the resource that the scene instance is associated with (e.g. resource the scene was loaded from.).
		/// </summary>
		[NativeWrapper]
		public UUID AssociatedResourceId
		{
			get
			{
				UUID temp;
				Internal_GetAssociatedResourceId(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the main camera component. See GetMainCamera().</summary>
		[NativeWrapper]
		public Camera MainCamera
		{
			get { return Internal_GetMainCameraComponent(mCachedPtr); }
		}

		/// <summary>Editor scene instance, if running from within the editor.</summary>
		[NativeWrapper]
		public IEditorSceneInstance EditorSceneInstance
		{
			get { return Internal_GetEditorSceneInstance(mCachedPtr); }
		}

		/// <summary>Checks are the components currently in the Running state.</summary>
		[NativeWrapper]
		public bool IsRunning
		{
			get { return Internal_IsRunning(mCachedPtr); }
		}

		/// <summary>
		/// Removes all scene objects from the scene, except for persistent objects. If <paramref name="forceAll"/> is true, 
		/// removes even the persistent objects.
		/// </summary>
		public void Clear(bool forceAll = false)
		{
			Internal_Clear(mCachedPtr, forceAll);
		}

		/// <summary>Creates a new scene object in the scene instance.</summary>
		/// <param name="name">Name of the scene object.</param>
		/// <param name="flags">Optional flags that control object behavior. See SceneObjectFlags.</param>
		public SceneObject CreateSceneObject(string name, int flags = 0)
		{
			return Internal_CreateSceneObject(mCachedPtr, name, flags);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject Internal_GetRoot(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsActive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsScene Internal_GetPhysicsScene(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAssociatedResourceId(IntPtr thisPtr, out UUID __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Camera Internal_GetMainCameraComponent(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern IEditorSceneInstance Internal_GetEditorSceneInstance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Clear(IntPtr thisPtr, bool forceAll);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject Internal_CreateSceneObject(IntPtr thisPtr, string name, int flags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsRunning(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(SceneInstance managedInstance, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(SceneInstance managedInstance, string name, SceneObject root);
	}

	/** @} */
}
