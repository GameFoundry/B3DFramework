//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Implementation of SpriteImage that renders a vector path.</summary>
	[ShowInInspector]
	public partial class SpriteVectorPath : SpriteImage
	{
		private SpriteVectorPath(bool __dummy0) { }
		protected SpriteVectorPath() { }

		/// <summary>Creates a new sprite vector path.</summary>
		public SpriteVectorPath(RRef<VectorPath> vectorPath, TSize2<int> size)
		{
			Internal_Create(this, vectorPath, ref size);
		}

		/// <summary>Creates a new sprite vector path.</summary>
		public SpriteVectorPath(SpriteVectorPathCreateInformation createInformation)
		{
			Internal_Create0(this, ref createInformation);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<SpriteVectorPath> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<SpriteVectorPath>(SpriteVectorPath x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		/// <summary>Sets the vector path to render.</summary>
		public void SetVectorPath(RRef<VectorPath> vectorPath)
		{
			Internal_SetVectorPath(mCachedPtr, vectorPath);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteVectorPath> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVectorPath(IntPtr thisPtr, RRef<VectorPath> vectorPath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(SpriteVectorPath managedInstance, RRef<VectorPath> vectorPath, ref TSize2<int> size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(SpriteVectorPath managedInstance, ref SpriteVectorPathCreateInformation createInformation);
	}

	/** @} */
}
