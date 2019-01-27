//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */

    public partial class AsyncOp
    {
        /// <summary>
        /// Retrieves the value returned by the async operation. Only valid if <see cref="IsCompleted"/> returns true.
        /// </summary>
        /// <typeparam name="T">Type of the return value. Caller must ensure to provide the valid type.</typeparam>
        /// <returns>Return value of the async operation.</returns>
        public T GetReturnValue<T>()
        {
            return (T)Internal_getReturnValue(mCachedPtr);
        }
    }

    /** @} */
}
