#pragma once
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

#include "config.h"
#include "geommath.hpp"
#include "half_float.hpp"

namespace My {
template <class T>
inline uint8_t to_unorm(T f) {
    f = (f < (T)0.0) ? (T)0.0 : ((f > (T)1.0) ? (T)1.0 : f);
    return (uint8_t)(std::min)((int)0xFF,(int)(f * T(256.0)));
}

enum class COMPRESSED_FORMAT : uint16_t {
    NONE,
    DXT1,
    DXT2,
    DXT3,
    DXT4,
    DXT5,
    DXT10,
    BC1,
    BC1A,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6H,
    BC7,
    PVRTC,
    ETC,
    ASTC_4x4,
    ASTC_5x4,
    ASTC_5x5,
    ASTC_6x5,
    ASTC_6x6,
    ASTC_8x5,
    ASTC_8x6,
    ASTC_8x8,
    ASTC_10x5,
    ASTC_10x6,
    ASTC_10x8,
    ASTC_10x10,
    ASTC_12x10,
    ASTC_12x12,
    ASTC_3x3x3,
    ASTC_4x3x3,
    ASTC_4x4x3,
    ASTC_4x4x4,
    ASTC_5x4x4,
    ASTC_5x5x4,
    ASTC_5x5x5,
    ASTC_6x5x5,
    ASTC_6x6x5,
    ASTC_6x6x6,
    UNKNOWN
};

enum class PIXEL_FORMAT : uint16_t {
    UNKNOWN,
    R8,
    RG8,
    RGB8,
    RGBA8,
    R16,
    RG16,
    RGB16,
    RGBA16,
    R32,
    RG32,
    RGB32,
    RGBA32,
    R10G10B10A2,
    R5G6B5,
    D24R8,
    D32
};

std::ostream& operator<<(std::ostream& out, COMPRESSED_FORMAT format);

struct Image {
    uint32_t Width{0};
    uint32_t Height{0};
    uint16_t bitcount{0};
    uint16_t bitdepth{0};
    size_t pitch{0};
    size_t data_size{0};
    bool compressed{false};
    bool is_float{false};
    bool is_signed{false};
    uint8_t* data{nullptr};
    COMPRESSED_FORMAT compress_format{COMPRESSED_FORMAT::NONE};
    PIXEL_FORMAT pixel_format{PIXEL_FORMAT::UNKNOWN};
    struct Mipmap {
        uint32_t Width{0};
        uint32_t Height{0};
        size_t pitch{0};
        size_t offset{0};
        size_t data_size{0};

        Mipmap(uint32_t width, uint32_t height, size_t pitch_, size_t offset_,
               size_t data_size_) {
            Width = width;
            Height = height;
            pitch = pitch_;
            offset = offset_;
            data_size = data_size_;
        }
    };
    std::vector<Mipmap> mipmaps;

    Image() = default;
    Image(const Image& rhs) = delete;  // disable copy contruct
    Image(Image&& rhs) noexcept;
    Image& operator=(const Image& rhs) = delete;  // disable copy assignment
    Image& operator=(Image&& rhs) noexcept;
    ~Image() {
        if (data) delete[] data;
    }

    float GetX(uint32_t x, uint32_t y) const {
        if (x >= Width || y >= Height) return 0;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
                return 0;
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::RG8:
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::RGBA8:
                return *(data + y * pitch + x * (bitcount >> 3)) / 255.0;
            case PIXEL_FORMAT::R5G6B5:
                return ((*(data + y * pitch + x * (bitcount >> 3)) & 0xF8) >> 3) / 32.0;
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::RG16:
            case PIXEL_FORMAT::RGB16:
            case PIXEL_FORMAT::RGBA16:
                return float32(*(uint16_t *)(data + y * pitch + x * (bitcount >> 3)));
            case PIXEL_FORMAT::R32:
            case PIXEL_FORMAT::RG32:
            case PIXEL_FORMAT::RGB32:
            case PIXEL_FORMAT::RGBA32:
                return *(float *)(data + y * pitch + x * (bitcount >> 3));
            case PIXEL_FORMAT::R10G10B10A2:
                return (*(uint32_t *)(data + y * pitch + x * (bitcount >> 3)) >> 22) / 1023.0;
            default:
                assert(0);
        }

