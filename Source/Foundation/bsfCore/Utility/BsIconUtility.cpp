//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsIconUtility.h"
#include "Image/BsPixelData.h"
#include "Image/BsColor.h"
#include "Error/BsException.h"

#define MSDOS_SIGNATURE 0x5A4D
#define PE_SIGNATURE 0x00004550
#define PE_32BIT_SIGNATURE 0x10B
#define PE_64BIT_SIGNATURE 0x20B
#define PE_NUM_DIRECTORY_ENTRIES 16
#define PE_SECTION_UNINITIALIZED_DATA 0x00000080
#define	PE_IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define PE_IMAGE_RT_ICON 3

namespace bs
{
	/**	MS-DOS header found at the beggining in a PE format file. */
	struct MSDOSHeader
	{
		UINT16 signature;
		UINT16 lastSize;
		UINT16 numBlocks;
		UINT16 numReloc;
		UINT16 hdrSize;
		UINT16 minAlloc;
		UINT16 maxAlloc;
		UINT16 ss;
		UINT16 sp;
		UINT16 checksum;
		UINT16 ip;
		UINT16 cs;
		UINT16 relocPos;
		UINT16 numOverlay;
		UINT16 reserved1[4];
		UINT16 oemId;
		UINT16 oemInfo;
		UINT16 reserved2[10];
		UINT32 lfanew;
	};

	/**	COFF header found in a PE format file. */
	struct COFFHeader
	{
		UINT16 machine;
		UINT16 numSections;
		UINT32 timeDateStamp;
		UINT32 ptrSymbolTable;
		UINT32 numSymbols;
		UINT16 sizeOptHeader;
		UINT16 characteristics;
	};

	/**	Contains address and size of data areas in a PE image. */
	struct PEDataDirectory
	{
		UINT32 virtualAddress;
		UINT32 size;
	};

	/**	Optional header in a 32-bit PE format file. */
	struct PEOptionalHeader32
	{
		UINT16 signature;
		UINT8 majorLinkerVersion;
		UINT8 minorLinkerVersion;
		UINT32 sizeCode;
		UINT32 sizeInitializedData;
		UINT32 sizeUninitializedData;
		UINT32 addressEntryPoint;
		UINT32 baseCode;
		UINT32 baseData;
		UINT32 baseImage;
		UINT32 alignmentSection;
		UINT32 alignmentFile;
		UINT16 majorOSVersion;
		UINT16 minorOSVersion;
		UINT16 majorImageVersion;
		UINT16 minorImageVersion;
		UINT16 majorSubsystemVersion;
		UINT16 minorSubsystemVersion;
		UINT32 reserved;
		UINT32 sizeImage;
		UINT32 sizeHeaders;
		UINT32 checksum;
		UINT16 subsystem;
		UINT16 characteristics;
		UINT32 sizeStackReserve;
		UINT32 sizeStackCommit;
		UINT32 sizeHeapReserve;
		UINT32 sizeHeapCommit;
		UINT32 loaderFlags;
		UINT32 NumRvaAndSizes;
		PEDataDirectory dataDirectory[16];
	};

	/**	Optional header in a 64-bit PE format file. */
	struct PEOptionalHeader64
	{
		UINT16 signature;
		UINT8 majorLinkerVersion;
		UINT8 minorLinkerVersion;
		UINT32 sizeCode;
		UINT32 sizeInitializedData;
		UINT32 sizeUninitializedData;
		UINT32 addressEntryPoint;
		UINT32 baseCode;
		UINT64 baseImage;
		UINT32 alignmentSection;
		UINT32 alignmentFile;
		UINT16 majorOSVersion;
		UINT16 minorOSVersion;
		UINT16 majorImageVersion;
		UINT16 minorImageVersion;
		UINT16 majorSubsystemVersion;
		UINT16 minorSubsystemVersion;
		UINT32 reserved;
		UINT32 sizeImage;
		UINT32 sizeHeaders;
		UINT32 checksum;
		UINT16 subsystem;
		UINT16 characteristics;
		UINT64 sizeStackReserve;
		UINT64 sizeStackCommit;
		UINT64 sizeHeapReserve;
		UINT64 sizeHeapCommit;
		UINT32 loaderFlags;
		UINT32 NumRvaAndSizes;
		PEDataDirectory dataDirectory[16];
	};

	/**	A section header in a PE format file. */
	struct PESectionHeader
	{
		char name[8];
		UINT32 virtualSize;
		UINT32 relativeVirtualAddress;
		UINT32 physicalSize;
		UINT32 physicalAddress;
		UINT8 deprecated[12];
		UINT32 flags;
	};

