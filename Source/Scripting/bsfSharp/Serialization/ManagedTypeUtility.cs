//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup Serialization
     *  @{
     */

    public partial class ManagedTypeUtility
    {
        /// <summary>
        /// Creates an empty instance of the specified type.
        /// </summary>
        /// <typeparam name="T">Type of the object to create. Must be serializable.</typeparam>
        /// <returns>New instance of the specified type, or null if the type is not serializable.</returns>
        public static T CreateObjectOfType<T>()
        {
            return (T)Internal_CreateObjectOfType(typeof(T));
        }
    }

    /** @} */
}
