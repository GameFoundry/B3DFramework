//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEditor
{
    /** @addtogroup GUI-Editor 
     *  @{
     */

    /// <summary>
    /// Contains various editor specific GUI style names. You may use these to customize look and feel of various GUI 
    /// elements.
    /// </summary>
    public static class EditorStyles
    {
        public const int DefaultFontSize = 8;
        public const string Blank = "Blank";
        public const string Label = "Label";
        public const string LabelCentered = "LabelCentered";
        public const string TitleLabel = "TitleLabel";
        public const string MultiLineLabel = "MultiLineLabel";
        public const string MultiLineLabelCentered = "MultiLineLabelCentered";
        public const string Button = "Button";
        public const string ButtonLeft = "ButtonLeft";
        public const string ButtonRight = "ButtonRight";
        public const string Toggle = "Toggle";
        public const string InputBox = "InputBox";
        public const string Foldout = "Foldout";
        public const string Expand = "Expand";
        public const string Separator = "Separator";
        public const string Header = "Header";
        public const string HeaderBackground = "HeaderBackground";
    }

    /// <summary>
    /// Editor GUI styles similar to <see cref="EditorStyles"/> but with a more specific use.
    /// </summary>
    internal static class EditorStylesInternal
    {
        public const string ColorSliderHorz = "ColorSliderHorz";
        public const string ColorSliderVert = "ColorSliderVert";
        public const string ColorSlider2DHandle = "ColorSlider2DHandle";
        public const string SelectionArea = "SelectionArea";
        public const string SelectableLabel = "SelectableLabel";
        public const string ScrollAreaBg = "ScrollAreaBg";
        public const string InspectorTitleBg = "InspectorTitleBg";
        public const string InspectorContentBg = "InspectorContentBg";
        public const string InspectorContentBgAlternate = "InspectorContentBgAlternate";
    }

    /** @} */
}
