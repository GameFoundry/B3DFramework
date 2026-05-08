---
title: Fonts
---

Fonts control how text characters are displayed and are used primarily throughout the GUI system. They are represented with the @b3d::Font class and are a **Resource**, meaning they can be imported, saved, and loaded as described in the resource manual.

Fonts can be imported from TTF or OTF formats using the importer.

~~~~~~~~~~~~~{.cpp}
// Import a font from disk
HFont font = GetImporter().Import<Font>("lato.ttf");
~~~~~~~~~~~~~

# Customizing import

Font import can be customized by providing a @b3d::FontImportOptions object to the importer.

~~~~~~~~~~~~~{.cpp}
TShared<FontImportOptions> importOptions = FontImportOptions::Create();
// Set required options here (as described below)

HFont font = GetImporter().Import<Font>("lato.ttf", importOptions);
~~~~~~~~~~~~~

Several properties can be customized on import, including font sizes, font style, DPI, and character ranges.

## Font sizes

Set @b3d::FontImportOptions::FontSizes with a list of font sizes (in points) to pre-render during import. When you use a font at a specific size, if that size was not imported, the nearest available size will be used instead, or glyphs can be rendered dynamically at runtime.

~~~~~~~~~~~~~{.cpp}
// Import font at sizes 11, 12, and 16 points
importOptions->FontSizes = { 11.0f, 12.0f, 16.0f };
~~~~~~~~~~~~~

## Font style

You can control several style options when importing fonts: bold, italic, and rendering mode.

### Bold

When bold font is enabled, font characters will be rendered thicker than normal. Set @b3d::FontImportOptions::Bold to enable or disable this option.

~~~~~~~~~~~~~{.cpp}
importOptions->Bold = true;
~~~~~~~~~~~~~

### Italic

When italic font is enabled, font characters will be rendered with a slight skew. Set @b3d::FontImportOptions::Italic to enable or disable this option.

~~~~~~~~~~~~~{.cpp}
importOptions->Italic = true;
~~~~~~~~~~~~~

### Render mode

Controls whether smoothing is applied to rendered fonts and what kind. See the @b3d::FontRenderMode enumeration for available modes. Set the render mode through @b3d::FontImportOptions::RenderMode.

~~~~~~~~~~~~~{.cpp}
importOptions->RenderMode = FontRenderMode::HintedSmooth;
~~~~~~~~~~~~~

Available render modes include:
- **Smooth**: Antialiased fonts without hinting (slightly more blurry)
- **Raster**: Non-antialiased fonts without hinting
- **HintedSmooth**: Antialiased fonts with hinting (recommended for most cases)
- **HintedRaster**: Non-antialiased fonts with hinting

## DPI

DPI (dots per inch) depends on the resolution of the display device versus its physical size. Devices with more pixels per inch have higher DPI values.

Setting a higher DPI will make your fonts larger in terms of pixels. This ensures the physical size of displayed characters remains consistent across devices with different resolution-to-size ratios.

~~~~~~~~~~~~~{.cpp}
// Set to high DPI used by full HD phone screens
importOptions->Dpi = 450;

// Or DPI used by most full HD monitors
importOptions->Dpi = 96;
~~~~~~~~~~~~~

## Character ranges

When importing a font, you must specify which characters to pre-render. By default, common English letters and other frequently used ASCII characters are imported (Unicode range 33-166). If you require other character sets such as Cyrillic, Japanese, or Korean characters, specify them using @b3d::FontImportOptions::CharIndexRanges.

