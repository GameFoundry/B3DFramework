//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup VectorGraphics
	 *  @{
	 */

	/// <summary>
	/// Represents a vector path containing curves and geometric shapes that can be rasterized to any dimension.
	/// </summary>
	[ShowInInspector]
	public partial class VectorPath : Resource
	{
		private VectorPath(bool __dummy0) { }
		protected VectorPath() { }

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<VectorPath> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<VectorPath>(VectorPath x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<VectorPath> Internal_GetRef(IntPtr thisPtr);
	}

	/** @} */
}
