//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Utility-Editor
     *  @{
     */

    /// <summary>
    /// Container and functionality to creating a serialized version of an object. The object must be of valid serializable
    /// type (<see cref="Component"/>, <see cref="ManagedResource"/> or a class/struct marked with 
    /// <see cref="SerializeObject"/> attribute).
    /// </summary>
    public class SerializedObject : ScriptObject
    {
        /// <summary>
        /// Constructs a new serialized object. Only for internal runtime use.
        /// </summary>
        private SerializedObject()
        { }

        /// <summary>
        /// Serializes all data within the provided component.
        /// </summary>
        /// <param name="obj">Component to deserialize.</param>
        /// <returns>Object containing serialized data.</returns>
        public static SerializedObject Create(Component obj)
        {
            if (obj == null)
                return null;

            IntPtr componentPtr = obj.GetCachedPtr();
            return Internal_CreateComponent(componentPtr);
        }

        /// <summary>
        /// Serializes all data within the provided resources.
        /// </summary>
        /// <param name="obj">Resource to deserialize.</param>
        /// <returns>Object containing serialized data.</returns>
        public static SerializedObject Create(ManagedResource obj)
        {
            if (obj == null)
                return null;

            IntPtr resourcePtr = obj.GetCachedPtr();
            return Internal_CreateResource(resourcePtr);
        }

        /// <summary>
        /// Serializes all data within the provided object.
        /// </summary>
        /// <param name="obj">Object to deserialize.</param>
        /// <returns>Object containing serialized data.</returns>
        public static SerializedObject Create(object obj)
        {
            if (obj == null)
                return null;

            return Internal_CreateGeneric(obj);
        }

        /// <summary>
        /// Deserializes data stored in this object. Components and resources cannot be deserialized.
        /// </summary>
        /// <typeparam name="T">Type to cast the object to after deserialization.</typeparam>
        /// <returns>Deserialized object if successful, null otherwise.</returns>
        public T Get<T>()
        {
            return (T) Internal_Deserialize(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializedObject Internal_CreateComponent(IntPtr componentPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializedObject Internal_CreateResource(IntPtr resourcePtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializedObject Internal_CreateGeneric(object obj);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_Deserialize(IntPtr instance);
    }

    /** @} */
}
