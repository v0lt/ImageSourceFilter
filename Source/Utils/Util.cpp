// Copyright (c) 2020-2025 v0lt, Aleksoid
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "stdafx.h"
#include "Util.h"

VERSIONHELPERAPI
IsWindows11OrGreater() // https://walbourn.github.io/windows-sdk-for-windows-11/
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
		VerSetConditionMask(
			0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			   VER_MINORVERSION, VER_GREATER_EQUAL),
			   VER_BUILDNUMBER, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN10);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN10);
	osvi.dwBuildNumber = 22000;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

LPCWSTR GetWindowsVersion()
{
	if (IsWindows11OrGreater()) {
		return L"11";
	}
	else if (IsWindows10OrGreater()) {
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

std::wstring HR2Str(const HRESULT hr)
{
	std::wstring str;
#define UNPACK_VALUE(VALUE) case VALUE: str = L#VALUE; break;
#define UNPACK_HR_WIN32(VALUE) case (((VALUE) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000): str = L#VALUE; break;
	switch (hr) {
		// Common HRESULT Values https://learn.microsoft.com/en-us/windows/win32/seccrypto/common-hresult-values
		UNPACK_VALUE(S_OK);
#ifdef _WINERROR_
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
		// some COM Error Codes (Generic) https://learn.microsoft.com/en-us/windows/win32/com/com-error-codes-1
		UNPACK_VALUE(REGDB_E_CLASSNOTREG);
		// some COM Error Codes (UI, Audio, DirectX, Codec) https://learn.microsoft.com/en-us/windows/win32/com/com-error-codes-10
		UNPACK_VALUE(DXGI_STATUS_OCCLUDED);
		UNPACK_VALUE(DXGI_STATUS_MODE_CHANGED);
		UNPACK_VALUE(DXGI_ERROR_INVALID_CALL);
		UNPACK_VALUE(DXGI_ERROR_DEVICE_REMOVED);
		UNPACK_VALUE(DXGI_ERROR_DEVICE_RESET);
		UNPACK_VALUE(DXGI_ERROR_SDK_COMPONENT_MISSING);
		UNPACK_VALUE(WINCODEC_ERR_COMPONENTNOTFOUND);
		UNPACK_VALUE(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT);
		UNPACK_VALUE(WINCODEC_ERR_UNSUPPORTEDOPERATION);
		UNPACK_VALUE(WINCODEC_ERR_PROPERTYUNEXPECTEDTYPE);
		// some System Error Codes https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes
		UNPACK_HR_WIN32(ERROR_GEN_FAILURE);
		UNPACK_HR_WIN32(ERROR_NOT_SUPPORTED);
		UNPACK_HR_WIN32(ERROR_INSUFFICIENT_BUFFER);
		UNPACK_HR_WIN32(ERROR_MOD_NOT_FOUND);
		UNPACK_HR_WIN32(ERROR_INVALID_WINDOW_HANDLE);
		UNPACK_HR_WIN32(ERROR_CLASS_ALREADY_EXISTS);
#endif
#ifdef _MFERROR_H
		UNPACK_VALUE(MF_E_INVALIDMEDIATYPE);
		UNPACK_VALUE(MF_E_NOTACCEPTING);
		UNPACK_VALUE(MF_E_NO_MORE_TYPES);
		UNPACK_VALUE(MF_E_INVALID_FORMAT);
#endif
#ifdef _D3D9_H_
		// some D3DERR values https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3derr
		UNPACK_VALUE(S_PRESENT_OCCLUDED);
		UNPACK_VALUE(S_PRESENT_MODE_CHANGED);
		UNPACK_VALUE(D3DERR_DEVICEHUNG);
		UNPACK_VALUE(D3DERR_DEVICELOST);
		UNPACK_VALUE(D3DERR_DEVICENOTRESET);
		UNPACK_VALUE(D3DERR_DEVICEREMOVED);
		UNPACK_VALUE(D3DERR_DRIVERINTERNALERROR);
		UNPACK_VALUE(D3DERR_INVALIDCALL);
		UNPACK_VALUE(D3DERR_OUTOFVIDEOMEMORY);
		UNPACK_VALUE(D3DERR_WASSTILLDRAWING);
		UNPACK_VALUE(D3DERR_NOTAVAILABLE);
#endif
	default:
		str = std::format(L"{:#010x}", (uint32_t)hr);
	};
#undef UNPACK_VALUE
#undef UNPACK_HR_WIN32
	return str;
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
