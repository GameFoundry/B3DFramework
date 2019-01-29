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
	/// Name and weight of a single shape in a morph target animation. Each shape internally represents a set of vertices that 
	/// describe the morph shape.
	/// </summary>
	[ShowInInspector]
	public partial class MorphShape : ScriptObject
	{
		private MorphShape(bool __dummy0) { }
		protected MorphShape() { }

		/// <summary>Returns the name of the shape.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public string Name
		{
			get { return Internal_getName(mCachedPtr); }
		}

		/// <summary>Returns the weight of the shape, determining how are different shapes within a channel blended.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Weight
		{
			get { return Internal_getWeight(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getWeight(IntPtr thisPtr);
	}

	/** @} */
}
