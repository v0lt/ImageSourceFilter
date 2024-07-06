/*
 * Copyright (C) 2020-2024 v0lt
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "stdafx.h"
#include "../Include/Version.h"
#include "Helper.h"

std::wstring GetVersionStr()
{
	std::wstring version = _CRT_WIDE(VERSION_STR);
#if VER_RELEASE != 1
	version += std::format(L" (git-{}-{})",
		_CRT_WIDE(_CRT_STRINGIZE(REV_DATE)),
		_CRT_WIDE(_CRT_STRINGIZE(REV_HASH))
	);
#endif
#ifdef _WIN64
	version.append(L" x64");
#endif
#ifdef _DEBUG
	version.append(L" DEBUG");
#endif
	return version;
}

LPCWSTR GetNameAndVersion()
{
	static std::wstring version = L"MPC Image Source " + GetVersionStr();

	return version.c_str();
}

void CopyFrameAsIs(const UINT height, BYTE* dst, UINT dst_pitch, const BYTE* src, int src_pitch)
{
	if (dst_pitch == src_pitch) {
		memcpy(dst, src, dst_pitch * height);
		return;
	}

	const UINT linesize = std::min((UINT)abs(src_pitch), dst_pitch);

	for (UINT y = 0; y < height; ++y) {
		memcpy(dst, src, linesize);
		src += src_pitch;
		dst += dst_pitch;
	}
}