	/**	A resource table header within a .rsrc section in a PE format file. */
	struct PEImageResourceDirectory
	{
		UINT32 flags;
		UINT32 timeDateStamp;
		UINT16 majorVersion;
		UINT16 minorVersion;
		UINT16 numNamedEntries;
		UINT16 numIdEntries;
	};

	/**	A single entry in a resource table within a .rsrc section in a PE format file. */
	struct PEImageResourceEntry
	{
		UINT32 type;
		UINT32 offsetDirectory : 31;
		UINT32 isDirectory : 1;
	};

	/** An entry in a resource table referencing resource data. Found within a .rsrc section in a PE format file. */
	struct PEImageResourceEntryData
	{
		UINT32 offsetData;
		UINT32 size;
		UINT32 codePage;
		UINT32 resourceHandle;
	};

	/**	Header used in icon file format. */
	struct IconHeader
	{
		UINT32 size;
		INT32 width;
		INT32 height;
		UINT16 planes;
		UINT16 bitCount;
		UINT32 compression;
		UINT32 sizeImage;
		INT32 xPelsPerMeter;
		INT32 yPelsPerMeter;
		UINT32 clrUsed;
		UINT32 clrImportant;
	};

	void IconUtility::UpdateIconExe(const Path& path, const Map<UINT32, SPtr<PixelData>>& pixelsPerSize)
	{
		// A PE file is structured as such:
		//  - MSDOS Header
		//  - PE Signature
		//  - COFF Header
		//  - PE Optional Header
		//  - One or multiple sections
		//   - .code
		//   - .data
		//   - ...
		//   - .rsrc
		//    - icon/cursor/etc data

		std::fstream stream;
		stream.Open(path.toPlatformString().c_str(), std::ios::in | std::ios::out | std::ios::binary);

		// First check magic number to ensure file is even an executable
		UINT16 magicNum;
		stream.Read((char*)&magicNum, sizeof(magicNum));
		if (magicNum != MSDOS_SIGNATURE)
			BS_EXCEPT(InvalidStateException, "Provided file is not a valid executable.");

		// Read the MSDOS header and skip over it
		stream.Seekg(0);

		MSDOSHeader msdosHeader;
		stream.Read((char*)&msdosHeader, sizeof(MSDOSHeader));

		// Read PE signature
		stream.Seekg(msdosHeader.lfanew);

		UINT32 peSignature;
		stream.Read((char*)&peSignature, sizeof(peSignature));

		if (peSignature != PE_SIGNATURE)
			BS_EXCEPT(InvalidStateException, "Provided file is not in PE format.");

		// Read COFF header
		COFFHeader coffHeader;
		stream.Read((char*)&coffHeader, sizeof(COFFHeader));

		if (coffHeader.sizeOptHeader == 0) // .exe files always have an optional header
			BS_EXCEPT(InvalidStateException, "Provided file is not a valid executable.");

		UINT32 numSectionHeaders = coffHeader.numSections;

		// Read optional header
		auto optionalHeaderPos = stream.Tellg();

		UINT16 optionalHeaderSignature;
		stream.Read((char*)&optionalHeaderSignature, sizeof(optionalHeaderSignature));

		PEDataDirectory* dataDirectory = nullptr;
		stream.Seekg(optionalHeaderPos);
		if (optionalHeaderSignature == PE_32BIT_SIGNATURE)
		{
			PEOptionalHeader32 optionalHeader;
			stream.Read((char*)&optionalHeader, sizeof(optionalHeader));

			dataDirectory = optionalHeader.dataDirectory + PE_IMAGE_DIRECTORY_ENTRY_RESOURCE;
		}
		else if (optionalHeaderSignature == PE_64BIT_SIGNATURE)
		{
			PEOptionalHeader64 optionalHeader;
			stream.Read((char*)&optionalHeader, sizeof(optionalHeader));

			dataDirectory = optionalHeader.dataDirectory + PE_IMAGE_DIRECTORY_ENTRY_RESOURCE;
		}
		else
			BS_EXCEPT(InvalidStateException, "Unrecognized PE format.");

		// Read section headers
		auto sectionHeaderPos = optionalHeaderPos + (std::ifstream::pos_type)coffHeader.sizeOptHeader;
		stream.Seekg(sectionHeaderPos);

		PESectionHeader* sectionHeaders = bs_stack_alloc<PESectionHeader>(numSectionHeaders);
		stream.Read((char*)sectionHeaders, sizeof(PESectionHeader) * numSectionHeaders);

		// Look for .rsrc section header
		std::function<void(PEImageResourceDirectory*, PEImageResourceDirectory*, UINT8*, UINT32)> setIconData =
			[&](PEImageResourceDirectory* base, PEImageResourceDirectory* current, UINT8* imageData, UINT32 sectionAddress)
		{
			UINT32 numEntries = current->numIdEntries; // Not supporting name entries
			PEImageResourceEntry* entries = (PEImageResourceEntry*)(current + 1);

			for (UINT32 i = 0; i < numEntries; i++)
			{
				// Only at root does the type identify resource type
				if (base == current && entries[i].type != PE_IMAGE_RT_ICON)
					continue;

				if (entries[i].isDirectory)
				{
					PEImageResourceDirectory* child = (PEImageResourceDirectory*)(((UINT8*)base) + entries[i].offsetDirectory);
					setIconData(base, child, imageData, sectionAddress);
				}
				else
				{
					PEImageResourceEntryData* data = (PEImageResourceEntryData*)(((UINT8*)base) + entries[i].offsetDirectory);

					UINT8* iconData = imageData + (data->offsetData - sectionAddress);
					updateIconData(iconData, pixelsPerSize);
				}
			}
		};

		for (UINT32 i = 0; i < numSectionHeaders; i++)
		{
			if (sectionHeaders[i].flags & PE_SECTION_UNINITIALIZED_DATA)
				continue;

			if (strcmp(sectionHeaders[i].name, ".rsrc") == 0)
			{
				UINT32 imageSize = sectionHeaders[i].physicalSize;
				UINT8* imageData = (UINT8*)bs_stack_alloc(imageSize);

				stream.Seekg(sectionHeaders[i].physicalAddress);
				stream.Read((char*)imageData, imageSize);

				UINT32 resourceDirOffset = dataDirectory->virtualAddress - sectionHeaders[i].relativeVirtualAddress;
				PEImageResourceDirectory* resourceDirectory = (PEImageResourceDirectory*)&imageData[resourceDirOffset];

				setIconData(resourceDirectory, resourceDirectory, imageData, sectionHeaders[i].relativeVirtualAddress);
				stream.Seekp(sectionHeaders[i].physicalAddress);
				stream.Write((char*)imageData, imageSize);

				bs_stack_free(imageData);
			}
		}

		bs_stack_free(sectionHeaders);
		stream.Close();
	}

