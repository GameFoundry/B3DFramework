//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    #pragma warning disable 649

    /// <summary>
    /// Allows you to access meta-data about a managed array and its children. Similar to Reflection but simpler and faster.
    /// </summary>
    public sealed class SerializableArray : ScriptObject
    {
        private SerializableProperty.FieldType elementPropertyType;
        private Type elementType;
        private SerializableProperty parentProperty;

        /// <summary>
        /// Type of serializable property used for the elements stored in the array.
        /// </summary>
        public SerializableProperty.FieldType ElementPropertyType
        {
            get { return elementPropertyType; }
        }

        /// <summary>
        /// Type used for the elements stored in the array.
        /// </summary>
        public Type ElementType
        {
            get { return elementType; }
        }

        /// <summary>
        /// Constructor for use by the runtime only.
        /// </summary>
        /// <param name="elementType">C# type of the elements in the array.</param>
        /// <param name="parentProperty">Property used for retrieving this entry.</param>
        private SerializableArray(Type elementType, SerializableProperty parentProperty)
        {
            this.parentProperty = parentProperty;
            this.elementType = elementType;
            elementPropertyType = SerializableProperty.DetermineFieldType(elementType);
        }

        /// <summary>
        /// Returns a serializable property for a specific array element.
        /// </summary>
        /// <param name="elementIdx">Index of the element in the array.</param>
        /// <returns>Serializable property that allows you to manipulate contents of the array entry.</returns>
        public SerializableProperty GetProperty(int elementIdx)
        {
            SerializableProperty.Getter getter = () =>
            {
                Array array = parentProperty.GetValue<Array>();

                if (array != null)
                    return array.GetValue(elementIdx);
                else
                    return null;
            };

            SerializableProperty.Setter setter = (object value) =>
            {
                Array array = parentProperty.GetValue<Array>();

                if(array != null)
                    array.SetValue(value, elementIdx);
            };

            SerializableProperty property = Internal_CreateProperty(mCachedPtr);
            property.Construct(ElementPropertyType, elementType, getter, setter);

            return property;
        }

        /// <summary>
        /// Returns number of elements in the array.
        /// </summary>
        /// <returns>Number of elements in the array.</returns>
        public int GetLength()
        {
            Array array = parentProperty.GetValue<Array>();

            if (array != null)
                return array.GetLength(0); // TODO - Support multi-rank arrays
            else
                return 0;
        }

        /// <summary>
        /// Uses the provided path elements to find an array element at the specified index, and returns a property
        /// to the element, or to a child property of that element.
        /// </summary>
        /// <param name="pathElements">Path elements representing field names and keys to look for.</param>
        /// <param name="elementIdx">Index in the <paramref name="pathElements"/> array to start the search at.</param>
        /// <returns>Property representing the final path element, or null if not found (array index is out of range, or 
        ///          property with that path doesn't exist).</returns>
        internal SerializableProperty FindProperty(PropertyPathElement[] pathElements, int elementIdx)
        {
            int arrayIdx;
            if (string.IsNullOrEmpty(pathElements[elementIdx].key))
                arrayIdx = 0;
            else
            {
                if (!int.TryParse(pathElements[elementIdx].key, out arrayIdx))
                    return null;
                else
                {
                    if (arrayIdx < 0 || arrayIdx >= GetLength())
                        return null;
                }
            }

            SerializableProperty property = GetProperty(arrayIdx);
            if (elementIdx == (pathElements.Length - 1))
                return property;

            return property.FindProperty(pathElements, elementIdx + 1);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializableProperty Internal_CreateProperty(IntPtr nativeInstance);
    }

    /** @} */
}
