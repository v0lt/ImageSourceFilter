/*
 * Copyright (C) 2020-2024 v0lt
 *
 * SPDX-License-Identifier: LGPL-2.1-only
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