	void IconUtility::UpdateIconData(UINT8* iconData, const Map<UINT32, SPtr<PixelData>>& pixelsPerSize)
	{
		IconHeader* iconHeader = (IconHeader*)iconData;

		if (iconHeader->size != sizeof(IconHeader) || iconHeader->compression != 0
			|| iconHeader->planes != 1 || iconHeader->bitCount != 32)
		{
			// Unsupported format
			return;
		}

		UINT8* iconPixels = iconData + sizeof(IconHeader);
		UINT32 width = iconHeader->width;
		UINT32 height = iconHeader->height / 2;

		auto iterFind = pixelsPerSize.Find(width);
		if (iterFind == pixelsPerSize.End() || iterFind->second->GetWidth() != width
			|| iterFind->second->GetHeight() != height)
		{
			// No icon of this size provided
			return;
		}

		// Write colors
		SPtr<PixelData> srcPixels = iterFind->second;
		UINT32* colorData = (UINT32*)iconPixels;

		UINT32 idx = 0;
		for (INT32 y = (INT32)height - 1; y >= 0; y--)
		{
			for (UINT32 x = 0; x < width; x++)
				colorData[idx++] = srcPixels->GetColorAt(x, y).GetAsBGRA();
		}

		// Write AND mask
		UINT32 colorDataSize = width * height * sizeof(UINT32);
		UINT8* maskData = iconPixels + colorDataSize;

		UINT32 numPackedPixels = width / 8; // One per bit in byte

		for (INT32 y = (INT32)height - 1; y >= 0; y--)
		{
			UINT8 mask = 0;
			for (UINT32 packedX = 0; packedX < numPackedPixels; packedX++)
			{
				for (UINT32 pixelIdx = 0; pixelIdx < 8; pixelIdx++)
				{
					UINT32 x = packedX * 8 + pixelIdx;
					Color color = srcPixels->GetColorAt(x, y);
					if (color.a < 0.25f)
						mask |= 1 << (7 - pixelIdx);
				}

				*maskData = mask;
				maskData++;
			}
		}
	}
}
