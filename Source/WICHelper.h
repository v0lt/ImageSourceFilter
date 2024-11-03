/*
 * Copyright (C) 2020-2024 v0lt
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

// Microsoft Camera Codec Pack
DEFINE_GUID(GUID_ContainerFormatRaw, 0xc1fc85cb, 0xd64f, 0x478b, 0xa4, 0xec, 0x69, 0xad, 0xc9, 0xee, 0x13, 0x92);

// Google WebP Codec
// https://github.com/webmproject/webp-wic-codec/blob/master/src/uuid.h
DEFINE_GUID(GUID_ContainerFormatWebp2, 0x1f122879, 0xeba0, 0x4670, 0x98, 0xc5, 0xcf, 0x29, 0xf3, 0xb9, 0x87, 0x11);

// CopyTrans HEIC for Windows
DEFINE_GUID(GUID_ContainerFormatHeic, 0x502a9ac6, 0x64f9, 0x43cb, 0x82, 0x44, 0xba, 0x76, 0x6b, 0x0c, 0x1b, 0x0b);

// Windows Imaging Component for HEIC file
// https://github.com/prsyahmi/wic_heic/blob/master/wic_heic/wic_heic.h
DEFINE_GUID(GUID_ContainerFormatHeic2, 0x98ce31dd, 0x5435, 0x41bd, 0xa1, 0xaf, 0x29, 0xc3, 0x87, 0xc2, 0x01, 0x35);

// Adobe DNG Codec
DEFINE_GUID(GUID_ContainerFormatAdng2, 0xcba90bec, 0x46e4, 0x4cb9, 0x82, 0x4f, 0x41, 0xa0, 0x9a, 0x12, 0x3c, 0x8d);

// Sony RAW Driver
DEFINE_GUID(GUID_ContainerFormatArw, 0x1d3b9fcd, 0xf3ab, 0x416a, 0x85, 0x69, 0xa4, 0x02, 0x86, 0x8c, 0x36, 0x0f);

// jxl-winthumb WIC Decoder
// https://github.com/saschanaz/jxl-winthumb/blob/main/src/lib.rs
DEFINE_GUID(GUID_ContainerFormatJXL, 0x81e337bc, 0xc1d1, 0x4dee, 0xa1, 0x7c, 0x40, 0x20, 0x41, 0xba, 0x9b, 0x5e);

// JPEG-LS Decoder
// https://github.com/team-charls/jpegls-wic-codec/blob/main/src/guids.ixx
DEFINE_GUID(GUID_ContainerFormatJpegLS, 0x52c25458, 0x282d, 0x4ef4, 0xa6, 0x9f, 0x2, 0x1b, 0xb2, 0x98, 0x45, 0x43);

enum ColorSystem_t {
	CS_YUV,
	CS_RGB, // RGB or CMYK
	CS_GRAY,
	CS_IDX
};

static const wchar_t* ContainerFormat2Str(const GUID guid)
{
	const wchar_t* pStr;

	if      (guid == GUID_ContainerFormatBmp)    { pStr = L"BMP"; }
	else if (guid == GUID_ContainerFormatPng)    { pStr = L"PNG"; }
	else if (guid == GUID_ContainerFormatIco)    { pStr = L"ICO"; }
	else if (guid == GUID_ContainerFormatJpeg)   { pStr = L"JPEG"; }
	else if (guid == GUID_ContainerFormatTiff)   { pStr = L"TIFF"; }
	else if (guid == GUID_ContainerFormatGif)    { pStr = L"GIF"; }
	else if (guid == GUID_ContainerFormatWmp)    { pStr = L"HD Photo/JPEG XR"; }
	else if (guid == GUID_ContainerFormatDds)    { pStr = L"DDS"; }
	else if (guid == GUID_ContainerFormatAdng
		 ||  guid == GUID_ContainerFormatAdng2)  { pStr = L"DNG"; }
	else if (guid == GUID_ContainerFormatHeif)   { pStr = L"HEIF"; }
	else if (guid == GUID_ContainerFormatWebp
		 ||  guid == GUID_ContainerFormatWebp2)  { pStr = L"WebP"; }
	else if (guid == GUID_ContainerFormatHeic
		 ||  guid == GUID_ContainerFormatHeic2)  { pStr = L"HEIC"; }
	else if (guid == GUID_ContainerFormatRaw)    { pStr = L"RAW"; }
	else if (guid == GUID_ContainerFormatArw)    { pStr = L"ARW"; }
	else if (guid == GUID_ContainerFormatJXL)    { pStr = L"JPEG XL"; }
	else if (guid == GUID_ContainerFormatJpegLS) { pStr = L"JPEG-LS"; }
	else { pStr = L"Unknown"; }

	return pStr;
}

struct PixelFormatDesc {
	WICPixelFormatGUID wicpfguid = GUID_NULL;
	const wchar_t*     str       = L"Unknown";
	int                cdepth    = 8;
	int                depth     = 32;
	ColorSystem_t      cstype    = CS_RGB;
	bool               alpha     = false;
};

static const PixelFormatDesc s_UnknownPixelFormatDesc = { };

static const PixelFormatDesc s_PixelFormatDescs[] = {
	{ GUID_WICPixelFormat1bppIndexed,           L"1bppIndexed",            8,   1, CS_IDX,  true  },
	{ GUID_WICPixelFormat2bppIndexed,           L"2bppIndexed",            8,   2, CS_IDX,  true  },
	{ GUID_WICPixelFormat4bppIndexed,           L"4bppIndexed",            8,   4, CS_IDX,  true  },
	{ GUID_WICPixelFormat8bppIndexed,           L"8bppIndexed",            8,   8, CS_IDX,  true  },
	{ GUID_WICPixelFormatBlackWhite ,           L"BlackWhite",             1,   1, CS_GRAY, false },
	{ GUID_WICPixelFormat2bppGray,              L"2bppGray",               2,   2, CS_GRAY, false },
	{ GUID_WICPixelFormat4bppGray,              L"4bppGray",               4,   4, CS_GRAY, false },
	{ GUID_WICPixelFormat8bppGray,              L"8bppGray",               8,   8, CS_GRAY, false },
	{ GUID_WICPixelFormat16bppBGR555,           L"16bppBGR555",            5,  16, CS_RGB,  false },
	{ GUID_WICPixelFormat16bppBGR565,           L"16bppBGR565",            6,  16, CS_RGB,  false },
	{ GUID_WICPixelFormat16bppGray,             L"16bppGray",             16,  16, CS_GRAY, false },
	{ GUID_WICPixelFormat32bppGrayFloat,        L"32bppGrayFloat",        32,  32, CS_GRAY, false },
	{ GUID_WICPixelFormat24bppBGR,              L"24bppBGR",               8,  24, CS_RGB,  false },
	{ GUID_WICPixelFormat24bppRGB,              L"24bppRGB",               8,  24, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppBGR,              L"32bppBGR",               8,  32, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppBGRA,             L"32bppBGRA",              8,  32, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppPBGRA,            L"32bppPBGRA",             8,  32, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppRGB,              L"32bppRGB",               8,  32, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppRGBA,             L"32bppRGBA",              8,  32, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppPRGBA,            L"32bppPRGBA",             8,  32, CS_RGB,  true  },
	{ GUID_WICPixelFormat48bppRGB,              L"48bppRGB",              16,  48, CS_RGB,  false },
	{ GUID_WICPixelFormat48bppBGR,              L"48bppBGR",              16,  48, CS_RGB,  false },
	{ GUID_WICPixelFormat64bppRGB,              L"64bppRGB",              16,  64, CS_RGB,  false },
	{ GUID_WICPixelFormat64bppRGBA,             L"64bppRGBA",             16,  64, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppBGRA,             L"64bppBGRA",             16,  64, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppPRGBA,            L"64bppPRGBA",            16,  64, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppPBGRA,            L"64bppPBGRA",            16,  64, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppRGBAHalf,         L"64bppRGBAHalf",         16,  64, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppRGBAFixedPoint,   L"64bppRGBAFixedPoint",   16,  64, CS_RGB,  true  },
	{ GUID_WICPixelFormat96bppRGBFloat,         L"96bppRGBFloat",         32,  96, CS_RGB,  false },
	{ GUID_WICPixelFormat128bppRGBAFloat,       L"128bppRGBAFloat",       32, 128, CS_RGB,  true  },
	{ GUID_WICPixelFormat128bppPRGBAFloat,      L"128bppPRGBAFloat",      32, 128, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppBGR101010,        L"32bppBGR101010",        10,  32, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppR10G10B10A2HDR10, L"32bppR10G10B10A2HDR10", 10,  32, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppCMYK,             L"32bppCMYK",              8,  32, CS_RGB,  false },
	{ GUID_WICPixelFormat40bppCMYKAlpha,        L"40bppCMYKAlpha",         8,  40, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppCMYK,             L"64bppCMYK",             16,  64, CS_RGB,  false },
	{ GUID_WICPixelFormat80bppCMYKAlpha,        L"80bppCMYKAlpha",        16,  80, CS_RGB,  true  },
};

static const PixelFormatDesc* GetPixelFormatDesc(const WICPixelFormatGUID guid)
{
	for (const auto& pfd : s_PixelFormatDescs) {
		if (pfd.wicpfguid == guid) {
			return &pfd;
		}
	}
	return &s_UnknownPixelFormatDesc;
}
