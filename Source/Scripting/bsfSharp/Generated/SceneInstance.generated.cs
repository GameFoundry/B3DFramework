//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
		/// Physical representation of the scene, as assigned by the physics sub-system. Exact implementation depends on the 
		/// physics plugin used.
		/// </summary>
		[NativeWrapper]
		public PhysicsScene Physics
		{
			get { return Internal_GetPhysicsScene(mCachedPtr); }
		}

		/// <summary>Creates a new scene object in the scene instance.</summary>
		public SceneObject CreateSceneObject(string name)
		{
			return Internal_CreateSceneObject(mCachedPtr, name);
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
		private static extern SceneObject Internal_CreateSceneObject(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(SceneInstance managedInstance, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(SceneInstance managedInstance, string name, SceneObject root);
	}

	/** @} */
}
