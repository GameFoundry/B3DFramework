//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Component that maps animation for specific bone also be applied to the SceneObject this component is attached to.  The 
	/// component will attach to the first found parent Animation component.
	/// </summary>
	[ShowInInspector]
	public partial class Bone : Component
	{
		private Bone(bool __dummy0) { }
		protected Bone() { }

		/// <summary>Determines the name of the bone the component is referencing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public string Name
		{
			get { return Internal_getBoneName(mCachedPtr); }
			set { Internal_setBoneName(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setBoneName(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getBoneName(IntPtr thisPtr);
	}

	/** @} */
}