Character ranges use Unicode character codes. You can find indices for various character sets at the official [Unicode charts](http://www.unicode.org/charts/).

~~~~~~~~~~~~~{.cpp}
// Add Cyrillic characters (Unicode range 0x0400-0x04FF)
importOptions->CharIndexRanges = { CharRange(0x0400, 0x04FF) };

// Import multiple ranges
importOptions->CharIndexRanges = {
    CharRange(33, 166),           // Basic ASCII
    CharRange(0x0400, 0x04FF),    // Cyrillic
    CharRange(0x4E00, 0x9FFF)     // CJK Unified Ideographs
};
~~~~~~~~~~~~~

# Dynamic font rendering

While importing fonts with pre-rendered character bitmaps is efficient for known character sets, the framework also supports dynamic glyph rendering at runtime. This allows you to render any character from the font's source data on-demand.

## Rendering glyphs at runtime

Use @b3d::Font::RenderGlyphs to render specific characters at a given size. This method adds the rendered glyphs to the font's texture atlas, making them available for rendering.

~~~~~~~~~~~~~{.cpp}
HFont font = GetImporter().Import<Font>("lato.ttf");

// Render specific characters at 12 points
float sizeInPoints = 12.0f;
u32 charactersToRender[] = { 'H', 'e', 'l', 'l', 'o' };
font->RenderGlyphs(sizeInPoints, charactersToRender);
~~~~~~~~~~~~~

By default, dynamically rendered glyphs are only kept in memory and will need to be re-rendered if the font is reloaded. You can optionally bake glyphs so they are saved with the font resource:

~~~~~~~~~~~~~{.cpp}
bool bakeGlyphs = true;
font->RenderGlyphs(sizeInPoints, charactersToRender, bakeGlyphs);
~~~~~~~~~~~~~

## Querying font information

Retrieve font bitmap information for a specific size using @b3d::Font::GetBitmap:

~~~~~~~~~~~~~{.cpp}
float sizeInPoints = 12.0f;
TShared<FontBitmapInformation> bitmapInformation = font->GetBitmap(sizeInPoints);

if (bitmapInformation)
{
    float lineHeight = bitmapInformation->LineHeight;
    float baselineOffset = bitmapInformation->BaselineOffset;
    float spaceWidth = bitmapInformation->SpaceWidth;
}
~~~~~~~~~~~~~

If you need the closest available size to a requested size, use @b3d::Font::GetClosestExistingBitmapSize:

~~~~~~~~~~~~~{.cpp}
float requestedSize = 11.5f;
float closestSize = font->GetClosestExistingBitmapSize(requestedSize);
~~~~~~~~~~~~~

Query information about specific characters using @b3d::Font::FindCharacterInformation:

~~~~~~~~~~~~~{.cpp}
u32 characterId = 'A';
float sizeInPoints = 12.0f;
const CharacterInformation* characterInfo = font->FindCharacterInformation(characterId, sizeInPoints);

if (characterInfo)
{
    float width = characterInfo->Width;
    float height = characterInfo->Height;
    float xAdvance = characterInfo->XAdvance;
}
~~~~~~~~~~~~~

## Managing glyph memory

Clear dynamically rendered glyphs to free up texture memory using @b3d::Font::ClearGlyphs:

~~~~~~~~~~~~~{.cpp}
// Clear all runtime-rendered glyphs, keeping baked glyphs
font->ClearGlyphs();

// Clear all glyphs including baked ones
font->ClearGlyphs(false);

// Clear glyphs for a specific size only
float sizeInPoints = 12.0f;
font->ClearGlyphs(sizeInPoints);
~~~~~~~~~~~~~

# SpriteGlyph

Individual font glyphs can be used as sprites through the @b3d::SpriteGlyph class. This allows single characters to be rendered as sprite images, which can be used with GUI elements, particle systems, and other rendering systems.

Create a sprite glyph by providing the font, the Unicode code point of the glyph, and the size in points:

~~~~~~~~~~~~~{.cpp}
HFont font = GetImporter().Import<Font>("lato.ttf");
u32 glyphCode = 'A'; // Unicode code point for the letter A
float sizeInPoints = 12.0f;
HSpriteGlyph spriteGlyph = SpriteGlyph::Create(font, glyphCode, sizeInPoints);
~~~~~~~~~~~~~

The sprite glyph will automatically handle rendering the character at the requested size. If the glyph was not pre-rendered during import, it will be rendered dynamically.

For more information about using sprite glyphs with other systems, see the [sprite images](00_spriteImages.md) manual.
