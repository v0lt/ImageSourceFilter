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
#include "resource.h"
#include "Helper.h"
#include "PropPage.h"

void SetCursor(HWND hWnd, LPCWSTR lpCursorName)
{
	SetClassLongPtrW(hWnd, GCLP_HCURSOR, (LONG_PTR)::LoadCursorW(nullptr, lpCursorName));
}

void SetCursor(HWND hWnd, UINT nID, LPCWSTR lpCursorName)
{
	SetCursor(::GetDlgItem(hWnd, nID), lpCursorName);
}

inline void ComboBox_AddStringData(HWND hWnd, int nIDComboBox, LPCWSTR str, LONG_PTR data)
{
	LRESULT lValue = SendDlgItemMessageW(hWnd, nIDComboBox, CB_ADDSTRING, 0, (LPARAM)str);
	if (lValue != CB_ERR) {
		SendDlgItemMessageW(hWnd, nIDComboBox, CB_SETITEMDATA, lValue, data);
	}
}

inline LONG_PTR ComboBox_GetCurItemData(HWND hWnd, int nIDComboBox)
{
	LRESULT lValue = SendDlgItemMessageW(hWnd, nIDComboBox, CB_GETCURSEL, 0, 0);
	if (lValue != CB_ERR) {
		lValue = SendDlgItemMessageW(hWnd, nIDComboBox, CB_GETITEMDATA, lValue, 0);
	}
	return lValue;
}

void ComboBox_SelectByItemData(HWND hWnd, int nIDComboBox, LONG_PTR data)
{
	LRESULT lCount = SendDlgItemMessageW(hWnd, nIDComboBox, CB_GETCOUNT, 0, 0);
	if (lCount != CB_ERR) {
		for (int idx = 0; idx < lCount; idx++) {
			const LRESULT lValue = SendDlgItemMessageW(hWnd, nIDComboBox, CB_GETITEMDATA, idx, 0);
			if (data == lValue) {
				SendDlgItemMessageW(hWnd, nIDComboBox, CB_SETCURSEL, idx, 0);
				break;
			}
		}
	}
}


// CISMainPPage

// https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd375010(v=vs.85).aspx

CISMainPPage::CISMainPPage(LPUNKNOWN lpunk, HRESULT* phr) :
	CBasePropertyPage(L"MainProp", lpunk, IDD_MAINPROPPAGE, IDS_MAINPROPPAGE_TITLE)
{
	DLog(L"CISMainPPage()");
}

CISMainPPage::~CISMainPPage()
{
	DLog(L"~CISMainPPage()");
}

void CISMainPPage::SetDurationToSlider()
{
	int pos = (m_SetsPP.iImageDuration >= 1 && m_SetsPP.iImageDuration <= 10)
		? m_SetsPP.iImageDuration
		: 11;
	SendDlgItemMessageW(IDC_SLIDER1, TBM_SETPOS, 1, pos);
}

void CISMainPPage::SetDurationToEdit()
{
	std::wstring str;
	if (m_SetsPP.iImageDuration >= 1 && m_SetsPP.iImageDuration <= 10) {
		str = std::to_wstring(m_SetsPP.iImageDuration);
	} else {
		str= L"unlimited";
	}
	SetDlgItemTextW(IDC_EDIT1, str.c_str());
}

int CISMainPPage::GetDurationFromSlider()
{
	LRESULT lValue = SendDlgItemMessageW(IDC_SLIDER1, TBM_GETPOS, 0, 0);
	return (lValue >= 1 && lValue <= 10) ? (int)lValue : 0;
}

void CISMainPPage::SetDimensionToEdit()
{
	std::wstring str = std::to_wstring(m_SetsPP.iMaxDimension);
	SetDlgItemTextW(IDC_EDIT2, str.c_str());
}

void CISMainPPage::SetControls()
{
	SetDurationToSlider();
	SetDurationToEdit();

	SendDlgItemMessageW(IDC_SLIDER2, TBM_SETPOS, 1, m_SetsPP.iMaxDimension / 4096);
	SetDimensionToEdit();
}

HRESULT CISMainPPage::OnConnect(IUnknown *pUnk)
{
	if (pUnk == nullptr) return E_POINTER;

	m_pImageSource = pUnk;
	if (!m_pImageSource) {
		return E_NOINTERFACE;
	}

	return S_OK;
}

HRESULT CISMainPPage::OnDisconnect()
{
	if (m_pImageSource == nullptr) {
		return E_UNEXPECTED;
	}

	m_pImageSource.Release();

	return S_OK;
}

HRESULT CISMainPPage::OnActivate()
{
	// set m_hWnd for CWindow
	m_hWnd = m_hwnd;

	m_pImageSource->GetSettings(m_SetsPP);

	SendDlgItemMessageW(IDC_SLIDER1, TBM_SETRANGE, 0, MAKELONG(1, 11));
	SendDlgItemMessageW(IDC_SLIDER1, TBM_SETTICFREQ, 1, 0);
	SendDlgItemMessageW(IDC_SLIDER2, TBM_SETRANGE, 0, MAKELONG(1, 4));
	SendDlgItemMessageW(IDC_SLIDER2, TBM_SETTICFREQ, 1, 0);
	SetDlgItemTextW(IDC_EDIT3, GetNameAndVersion());

	SetControls();

	SetCursor(m_hWnd, IDC_ARROW);

	return S_OK;
}

INT_PTR CISMainPPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND) {
		const int nID = LOWORD(wParam);

		if (HIWORD(wParam) == BN_CLICKED) {
			if (nID == IDC_BUTTON1) {
				m_SetsPP.SetDefault();
				SetControls();
				SetDirty();
				return (LRESULT)1;
			}
		}
	}
	else if (uMsg == WM_HSCROLL) {
		if ((HWND)lParam == GetDlgItem(IDC_SLIDER1)) {
			const int duration = GetDurationFromSlider();
			if (duration != m_SetsPP.iImageDuration) {
				m_SetsPP.iImageDuration = duration;
				SetDurationToEdit();
				SetDirty();
			}
			return (LRESULT)1;
		}
		if ((HWND)lParam == GetDlgItem(IDC_SLIDER2)) {
			LRESULT lValue = SendDlgItemMessageW(IDC_SLIDER2, TBM_GETPOS, 0, 0) * 4096;
			if (lValue != m_SetsPP.iMaxDimension) {
				m_SetsPP.iMaxDimension = lValue;
				SetDimensionToEdit();
				SetDirty();
			}
			return (LRESULT)1;
		}
	}

	// Let the parent class handle the message.
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

HRESULT CISMainPPage::OnApplyChanges()
{
	m_pImageSource->SetSettings(m_SetsPP);
	m_pImageSource->SaveSettings();

	return S_OK;
}