        return 0;
    }

    float GetY(uint32_t x, uint32_t y) const {
        if (x >= Width || y >= Height) return 0;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::R32:
                return 0;
            case PIXEL_FORMAT::RG8:
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::RGBA8:
                return *(data + y * pitch + x * (bitcount >> 3) + 1) / 255.0;
            case PIXEL_FORMAT::RG16:
            case PIXEL_FORMAT::RGB16:
            case PIXEL_FORMAT::RGBA16:
                return float32(*(uint16_t *)(data + y * pitch + x * (bitcount >> 3) + 2));
            case PIXEL_FORMAT::RG32:
            case PIXEL_FORMAT::RGB32:
            case PIXEL_FORMAT::RGBA32:
                return *(float *)(data + y * pitch + x * (bitcount >> 3) + 4);
            case PIXEL_FORMAT::R10G10B10A2:
                return ((*(uint32_t *)(data + y * pitch + x * (bitcount >> 3)) >> 12) & 0x3FF) / 1023.0;
            case PIXEL_FORMAT::R5G6B5:
                return (((*(data + y * pitch + x * (bitcount >> 3)) & 0x07)
                        << 3) +
                       ((*(data + y * pitch + x * (bitcount >> 3) + 1) &
                         0xE0) >>
                        5)) / 64.0;
            default:
                assert(0);
        }

