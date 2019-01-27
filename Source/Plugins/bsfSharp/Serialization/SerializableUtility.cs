//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Provides utility methods dealing with object serialization.
    /// </summary>
    public static class SerializableUtility
    {
        /// <summary>
        /// Clones the specified object. Non-serializable types and fields are ignored in clone. A deep copy is performed
        /// on all serializable elements except for resources or game objects.
        /// </summary>
        /// <param name="original">Non-null reference to the object to clone. Object type must be serializable.</param>
        /// <returns>Deep copy of the original object.</returns>
        public static object Clone(object original)
        {
            return Internal_Clone(original);
        }

        /// <summary>
        /// Creates an empty instance of the specified type. 
        /// </summary>
        /// <typeparam name="T">Type of the object to create. Must be serializable.</typeparam>
        /// <returns>New instance of the specified type, or null if the type is not serializable.</returns>
        public static T Create<T>()
        {
            return (T)Internal_Create(typeof(T));
        }

        /// <summary>
        /// Creates an empty instance of the specified type. 
        /// </summary>
        /// <param name="type">Type of the object to create. Must be serializable.</param>
        /// <returns>New instance of the specified type, or null if the type is not serializable.</returns>
        public static object Create(Type type)
        {
            return Internal_Create(type);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_Clone(object original);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_Create(Type type);
    }

    /** @} */
}
