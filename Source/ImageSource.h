/*
 * (C) 2020-2022 see Authors.txt
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

#include <atltypes.h>
#include <thread>
#include "Helper.h"
#include "WICHelper.h"
#include "IImageSource.h"
#include "../Include/FilterInterfacesImpl.h"

#define STR_CLSID_ImageSource "{7DB5C3B3-2419-4508-B1D0-F2D22DA8E439}"

class __declspec(uuid(STR_CLSID_ImageSource))
	CMpcImageSource
	: public CSource
	, public IFileSourceFilter
	, public IAMFilterMiscFlags
	, public ISpecifyPropertyPages
	, public IImageSource
	, public CExFilterConfigImpl
{
private:
	friend class CImageStream;
	// Options
	Settings_t m_Sets;

	std::wstring m_fn;

public:
	CMpcImageSource(LPUNKNOWN lpunk, HRESULT* phr);
	~CMpcImageSource();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

	// IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt) override;
	STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt) override;

	// IAMFilterMiscFlags
	STDMETHODIMP_(ULONG) GetMiscFlags() override;

	// ISpecifyPropertyPages
	STDMETHODIMP GetPages(CAUUID* pPages) override;

	// IImageSource
	STDMETHODIMP_(bool) GetActive() override;
	STDMETHODIMP_(void) GetSettings(Settings_t& setings) override;
	STDMETHODIMP_(void) SetSettings(const Settings_t setings) override;
	STDMETHODIMP SaveSettings() override;

	// IExFilterConfig
	STDMETHODIMP GetInt64(LPCSTR field, __int64* value) override;
};

class CImageStream
	: public CSourceStream
	, public CSourceSeeking
{
private:
	CCritSec m_cSharedState;

	REFERENCE_TIME m_AvgTimePerFrame = UNITS * 2; // 0.5 fps
	REFERENCE_TIME m_rtSampleTime = 0;
	REFERENCE_TIME m_rtPosition = 0;

	BOOL m_bDiscontinuity = FALSE;
	BOOL m_bFlushing = FALSE;

	std::vector<CMediaType> m_mts;

	CComPtr<IWICImagingFactory> m_pWICFactory;

	CComPtr<IWICBitmap> m_pBitmap;
	CComPtr<IWICBitmapSource> m_pBitmap1;
	CComPtr<IWICBitmapSource> m_pBitmap2;

	UINT m_Width  = 0;
	UINT m_Height = 0;
	UINT m_maxBufferSize = 0;
	std::wstring m_ContainerFormat;
	PixelFormatDesc m_DecodePixFmtDesc;
	WICPixelFormatGUID m_OuputPixFmt1 = GUID_NULL;
	WICPixelFormatGUID m_OuputPixFmt2 = GUID_NULL;
	GUID m_subtype1 = GUID_NULL;
	GUID m_subtype2 = GUID_NULL;

	HRESULT OnThreadStartPlay() override;
	HRESULT OnThreadCreate() override;

	void UpdateFromSeek();
	STDMETHODIMP SetRate(double dRate) override;

	HRESULT ChangeStart() override;
	HRESULT ChangeStop() override;
	HRESULT ChangeRate() override { return S_OK; }

	void SetPixelFormats(IWICBitmapFrameDecode* pFrameDecode);

public:
	CImageStream(const WCHAR* name, CSource* pParent, HRESULT* phr);
	virtual ~CImageStream();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

	HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties) override;
	HRESULT FillBuffer(IMediaSample* pSample) override;
	HRESULT CheckMediaType(const CMediaType* pMediaType) override;
	HRESULT SetMediaType(const CMediaType* pMediaType) override;
	HRESULT GetMediaType(int iPosition, CMediaType* pmt) override;

	STDMETHODIMP Notify(IBaseFilter* pSender, Quality q) override;
};