        return 0;
    }

    float GetZ(uint32_t x, uint32_t y) const {
        if (x >= Width || y >= Height) return 0;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::R32:
                return 0;
            case PIXEL_FORMAT::RG8:
                return 0;
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::RGBA8:
                return *(data + y * pitch + x * (bitcount >> 3) + 2) / 255.0;
            case PIXEL_FORMAT::RG16:
                return 0;
            case PIXEL_FORMAT::RGB16:
            case PIXEL_FORMAT::RGBA16:
                return float32(*(uint16_t *)(data + y * pitch + x * (bitcount >> 3) + 4));
            case PIXEL_FORMAT::RG32:
                return 0;
            case PIXEL_FORMAT::RGB32:
            case PIXEL_FORMAT::RGBA32:
                return *(float *)(data + y * pitch + x * (bitcount >> 3) + 8);
            case PIXEL_FORMAT::R10G10B10A2:
                return ((*(uint32_t *)(data + y * pitch + x * (bitcount >> 3)) >> 2) & 0x3FF) / 1023.0;
            case PIXEL_FORMAT::R5G6B5:
                return (*(data + y * pitch + x * (bitcount >> 3) + 1) & 0x1F);
            default:
                assert(0);
        }

        return 0;
    }

    float GetW(uint32_t x, uint32_t y) const {
        if (x >= Width || y >= Height) return 0xFF;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::R32:
            case PIXEL_FORMAT::RG8:
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::R5G6B5:
                return 1.0;
            case PIXEL_FORMAT::RGBA8:
                return *(data + y * pitch + x * (bitcount >> 3) + 3) / 255.0;
            case PIXEL_FORMAT::RG16:
            case PIXEL_FORMAT::RGB16:
                return 1.0;
            case PIXEL_FORMAT::RGBA16:
                return float32(*(uint16_t *)(data + y * pitch + x * (bitcount >> 3) + 6));
            case PIXEL_FORMAT::RG32:
            case PIXEL_FORMAT::RGB32:
                return 1.0;
            case PIXEL_FORMAT::RGBA32:
                return *(float *)(data + y * pitch + x * (bitcount >> 3) + 12);
            case PIXEL_FORMAT::R10G10B10A2:
                return (*(uint32_t *)(data + y * pitch + x * (bitcount >> 3)) & 0x3) / 4.0;
            default:
                assert(0);
        }

        return 0;
    }

    void SetR(uint32_t x, uint32_t y, int value) {
        if (x >= Width || y >= Height) return;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
                break;
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::RG8:
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::RGBA8:
                *(data + y * pitch + x * (bitcount >> 3)) = value & 0xFF;
                break;
            case PIXEL_FORMAT::R5G6B5:
                *(data + y * pitch + x * (bitcount >> 3)) |= (value << 3) & 0xF8;
                break;
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::RG16:
            case PIXEL_FORMAT::RGB16:
            case PIXEL_FORMAT::RGBA16:
                *(data + y * pitch + x * (bitcount >> 3)) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 1) = (value & 0xFF'00) >> 8;
                break;
            case PIXEL_FORMAT::R32:
            case PIXEL_FORMAT::RG32:
            case PIXEL_FORMAT::RGB32:
            case PIXEL_FORMAT::RGBA32:
                *(data + y * pitch + x * (bitcount >> 3)) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 1) = (value & 0xFF'00) >> 8;
                *(data + y * pitch + x * (bitcount >> 3) + 2) = (value & 0xFF'00'00) >> 16;
                *(data + y * pitch + x * (bitcount >> 3) + 3) = value & 0xFF'00'00'00 >> 24;
                break;
            case PIXEL_FORMAT::R10G10B10A2:
                // not supported
                break;
            default:
                assert(0);
        }

        return;
    }

    void SetG(uint32_t x, uint32_t y, int value) {
        if (x >= Width || y >= Height) return;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
            case PIXEL_FORMAT::R8:
                break;
            case PIXEL_FORMAT::RG8:
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::RGBA8:
                *(data + y * pitch + x * (bitcount >> 3) + 1) = value & 0xFF;
                break;
            case PIXEL_FORMAT::R5G6B5:
                *(data + y * pitch + x * (bitcount >> 3)) |= (value & 0x38) >> 3;
                *(data + y * pitch + x * (bitcount >> 3) + 1) |= (value & 0x07);
                break;
            case PIXEL_FORMAT::R16:
                break;
            case PIXEL_FORMAT::RG16:
            case PIXEL_FORMAT::RGB16:
            case PIXEL_FORMAT::RGBA16:
                *(data + y * pitch + x * (bitcount >> 3) + 2) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 3) = (value & 0xFF'00) >> 8;
                break;
            case PIXEL_FORMAT::R32:
                break;
            case PIXEL_FORMAT::RG32:
            case PIXEL_FORMAT::RGB32:
            case PIXEL_FORMAT::RGBA32:
                *(data + y * pitch + x * (bitcount >> 3) + 4) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 5) = (value & 0xFF'00) >> 8;
                *(data + y * pitch + x * (bitcount >> 3) + 6) = (value & 0xFF'00'00) >> 16;
                *(data + y * pitch + x * (bitcount >> 3) + 7) = value & 0xFF'00'00'00 >> 24;
                break;
            case PIXEL_FORMAT::R10G10B10A2:
                // not supported
                break;
            default:
                assert(0);
        }

        return;
    }

    void SetB(uint32_t x, uint32_t y, int value) {
        if (x >= Width || y >= Height) return;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::RG8:
                break;
            case PIXEL_FORMAT::RGB8:
            case PIXEL_FORMAT::RGBA8:
                *(data + y * pitch + x * (bitcount >> 3) + 2) = value & 0xFF;
                break;
            case PIXEL_FORMAT::R5G6B5:
                *(data + y * pitch + x * (bitcount >> 3) + 2) |= (value & 0x1F);
                break;
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::RG16:
                break;
            case PIXEL_FORMAT::RGB16:
            case PIXEL_FORMAT::RGBA16:
                *(data + y * pitch + x * (bitcount >> 3) + 4) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 5) = (value & 0xFF'00) >> 8;
                break;
            case PIXEL_FORMAT::R32:
            case PIXEL_FORMAT::RG32:
                break;
            case PIXEL_FORMAT::RGB32:
            case PIXEL_FORMAT::RGBA32:
                *(data + y * pitch + x * (bitcount >> 3) + 8) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 9) = (value & 0xFF'00) >> 8;
                *(data + y * pitch + x * (bitcount >> 3) + 0xA) = (value & 0xFF'00'00) >> 16;
                *(data + y * pitch + x * (bitcount >> 3) + 0xB) = value & 0xFF'00'00'00 >> 24;
                break;
            case PIXEL_FORMAT::R10G10B10A2:
                // not supported
                break;
            default:
                assert(0);
        }

        return;
    }

    void SetA(uint32_t x, uint32_t y, int value) {
        if (x >= Width || y >= Height) return;

        switch (pixel_format) {
            case PIXEL_FORMAT::UNKNOWN:
            case PIXEL_FORMAT::R8:
            case PIXEL_FORMAT::RG8:
            case PIXEL_FORMAT::RGB8:
                break;
            case PIXEL_FORMAT::RGBA8:
                *(data + y * pitch + x * (bitcount >> 3) + 3) = value & 0xFF;
                break;
            case PIXEL_FORMAT::R5G6B5:
            case PIXEL_FORMAT::R16:
            case PIXEL_FORMAT::RG16:
            case PIXEL_FORMAT::RGB16:
                break;
            case PIXEL_FORMAT::RGBA16:
                *(data + y * pitch + x * (bitcount >> 3) + 6) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 7) = (value & 0xFF'00) >> 8;
                break;
            case PIXEL_FORMAT::R32:
            case PIXEL_FORMAT::RG32:
            case PIXEL_FORMAT::RGB32:
                break;
            case PIXEL_FORMAT::RGBA32:
                *(data + y * pitch + x * (bitcount >> 3) + 0xC) = value & 0xFF;
                *(data + y * pitch + x * (bitcount >> 3) + 0xD) = (value & 0xFF'00) >> 8;
                *(data + y * pitch + x * (bitcount >> 3) + 0xE) = (value & 0xFF'00'00) >> 16;
                *(data + y * pitch + x * (bitcount >> 3) + 0xF) = value & 0xFF'00'00'00 >> 24;
                break;
            case PIXEL_FORMAT::R10G10B10A2:
                // not supported
                break;
            default:
                assert(0);
        }

        return;
    }

    uint8_t GetR(uint32_t x, uint32_t y) const { return to_unorm(GetX(x, y)); }

    uint8_t GetG(uint32_t x, uint32_t y) const { return to_unorm(GetY(x, y)); }

    uint8_t GetB(uint32_t x, uint32_t y) const { return to_unorm(GetZ(x, y)); }

    uint8_t GetA(uint32_t x, uint32_t y) const { return to_unorm(GetW(x, y)); }

    void SaveTGA(const char* filename) const {
        assert(filename != NULL);

        if (compressed) {
            fprintf(stderr, "SaveTGA is called but the image is compressed.\n");
            return;
        }

        FILE* file = fopen(filename, "wb");

        // misc header information
        for (int i = 0; i < 18; i++) {
            if (i == 2)
                fprintf(file, "%c", 2);
            else if (i == 12)
                fprintf(file, "%c", Width & 0xFF);
            else if (i == 13)
                fprintf(file, "%c", (Width & 0xFF00) >> 8);
            else if (i == 14)
                fprintf(file, "%c", Height & 0xFF);
            else if (i == 15)
                fprintf(file, "%c", (Height & 0xFF00) >> 8);
            else if (i == 16) {
                fprintf(file, "%c", 32);
            }
            else if (i == 17) {
                fprintf(file, "%c", 0x08);
            }
            else
                fprintf(file, "%c", 0);
        }
        // the data
        for (uint32_t y = 0; y < Height; y++) {
            for (uint32_t x = 0; x < Width; x++) {
                // note reversed order: b, g, r
                fputc(GetB(x, y), file);
                fputc(GetG(x, y), file);
                fputc(GetR(x, y), file);
                fputc(GetA(x, y), file);
            }
        }
        fclose(file);
    }
};

std::ostream& operator<<(std::ostream& out, const Image& image);

void adjust_image(Image& image);
}  // namespace My