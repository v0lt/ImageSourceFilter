/*
* (C) 2020 see Authors.txt
*
* This file is part of MPC-BE.
*
* MPC-BE is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* MPC-BE is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#pragma once

enum ColorSystem_t {
	CS_YUV,
	CS_RGB,
	CS_GRAY,
	CS_IDX
};

struct PixelFormatDesc {
	WICPixelFormatGUID wicpfguid;
	const char*        str;
	int                cdepth;
	ColorSystem_t      cstype;
	bool               alpha;
};

static const char* ContainerFormat2Str(const GUID guid)
{
	const char* pStr;

	if (guid == GUID_ContainerFormatBmp)       { pStr = "BMP"; }
	else if (guid == GUID_ContainerFormatPng)  { pStr = "PNG"; }
	else if (guid == GUID_ContainerFormatIco)  { pStr = "ICO"; }
	else if (guid == GUID_ContainerFormatJpeg) { pStr = "JPEG"; }
	else if (guid == GUID_ContainerFormatTiff) { pStr = "TIFF"; }
	else if (guid == GUID_ContainerFormatGif)  { pStr = "GIF"; }
	else if (guid == GUID_ContainerFormatWmp)  { pStr = "HD Photo/JPEG XR"; }
	else if (guid == GUID_ContainerFormatDds)  { pStr = "DDS"; }
//	else if (guid == GUID_ContainerFormatAdng) { pStr = "DNG"; }
//	else if (guid == GUID_ContainerFormatHeif) { pStr = "HEIF"; }
//	else if (guid == GUID_ContainerFormatWebp) { pStr = "WebP"; }
//	else if (guid == GUID_ContainerFormatRaw)  { pStr = "RAW"; }
	else { pStr = "Unknown"; }

	return pStr;
}

static const PixelFormatDesc s_UnknownPixelFormatDesc = { GUID_NULL, "Unknown", 0, CS_RGB, false };

static const PixelFormatDesc s_PixelFormatDescs[] = {
	{ GUID_WICPixelFormat1bppIndexed, "1bppIndexed",  8, CS_IDX,  true  },
	{ GUID_WICPixelFormat2bppIndexed, "2bppIndexed",  8, CS_IDX,  true  },
	{ GUID_WICPixelFormat4bppIndexed, "4bppIndexed",  8, CS_IDX,  true  },
	{ GUID_WICPixelFormat8bppIndexed, "8bppIndexed",  8, CS_IDX,  true  },
	{ GUID_WICPixelFormatBlackWhite , "BlackWhite",   1, CS_GRAY, false },
	{ GUID_WICPixelFormat2bppGray,    "2bppGray",     2, CS_GRAY, false },
	{ GUID_WICPixelFormat4bppGray,    "4bppGray",     4, CS_GRAY, false },
	{ GUID_WICPixelFormat8bppGray,    "8bppGray",     8, CS_GRAY, false },
	{ GUID_WICPixelFormat16bppBGR555, "16bppBGR555",  5, CS_RGB,  false },
	{ GUID_WICPixelFormat16bppBGR565, "16bppBGR565",  6, CS_RGB,  false },
	{ GUID_WICPixelFormat16bppGray,   "16bppGray",   16, CS_GRAY, false },
	{ GUID_WICPixelFormat24bppBGR,    "24bppBGR",     8, CS_RGB,  false },
	{ GUID_WICPixelFormat24bppRGB,    "24bppRGB",     8, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppBGR,    "32bppBGR",     8, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppBGRA,   "32bppBGRA",    8, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppPBGRA,  "32bppPBGRA",   8, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppRGB,    "32bppRGB",     8, CS_RGB,  false },
	{ GUID_WICPixelFormat32bppRGBA,   "32bppRGBA",    8, CS_RGB,  true  },
	{ GUID_WICPixelFormat32bppPRGBA,  "32bppPRGBA",   8, CS_RGB,  true  },
	{ GUID_WICPixelFormat48bppRGB,    "48bppRGB",    16, CS_RGB,  false },
	{ GUID_WICPixelFormat48bppBGR,    "48bppBGR",    16, CS_RGB,  false },
	{ GUID_WICPixelFormat64bppRGB,    "64bppRGB",    16, CS_RGB,  false },
	{ GUID_WICPixelFormat64bppRGBA,   "64bppRGBA",   16, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppBGRA,   "64bppBGRA",   16, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppPRGBA,  "64bppPRGBA",  16, CS_RGB,  true  },
	{ GUID_WICPixelFormat64bppPBGRA,  "64bppPBGRA",  16, CS_RGB,  true  },
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

static void GetPixelFormats(
	IWICImagingFactory* pWICFactory,
	IWICBitmapFrameDecode* pFrameDecode,
	PixelFormatDesc& srcPixFmtDesc,
	WICPixelFormatGUID& convertPixFmt,
	GUID& subtype)
{
	ASSERT(pWICFactory);
	ASSERT(pFrameDecode);

	WICPixelFormatGUID pixelFormat = GUID_NULL;
	HRESULT hr = pFrameDecode->GetPixelFormat(&pixelFormat);

	srcPixFmtDesc = *GetPixelFormatDesc(pixelFormat);
	CComPtr<IWICPalette> pPalette;
	UINT colorCount = 0;
	hr = pWICFactory->CreatePalette(&pPalette);
	hr = pFrameDecode->CopyPalette(pPalette);
	hr = pPalette->GetColorCount(&colorCount);

	if (srcPixFmtDesc.cstype == CS_IDX) {
		ASSERT(colorCount > 0);
		// specify PixelFormatDesc for indexed format
		BOOL blackwhite = FALSE;
		BOOL grayscale = FALSE;
		BOOL alpha = FALSE;
		hr = pPalette->IsBlackWhite(&blackwhite);
		hr = pPalette->IsGrayscale(&grayscale);
		hr = pPalette->HasAlpha(&alpha);

		srcPixFmtDesc.alpha = (alpha == TRUE);
		srcPixFmtDesc.cstype = ((blackwhite || grayscale) & !alpha) ? CS_GRAY : CS_RGB;
	}

	if (srcPixFmtDesc.cstype == CS_RGB) {
		if (srcPixFmtDesc.alpha) {
			convertPixFmt = GUID_WICPixelFormat32bppPBGRA;
			subtype = MEDIASUBTYPE_ARGB32;
		}
		else {
			convertPixFmt = GUID_WICPixelFormat32bppBGR;
			subtype = MEDIASUBTYPE_RGB32;
		}
	}
	// TODO
	//else if (srcPixFmtDesc.cstype == CS_GRAY) {
	//	convertPixFmt = ;
	//	subtype = ;
	//}
	else {
		convertPixFmt = GUID_WICPixelFormat32bppPBGRA;
		subtype = MEDIASUBTYPE_ARGB32;
	}
}
