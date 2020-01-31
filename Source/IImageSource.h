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

struct Settings_t {
	int  iImageDuration;
	UINT iMaxDimension;

	Settings_t() {
		SetDefault();
	}

	void SetDefault() {
		iImageDuration = 0;
		iMaxDimension  = 4096;
	}
};

interface __declspec(uuid("1D6BC606-BEFB-4760-BC86-F4FC53DE4CF9"))
IImageSource : public IUnknown {
	STDMETHOD_(bool, GetActive()) PURE;

	STDMETHOD_(void, GetSettings(Settings_t& setings)) PURE;
	STDMETHOD_(void, SetSettings(const Settings_t setings)) PURE; // use copy of setings here

	STDMETHOD(SaveSettings()) PURE;
};
