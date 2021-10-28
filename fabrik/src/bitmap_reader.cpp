
#include "pch.h"
#include "bitmap_reader.h"
#include "defer.h"
#include "utils.h"
#include "Psd/Psd.h"
#include "Psd/PsdInterleave.h"
#include "Psd/PsdMallocAllocator.h"
#include "Psd/PsdNativeFile.h"
#include "Psd/PsdDocument.h"
#include "Psd/PsdParseDocument.h"
#include "Psd/PsdLayerMaskSection.h"
#include "Psd/PsdImageDataSection.h"
#include "Psd/PsdParseImageDataSection.h"
#include "Psd/PsdParseLayerMaskSection.h"
#include "Psd/PsdImageDataSection.h"
#include "Psd/PsdPlanarImage.h"
#include "Psd/PsdColorMode.h"

namespace bitmap_reader {
	ALLEGRO_BITMAP* load_bitmap_png(std::string_view path) {
		return al_load_bitmap(path.data());
	}

	ALLEGRO_BITMAP* load_bitmap_psd(std::string_view path) {
		using namespace psd;

		MallocAllocator allocator;
		NativeFile file(&allocator);
		defer{ file.Close(); };

		// try opening the file. if it fails, bail out.
		if (!file.OpenRead(to_wstring(path).c_str()))
			throw std::runtime_error("failed to open file");

		// create a new document that can be used for extracting different sections from the PSD.
		// additionally, the document stores information like width, height, bits per pixel, etc.
		Document* document = CreateDocument(&file, &allocator);
		if (!document)
			throw std::runtime_error("failed to create document");

		defer{ DestroyDocument(document, &allocator); };

		if (document->colorMode != colorMode::RGB)
			throw std::runtime_error("Document is not in RGB color mode");

		bool hasTransparencyMask = false;
		LayerMaskSection* layerMaskSection = ParseLayerMaskSection(document, &file, &allocator);
		if (layerMaskSection) {
			hasTransparencyMask = layerMaskSection->hasTransparencyMask;
		}

		ImageDataSection* imageData = ParseImageDataSection(document, &file, &allocator);
		if (imageData == nullptr)
			throw std::runtime_error("no image data");

		defer{ DestroyImageDataSection(imageData, &allocator); };


		// interleave the planar image data into one RGB or RGBA image.
		// store the rest of the (alpha) channels and the transparency mask separately.
		const unsigned int imageCount = imageData->imageCount;

		// note that an image can have more than 3 channels, but still no transparency mask in case all extra channels
		// are actual alpha channels.
		bool isRgb = false;
		if (imageCount == 3) {
			// imageData->images[0], imageData->images[1] and imageData->images[2] contain the R, G, and B channels of the merged image.
			// they are always the size of the canvas/document, so we can interleave them using imageUtil::InterleaveRGB directly.
			isRgb = true;
		}
		else if (imageCount >= 4) {
			// check if we really have a transparency mask that belongs to the "main" merged image.
			if (hasTransparencyMask) {
				// we have 4 or more images/channels, and a transparency mask.
				// this means that images 0-3 are RGBA, respectively.
				isRgb = false;
			}
			else {
				// we have 4 or more images stored in the document, but none of them is the transparency mask.
				// this means we are dealing with RGB (!) data, and several additional alpha channels.
				isRgb = true;
			}
		}

		ALLEGRO_BITMAP* newBitmap = al_create_bitmap(document->width, document->height);

		if (isRgb) {
			// RGB
			if (document->bitsPerChannel == 8) {
				// No fucking clue why ABGR
				auto locked = al_lock_bitmap(newBitmap, ALLEGRO_PIXEL_FORMAT_ABGR_8888, ALLEGRO_LOCK_WRITEONLY);
				defer{ al_unlock_bitmap(newBitmap); };

				imageUtil::InterleaveRGB((const uint8_t*)imageData->images[0].data,
										 (const uint8_t*)imageData->images[1].data,
										 (const uint8_t*)imageData->images[2].data,
										 0,
										 (uint8_t*)locked->data,
										 document->width,
										 document->height);
			}
			else if (document->bitsPerChannel == 16) {
				//image16 = CreateInterleavedImage<uint16_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, document->width, document->height);
			}
			else if (document->bitsPerChannel == 32) {
				//image32 = CreateInterleavedImage<float32_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, document->width, document->height);
			}
		}
		else {
			// RGBA
			if (document->bitsPerChannel == 8) {
				auto locked = al_lock_bitmap(newBitmap, ALLEGRO_PIXEL_FORMAT_ABGR_8888, ALLEGRO_LOCK_WRITEONLY);
				defer{ al_unlock_bitmap(newBitmap); };

				imageUtil::InterleaveRGBA((const uint8_t*)imageData->images[0].data,
										  (const uint8_t*)imageData->images[1].data,
										  (const uint8_t*)imageData->images[2].data,
										  (const uint8_t*)imageData->images[3].data,
										  (uint8_t*)locked->data,
										  document->width,
										  document->height);

				// By default, Allegro pre-multiplies the alpha channel of an image with the images color data when it loads it
				auto  data = (uint8_t*)locked->data;
				auto srcAlpha = (const uint8_t*)imageData->images[3].data;
				for (size_t y = 0; y < document->height; y++) {
					for (size_t x = 0; x < document->width; x++) {
						const auto a = (*srcAlpha++) / 255.f;
						data[x * 4 + 0] = (uint8_t)std::clamp(data[x * 4 + 0] * a, 0.f, 255.f); // R
						data[x * 4 + 1] = (uint8_t)std::clamp(data[x * 4 + 1] * a, 0.f, 255.f); // G
						data[x * 4 + 2] = (uint8_t)std::clamp(data[x * 4 + 2] * a, 0.f, 255.f); // B
					}
					data += locked->pitch;
				}
			}
			else if (document->bitsPerChannel == 16) {
				//image16 = CreateInterleavedImage<uint16_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, imageData->images[3].data, document->width, document->height);
			}
			else if (document->bitsPerChannel == 32) {
				//image32 = CreateInterleavedImage<float32_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, imageData->images[3].data, document->width, document->height);
			}
		}

		return newBitmap;
	}

	expected<ALLEGRO_BITMAP*> load_bitmap(std::string_view path) {
		ZoneScoped;
		ZoneText(path.data(), path.size());

		if (path.ends_with(".png"))
			return load_bitmap_png(path);
		else if (path.ends_with(".psd"))
			return load_bitmap_psd(path);

		return error{ std::format("Unknown file extension for bitmap '{}'", path) };
	}
}