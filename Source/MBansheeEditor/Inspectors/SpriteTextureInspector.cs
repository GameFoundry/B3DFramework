//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="SpriteTexture"/> resource.
    /// </summary>
    [CustomInspector(typeof(SpriteTexture))]
    internal class SpriteTextureInspector : Inspector
    {
        private GUITextureField textureField = new GUITextureField(new LocEdString("Atlas"));
        private GUIVector2Field offsetField = new GUIVector2Field(new LocEdString("Offset"));
        private GUIVector2Field scaleField = new GUIVector2Field(new LocEdString("Scale"));

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            SpriteTexture spriteTexture = InspectedObject as SpriteTexture;
            if (spriteTexture == null)
                return;

            textureField.OnChanged += (x) =>
            {
                Texture texture = Resources.Load<Texture>(x);
                spriteTexture.Texture = texture;
                EditorApplication.SetDirty(spriteTexture);
            };

            offsetField.OnChanged += (x) =>
            {
                spriteTexture.Offset = x;
                EditorApplication.SetDirty(spriteTexture);
            };

            scaleField.OnChanged += (x) =>
            {
                spriteTexture.Scale = x;
                EditorApplication.SetDirty(spriteTexture);
            };

            Layout.AddElement(textureField);
            Layout.AddElement(offsetField);
            Layout.AddElement(scaleField);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            SpriteTexture spriteTexture = InspectedObject as SpriteTexture;
            if (spriteTexture == null)
                return InspectableState.NotModified;

            textureField.Value = spriteTexture.Texture;
            offsetField.Value = spriteTexture.Offset;
            scaleField.Value = spriteTexture.Scale;

            return InspectableState.NotModified;
        }
    }

    /** @} */
}