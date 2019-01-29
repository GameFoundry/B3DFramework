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
	/// <summary>
	/// Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until  IsComplete 
	/// returns true.
	/// </summary>
	[ShowInInspector]
	public partial class AsyncOp : ScriptObject
	{
		private AsyncOp(bool __dummy0) { }
		protected AsyncOp() { }

		/// <summary>Returns true if the async operation has completed.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool IsComplete
		{
			get { return Internal_isComplete(mCachedPtr); }
		}

		/// <summary>Retrieves the value returned by the async operation. Only valid if IsComplete returns true.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public object ReturnValue
		{
			get { return Internal_getReturnValue(mCachedPtr); }
		}

		/// <summary>Blocks the caller thread until the AsyncOp completes.</summary>
		public void BlockUntilComplete()
		{
			Internal_blockUntilComplete(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isComplete(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_getReturnValue(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_blockUntilComplete(IntPtr thisPtr);
	}
}
