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
#include <memory>
#include "../Include/Version.h"
#include "Helper.h"

#ifndef _WIN32_WINNT_WINTHRESHOLD
#define _WIN32_WINNT_WINTHRESHOLD 0x0A00
VERSIONHELPERAPI IsWindows10OrGreater()
{
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
}
#endif

LPCWSTR GetWindowsVersion()
{
	if (IsWindows10OrGreater()) {
		return L"10";
	}
	else if (IsWindows8Point1OrGreater()) {
		return L"8.1";
	}
	else if (IsWindows8OrGreater()) {
		return L"8";
	}
	else if (IsWindows7SP1OrGreater()) {
		return L"7 SP1";
	}
	else if (IsWindows7OrGreater()) {
		return L"7";
	}
	return L"Vista or older";
}

CStringW GetVersionStr()
{
	CStringW version;
#if MPCIS_RELEASE
	version.Format(L"v%S", MPCIS_VERSION_STR);
#else
	version.Format(L"v%S (git-%s-%s)",
		MPCIS_VERSION_STR,
		_CRT_WIDE(_CRT_STRINGIZE(MPCIS_REV_DATE)),
		_CRT_WIDE(_CRT_STRINGIZE(MPCIS_REV_HASH))
	);
#endif
#ifdef _WIN64
	version.Append(L" x64");
#endif
#ifdef _DEBUG
	version.Append(L" DEBUG");
#endif
	return version;
}

LPCWSTR GetNameAndVersion()
{
	static CStringW version = L"MPC Image Source " + GetVersionStr();

	return (LPCWSTR)version;
}

CStringW HR2Str(const HRESULT hr)
{
	CStringW str;
#define UNPACK_VALUE(VALUE) case VALUE: str = L#VALUE; break;
#define UNPACK_HR_WIN32(VALUE) case (((VALUE) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000): str = L#VALUE; break;
	switch (hr) {
		// common HRESULT values https://docs.microsoft.com/en-us/windows/desktop/seccrypto/common-hresult-values
		UNPACK_VALUE(S_OK);
		UNPACK_VALUE(S_FALSE);
		UNPACK_VALUE(E_NOTIMPL);
		UNPACK_VALUE(E_NOINTERFACE);
		UNPACK_VALUE(E_POINTER);
		UNPACK_VALUE(E_ABORT);
		UNPACK_VALUE(E_FAIL);
		UNPACK_VALUE(E_UNEXPECTED);
		UNPACK_VALUE(E_ACCESSDENIED);
		UNPACK_VALUE(E_HANDLE);
		UNPACK_VALUE(E_OUTOFMEMORY);
		UNPACK_VALUE(E_INVALIDARG);
		// some System Error Codes
		UNPACK_HR_WIN32(ERROR_INVALID_WINDOW_HANDLE);
	default:
		str.Format(L"0x%08x", hr);
	};
#undef UNPACK_VALUE
#undef UNPACK_HR_WIN32
	return str;
}

const WCHAR* ContainerFormat2Str(const GUID guid)
{
	const WCHAR* pStr;

	if (guid == GUID_ContainerFormatBmp)       { pStr = L"BMP"; }
	else if (guid == GUID_ContainerFormatPng)  { pStr = L"PNG"; }
	else if (guid == GUID_ContainerFormatIco)  { pStr = L"ICO"; }
	else if (guid == GUID_ContainerFormatJpeg) { pStr = L"JPEG"; }
	else if (guid == GUID_ContainerFormatTiff) { pStr = L"TIFF"; }
	else if (guid == GUID_ContainerFormatGif)  { pStr = L"GIF"; }
	else if (guid == GUID_ContainerFormatWmp)  { pStr = L"HD Photo"; }
	else if (guid == GUID_ContainerFormatDds)  { pStr = L"DDS"; }
	else { pStr = L"Unknown"; }
	 
	return pStr;
}

HRESULT GetDataFromResource(LPVOID& data, DWORD& size, UINT resid)
{
	static const HMODULE hModule = (HMODULE)&__ImageBase;

	HRSRC hrsrc = FindResourceW(hModule, MAKEINTRESOURCEW(resid), L"FILE");
	if (!hrsrc) {
		return E_INVALIDARG;
	}
	HGLOBAL hGlobal = LoadResource(hModule, hrsrc);
	if (!hGlobal) {
		return E_FAIL;
	}
	size = SizeofResource(hModule, hrsrc);
	if (!size) {
		return E_FAIL;
	}
	data = LockResource(hGlobal);
	if (!data) {
		return E_FAIL;
	}

	return S_OK;
}
