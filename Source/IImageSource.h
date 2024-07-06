/*
 * Copyright (C) 2020-2024 v0lt
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

struct Settings_t {
	int  iImageDuration;
	UINT iMaxDimension;

	Settings_t() {
		SetDefault();
	}

	void SetDefault() {
		iImageDuration = 0;
		iMaxDimension  = 8192;
	}
};

interface __declspec(uuid("1D6BC606-BEFB-4760-BC86-F4FC53DE4CF9"))
IImageSource : public IUnknown {
	STDMETHOD_(bool, GetActive()) PURE;

	STDMETHOD_(void, GetSettings(Settings_t& setings)) PURE;
	STDMETHOD_(void, SetSettings(const Settings_t setings)) PURE; // use copy of setings here

	STDMETHOD(SaveSettings()) PURE;
};
