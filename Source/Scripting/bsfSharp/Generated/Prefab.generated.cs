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

	/// <summary>
	/// Prefab is a saveable hierarchy of scene objects. It can be instanced, and instances will maintain a link to the 
	/// original prefab they were created from, allowing you to update them to latest version if the prefab changes. Prefabs 
	/// can also be nested within each-other, as long as there are no circular dependencies. Instanced prefabs can also 
	/// contain per instance modifications that will be preserved even if the prefab they were created from changes.
	/// </summary>
	[ShowInInspector]
	public partial class Prefab : Resource
	{
		private Prefab(bool __dummy0) { }
		protected Prefab() { }

		/// <summary>
		/// Creates a new prefab from the provided scene object. If the scene object has an existing prefab link it will be 
		/// broken. After the prefab is created the scene object will be automatically linked to it.
		/// </summary>
		public Prefab(SceneObject sceneObject)
		{
			Internal_Create(this, sceneObject);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Prefab> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Prefab>(Prefab x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		/// <summary>
		/// Instantiates a prefab by creating an instance of the prefab&apos;s scene object hierarchy. The returned hierarchy 
		/// will be parented to the provided scene instance root.
		/// </summary>
		/// <param name="sceneInstance">Scene instance into which to instantiate the prefab instance in.</param>
		/// <returns>Instantiated clone of the prefab&apos;s scene object hierarchy.</returns>
		public SceneObject Instantiate(SceneInstance sceneInstance)
		{
			return Internal_Instantiate(mCachedPtr, sceneInstance);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Prefab> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject Internal_Instantiate(IntPtr thisPtr, SceneInstance sceneInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(Prefab managedInstance, SceneObject sceneObject);
	}

	/** @} */
}
