/*
 * Copyright (C) 2020-2025 v0lt
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

#include "Utils/Util.h"
#include "Utils/MediaTypes.h"
#include "Utils/StringUtil.h"

LPCWSTR GetNameAndVersion();

void CopyFrameAsIs(const UINT height, BYTE* dst, UINT dst_pitch, const BYTE* src, int src_pitch);
