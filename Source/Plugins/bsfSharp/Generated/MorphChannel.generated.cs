//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// A collection of morph shapes that are sequentially blended together. Each shape has a weight in range [0, 1] which 
	/// determines at what point is that shape blended. As the channel percent moves from 0 to 1, different shapes will be 
	/// blended with those before or after them, depending on their weight.
	/// </summary>
	[ShowInInspector]
	public partial class MorphChannel : ScriptObject
	{
		private MorphChannel(bool __dummy0) { }
		protected MorphChannel() { }

		/// <summary>Returns the unique name of the channel.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public string Name
		{
			get { return Internal_getName(mCachedPtr); }
		}

		/// <summary>Returns all morph shapes within this channel, in order from lowest to highest.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public MorphShape[] Shapes
		{
			get { return Internal_getShapes(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MorphShape[] Internal_getShapes(IntPtr thisPtr);
	}

	/** @} */
}
