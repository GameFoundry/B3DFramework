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
    /// Renders an inspector for the <see cref="PlainText"/> resource.
    /// </summary>
    [CustomInspector(typeof(PlainText))]
    internal class PlainTextInspector : Inspector
    {
        private const int MAX_SHOWN_CHARACTERS = 3000;

        private GUILabel textLabel = new GUILabel("", EditorStyles.MultiLineLabel, GUIOption.FixedHeight(500));
        private GUITexture textBg = new GUITexture(null, EditorStylesInternal.ScrollAreaBg);

        private string shownText = "";

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            PlainText plainText = InspectedObject as PlainText;
            if (plainText == null)
                return;

            GUIPanel textPanel = Layout.AddPanel();
            GUILayout textLayoutY = textPanel.AddLayoutY();
            textLayoutY.AddSpace(5);
            GUILayout textLayoutX = textLayoutY.AddLayoutX();
            textLayoutX.AddSpace(5);
            textLayoutX.AddElement(textLabel);
            textLayoutX.AddSpace(5);
            textLayoutY.AddSpace(5);

            GUIPanel textBgPanel = textPanel.AddPanel(1);
            textBgPanel.AddElement(textBg);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            PlainText plainText = InspectedObject as PlainText;
            if (plainText == null)
                return InspectableState.NotModified;

            string newText = plainText.Text;
            string newShownText = plainText.Text.Substring(0, MathEx.Min(newText.Length, MAX_SHOWN_CHARACTERS));

            if (newShownText != shownText)
            {
                textLabel.SetContent(newShownText);
                shownText = newShownText;
            }

            return InspectableState.NotModified;
        }
    }

    /** @} */
}