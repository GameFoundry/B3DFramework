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
	/// Contains a set of morph channels used for morph target animation. Each morph channel contains one or multiple shapes 
	/// which are blended together depending on frame animation. Each channel is then additively blended together depending on 
	/// some weight.
	/// </summary>
	[ShowInInspector]
	public partial class MorphShapes : ScriptObject
	{
		private MorphShapes(bool __dummy0) { }
		protected MorphShapes() { }

		/// <summary>Returns a list of all morph channels in the morph animation.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public MorphChannel[] Channels
		{
			get { return Internal_getChannels(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MorphChannel[] Internal_getChannels(IntPtr thisPtr);
	}

	/** @} */
}
