/*
 * (C) 2020-2021 see Authors.txt
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

#include "IImageSource.h"

// CISMainPPage

class __declspec(uuid("DF0FC50C-81CE-4141-9BC3-0BEB233181BB"))
	CISMainPPage : public CBasePropertyPage, public CWindow
{
	CComQIPtr<IImageSource> m_pImageSource;

	Settings_t m_SetsPP;
	int iDurationSliderPos = 0;

public:
	CISMainPPage(LPUNKNOWN lpunk, HRESULT* phr);
	~CISMainPPage();

private:
	void SetDurationToSlider();
	void SetDurationToEdit();
	int GetDurationFromSlider();
	void SetDimensionToEdit();

	void SetControls();

	HRESULT OnConnect(IUnknown* pUnknown) override;
	HRESULT OnDisconnect() override;
	HRESULT OnActivate() override;
	void SetDirty()
	{
		m_bDirty = TRUE;
		if (m_pPageSite) {
			m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
		}
	}
	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	HRESULT OnApplyChanges() override;
};
