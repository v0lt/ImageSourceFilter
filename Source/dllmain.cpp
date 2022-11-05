/*
 * (C) 2020-2022 see Authors.txt
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

#include "stdafx.h"
#include <InitGuid.h>
#include "ImageSource.h"
#include "PropPage.h"

template <class T>
static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
	*phr = S_OK;
	CUnknown* punk = new(std::nothrow) T(lpunk, phr);
	if (punk == nullptr) {
		*phr = E_OUTOFMEMORY;
	}
	return punk;
}

const AMOVIESETUP_PIN sudpPins[] = {
	{(LPWSTR)L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, 0, nullptr},
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CMpcImageSource), L"MPC Image Source", MERIT_NORMAL, (UINT)std::size(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMpcImageSource>, nullptr, &sudFilter[0]},
	{L"MainProp", &__uuidof(CISMainPPage), CreateInstance<CISMainPPage>, nullptr, nullptr},
};

int g_cTemplates = (int)std::size(g_Templates);

STDAPI DllRegisterServer()
{
	const LPCWSTR value_data[] {
		_CRT_WIDE(STR_CLSID_ImageSource),
		L"0,8,,89504E470D0A1A0A",          // PNG
		L"0,3,,FFD8FF",                    // JPEG
		L"0,2,,424D,6,4,,00000000",        // BMP
		L"0,4,,52494646,8,4,,57454250",    // WebP
		L"4,8,,667479706D696631,16,8,,6D69663168656963", // HEIF (.heic)
		L"4,8,,6674797068656963,16,8,,6D69663168656963", // HEIF (.heif)
		L"4,8,,6674797061766966,16,8,,6D69663161766966", // AVIF (.avif)
		L"0,2,,FF0A",                      // JPEG XL codestream
		L"0,12,,0000000C4A584C200D0A870A", // JPEG XL container
		L"0,4,,44445320",                  // DDS
		L"0,4,,49491A00",                  // CRW (Canon)
		L"0,4,,49492A00",                  // TIFF, DNG and other
		L"0,4,,4949524F",                  // ORF (Olympus)
		L"0,4,,49495500",                  // RW2 (Panasonic), RAW (Leica)
		L"0,4,,4949BC01",                  // HD Photo/JPEG XR
		L"0,4,,4D4D002A",                  // PEF (Pentax), NEF (Nikon), SRW (Samsung), ERF (Epson)
		L"0,16,,46554A4946494C4D4343442D52415720", // RAF (Fuji)
		L"0,4,,004D524D",                  // MRW (Minolta)
	};

	HKEY hKey;
	LONG ec = ::RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}\\" _CRT_WIDE(STR_CLSID_ImageSource), 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);
	if (ec == ERROR_SUCCESS) {
		ec = ::RegSetValueExW(hKey, L"Source Filter", 0, REG_SZ,
			reinterpret_cast<BYTE*>(const_cast<LPWSTR>(value_data[0])),
			(DWORD)(wcslen(value_data[0]) + 1) * sizeof(WCHAR));

		WCHAR value_name[4] = {};
		for (unsigned i = 1; i < std::size(value_data); i++) {
			_ultow_s(i, value_name, 10);
			ec = ::RegSetValueExW(hKey, value_name, 0, REG_SZ,
				reinterpret_cast<BYTE*>(const_cast<LPWSTR>(value_data[i])),
				(DWORD)(wcslen(value_data[i]) + 1) * sizeof(WCHAR));
		}

		::RegCloseKey(hKey);
	}

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	HKEY hKey;
	LONG ec = ::RegOpenKeyExW(HKEY_CLASSES_ROOT, L"Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}", 0, KEY_ALL_ACCESS, &hKey);
	if (ec == ERROR_SUCCESS) {
		ec = ::RegDeleteKeyW(hKey, _CRT_WIDE(STR_CLSID_ImageSource));
		::RegCloseKey(hKey);
	}

	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD dwReason, LPVOID pReserved)
{
	return DllEntryPoint(hDllHandle, dwReason, pReserved);
}
