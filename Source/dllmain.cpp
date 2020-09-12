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
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, 0, nullptr},
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CMpcImageSource), L"MPC Image Source", MERIT_NORMAL, std::size(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMpcImageSource>, nullptr, &sudFilter[0]},
	{L"MainProp", &__uuidof(CISMainPPage), CreateInstance<CISMainPPage>, nullptr, nullptr},
};

int g_cTemplates = std::size(g_Templates);

STDAPI DllRegisterServer()
{
	const struct {
		LPCWSTR name;
		LPCWSTR data;
	} values[] {
		{ L"0", L"0,8,,89504E470D0A1A0A" },       // PNG
		{ L"1", L"0,3,,FFD8FF" },                 // JPEG
		{ L"2", L"0,2,,424D,6,4,,00000000" },     // BMP
		{ L"3", L"0,4,,49492A00" },               // TIFF
		{ L"4", L"0,4,,4949BC01" },               // HD Photo/JPEG XR
		{ L"5", L"0,4,,52494646,8,4,,57454250" }, // WebP
		{ L"6", L"4,8,,667479706D696631,16,8,,6D69663168656963" }, // HEIF (.heic)
		{ L"6", L"4,8,,6674797068656963,16,8,,6D69663168656963" }, // HEIF (.heif)
		// This may not work for JPEG and BMP. "Generate Still Video" still connects.
		{ L"Source Filter", L"{7DB5C3B3-2419-4508-B1D0-F2D22DA8E439}" },
	};

	HKEY hKey;
	LONG ec = ::RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}\\{7DB5C3B3-2419-4508-B1D0-F2D22DA8E439}", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);
	if (ec == ERROR_SUCCESS) {
		for (const auto& value : values) {
			ec = ::RegSetValueExW(hKey, value.name, 0, REG_SZ,
				reinterpret_cast<BYTE*>(const_cast<LPWSTR>(value.data)),
				(DWORD)(wcslen(value.data) + 1) * sizeof(WCHAR));
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
		ec = ::RegDeleteKeyW(hKey, L"{7DB5C3B3-2419-4508-B1D0-F2D22DA8E439}");
		::RegCloseKey(hKey);
	}

	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD dwReason, LPVOID pReserved)
{
	return DllEntryPoint(hDllHandle, dwReason, pReserved);
}
