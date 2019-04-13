//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Base class for all <see cref="AsyncOp{T}"/> specializations.
    /// </summary>
    public class AsyncOpBase : ScriptObject
    {
        protected AsyncOpBase() { }

        /// <summary>Returns true if the async operation has completed.</summary>
        public bool IsComplete
        {
            get { return Internal_IsComplete(mCachedPtr); }
        }

        /// <summary>
        /// Retrieves the value returned by the async operation. Only valid if <see cref="IsComplete"/> returns true.
        /// </summary>
        public object GenericValue
        {
            get { return Internal_GetValue(mCachedPtr); }
        }

        /// <summary>Blocks the caller thread until the AsyncOp completes.</summary>
        public void BlockUntilComplete()
        {
            Internal_BlockUntilComplete(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsComplete(IntPtr thisPtr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_GetValue(IntPtr thisPtr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_BlockUntilComplete(IntPtr thisPtr);
    }

    /// <summary>
    /// Type you may use to check on the results of an asynchronous operation. Contains uninitialized data until
    /// <see cref="AsyncOpBase.IsComplete"/> returns true.
    /// </summary>
    public class AsyncOp<T> : AsyncOpBase where T: class
    {
        /// <summary>
        /// Retrieves the value returned by the async operation. Only valid if <see cref="IsCompleted"/> returns true.
        /// </summary>
        /// <typeparam name="T">Type of the return value. Caller must ensure to provide the valid type.</typeparam>
        /// <returns>Return value of the async operation.</returns>
        public T Value
        {
            get { return GenericValue as T; }
        }
    }

    /** @} */
}
