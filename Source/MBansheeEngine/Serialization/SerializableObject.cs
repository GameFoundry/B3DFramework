//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using System.Text;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    #pragma warning disable 649

    /// <summary>
    /// Allows you to access meta-data about a managed object and its fields. Similar to Reflection but simpler and faster.
    /// </summary>
    public sealed class SerializableObject : ScriptObject
    {
        internal SerializableProperty parentProperty;
        internal object parentObject;
        private SerializableField[] _fields;

        /// <summary>
        /// Creates a new serializable object for the specified object type.
        /// </summary>
        /// <param name="objectType">C# type of the object.</param>
        /// <param name="parentProperty">Property used for retrieving this entry.</param>
        public SerializableObject(Type objectType, SerializableProperty parentProperty)
        {
            Internal_CreateInstance(this, objectType);

            this.parentProperty = parentProperty;
            this.parentObject = null;
        }

        /// <summary>
        /// Creates a new serializable object for the specified object type.
        /// </summary>
        /// <param name="objectType">C# type of the object.</param>
        /// <param name="parentObject">Specific instance of the object of <paramref name="objectType"/>.</param>
        public SerializableObject(Type objectType, object parentObject)
        {
            Internal_CreateInstance(this, objectType);

            this.parentProperty = null;
            this.parentObject = parentObject;
        }

        /// <summary>
        /// Creates a new serializable object for the specified object. Object must not be null.
        /// </summary>
        /// <param name="parentObject">Specific instance of the object.</param>
        public SerializableObject(object parentObject)
        {
            Internal_CreateInstance(this, parentObject.GetType());

            this.parentProperty = null;
            this.parentObject = parentObject;
        }

        /// <summary>
        /// Returns all fields in the object.
        /// </summary>
        public SerializableField[] Fields
        {
            get { return _fields; }
        }

        /// <summary>
        /// Returns the specific object instance this object is operating on.
        /// </summary>
        /// <returns>Object instance.</returns>
        public object GetReferencedObject()
        {
            if (parentProperty != null)
                return parentProperty.GetValue<object>();
            else
                return parentObject;
        }

        /// <summary>
        /// Searches the object, and all child objects for a field or entry with the specified name.
        /// </summary>
        /// <param name="path">Slash separated path with field name(s) to search for. If a field contains an array, list or
        ///                    a dictionary append its name with a "[x]" where x is the element index in the array/list, or
        ///                    a key name (surrounded by "") in case of a dictionary. Only primitive dictionary keys are
        ///                    supported.
        /// 
        ///                    Example path: subObject/myDictionary["someElement"]/theArray[4]/fieldToGet
        ///                    </param>
        /// <returns>Property you can use for reading or modifying the property, or null if not found.</returns>
        public SerializableProperty FindProperty(string path)
        {
            if (path == null)
                return null;

            string trimmedPath = path.Trim('/');
            string[] pathEntries = trimmedPath.Split('/');
            PropertyPathElement[] pathElements = new PropertyPathElement[pathEntries.Length];
            for (int i = 0; i < pathEntries.Length; i++)
            {
                string entry = pathEntries[i];
                if (string.IsNullOrEmpty(entry))
                    return null;

                pathElements[i] = new PropertyPathElement();
                if (entry[entry.Length - 1] == ']')
                {
                    bool foundKey = false;
                    int j = entry.Length - 2;
                    for (; j >= 0; j--)
                    {
                        if (entry[j] == '[')
                        {
                            foundKey = true;
                            break;
                        }
                    }

                    if (foundKey)
                    {
                        pathElements[i].name = entry.Substring(0, j);
                        pathElements[i].key = entry.Substring(j + 1, (entry.Length - 1) - (j + 1));
                    }
                    else
                        pathElements[i].name = entry;
                }
                else
                {
                    pathElements[i].name = entry;
                }
            }

            return FindProperty(pathElements, 0);
        }

        /// <summary>
        /// Searches the object hierarchy using the provided path elements. <see cref="FindProperty(string)"/>
        /// </summary>
        /// <param name="pathElements">Path elements representing field names and keys to look for.</param>
        /// <param name="elementIdx">Index in the <paramref name="pathElements"/> array to start the search at.</param>
        /// <returns>Property representing the final path element, or null if not found.</returns>
        internal SerializableProperty FindProperty(PropertyPathElement[] pathElements, int elementIdx)
        {
            SerializableField field = FindField(pathElements[0].name);
            if (field != null)
            {
                SerializableProperty property = field.GetProperty();
                if (elementIdx == (pathElements.Length - 1))
                    return property;

                return property.FindProperty(pathElements, elementIdx + 1);
            }

            return null;
        }

        /// <summary>
        /// Searches the object for a field with the specified name.
        /// </summary>
        /// <param name="name">Name of the field.</param>
        /// <returns>Object representing the field, if found, null otherwise.</returns>
        public SerializableField FindField(string name)
        {
            foreach (var field in _fields)
            {
                if (field.Name == name)
                    return field;
            }

            return null;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(SerializableObject instance, Type objectType);
    }

    /// <summary>
    /// Contains a single element of a path to a field or array/list/dictionary entry, as used for 
    /// <see cref="SerializableObject"/>.
    /// </summary>
    internal struct PropertyPathElement
    {
        public string name;
        public string key;
    }

    /** @} */
}
