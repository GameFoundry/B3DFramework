//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

		/// <summary>Name of the scene.</summary>
		[NativeWrapper]
		public string Name
		{
			get { return Internal_getName(mCachedPtr); }
		}

		/// <summary>Root object of the scene.</summary>
		[NativeWrapper]
		public SceneObject Root
		{
			get { return Internal_getRoot(mCachedPtr); }
		}

		/// <summary>
		/// Checks is the scene currently active. IF inactive the scene properties aside from the name are undefined.
		/// </summary>
		[NativeWrapper]
		public bool IsActive
		{
			get { return Internal_isActive(mCachedPtr); }
		}

		/// <summary>
		/// Physical representation of the scene, as assigned by the physics sub-system. Exact implementation depends on the  
		/// physics plugin used.
		/// </summary>
		[NativeWrapper]
		public PhysicsScene Physics
		{
			get { return Internal_getPhysicsScene(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject Internal_getRoot(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isActive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsScene Internal_getPhysicsScene(IntPtr thisPtr);
	}

	/** @} */
}
