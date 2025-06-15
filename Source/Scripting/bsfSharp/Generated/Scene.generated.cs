//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Localization
	 *  @{
	 */

	/// <summary>Saveable hierarchy of scene objects that can be instantiated into a SceneInstance.</summary>
	[ShowInInspector]
	public partial class Scene : Resource
	{
		private Scene(bool __dummy0) { }
		protected Scene() { }

		/// <summary>
		/// Creates a new scene from the provided scene object. The provided scene object and all saveable children will be 
		/// cloned as part of the scene.
		/// </summary>
		public Scene(SceneObject sceneObject)
		{
			Internal_Create(this, sceneObject);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Scene> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Scene>(Scene x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		/// <summary>Instantiates a scene by creating an instance of the scene object hierarchy.</summary>
		public SceneInstance Instantiate()
		{
			return Internal_Instantiate(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Scene> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneInstance Internal_Instantiate(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(Scene managedInstance, SceneObject sceneObject);
	}

	/** @} */
}
