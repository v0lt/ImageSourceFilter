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
#include "Helper.h"
#include "PropPage.h"
#include "../Include/Version.h"
#include "ImageSource.h"

#define OPT_REGKEY_VIDEORENDERER L"Software\\MPC-BE Filters\\MPC Image Source"

//
// CMpcImageSource
//

CMpcImageSource::CMpcImageSource(LPUNKNOWN lpunk, HRESULT* phr)
	: CSource(L"MPC Image Source", lpunk, __uuidof(this))
{
#ifdef _DEBUG
	DbgSetModuleLevel(LOG_TRACE, DWORD_MAX);
	DbgSetModuleLevel(LOG_ERROR, DWORD_MAX);
#endif

	DLog(L"Windows %s", GetWindowsVersion());
	DLog(GetNameAndVersion());

	if (phr) {
		*phr = S_OK;
	}

	// read settings

	CRegKey key;
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, OPT_REGKEY_VIDEORENDERER, KEY_READ)) {
	}

	return;
}

CMpcImageSource::~CMpcImageSource()
{
	DLog(L"~CMpcImageSource()");
}

STDMETHODIMP CMpcImageSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		QI(IFileSourceFilter)
		QI(IAMFilterMiscFlags)
		QI(ISpecifyPropertyPages)
		QI(IImageSource)
		QI(IExFilterConfig)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// IFileSourceFilter

STDMETHODIMP CMpcImageSource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
	// TODO: destroy any already existing pins and create new, now we are just going die nicely instead of doing it :)
	if (GetPinCount() > 0) {
		return VFW_E_ALREADY_CONNECTED;
	}

	HRESULT hr = S_OK;
	if (!(new CImageStream(L"SubtitleStream", this, &hr))) {
		return E_OUTOFMEMORY;
	}

	if (FAILED(hr)) {
		return hr;
	}

	m_fn = pszFileName;

	return S_OK;
}

STDMETHODIMP CMpcImageSource::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
	CheckPointer(ppszFileName, E_POINTER);

	size_t nCount = m_fn.GetLength() + 1;
	*ppszFileName = (LPOLESTR)CoTaskMemAlloc(nCount * sizeof(WCHAR));
	if (!(*ppszFileName)) {
		return E_OUTOFMEMORY;
	}

	wcscpy_s(*ppszFileName, nCount, m_fn);

	return S_OK;
}

// IAMFilterMiscFlags

STDMETHODIMP_(ULONG) CMpcImageSource::GetMiscFlags()
{
	return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

// ISpecifyPropertyPages
STDMETHODIMP CMpcImageSource::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems = 1;
	pPages->pElems = reinterpret_cast<GUID*>(CoTaskMemAlloc(sizeof(GUID) * pPages->cElems));
	if (pPages->pElems == nullptr) {
		return E_OUTOFMEMORY;
	}

	pPages->pElems[0] = __uuidof(CIRMainPPage);

	return S_OK;
}

// IImageSource

STDMETHODIMP_(bool) CMpcImageSource::GetActive()
{
	return S_FALSE;
}

STDMETHODIMP_(void) CMpcImageSource::GetSettings(Settings_t& setings)
{
	setings = m_Sets;
}

STDMETHODIMP_(void) CMpcImageSource::SetSettings(const Settings_t setings)
{
	m_Sets = setings;
}

STDMETHODIMP CMpcImageSource::SaveSettings()
{
	CRegKey key;
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, OPT_REGKEY_VIDEORENDERER)) {

	}

	return S_OK;
}

// IExFilterConfig

STDMETHODIMP CMpcImageSource::GetInt64(LPCSTR field, __int64 *value)
{
	CheckPointer(value, E_POINTER);

	if (!strcmp(field, "version")) {
		*value  = ((uint64_t)MPCIS_VERSION_MAJOR << 48)
				| ((uint64_t)MPCIS_VERSION_MINOR << 32)
				| ((uint64_t)MPCIS_VERSION_BUILD << 16)
				| ((uint64_t)MPCIS_REV_NUM);
		return S_OK;
	}

	return E_INVALIDARG;
}

//
// CImageStream
//

CImageStream::CImageStream(const WCHAR* name, CSource* pParent, HRESULT* phr)
	: CSourceStream(name, phr, pParent, L"Output")
	, CSourceSeeking(name, (IPin*)this, phr, &m_cSharedState)
	, m_AvgTimePerFrame(0)
	, m_rtSampleTime(0)
	, m_rtPosition(0)
	, m_bDiscontinuity(FALSE)
	, m_bFlushing(FALSE)
{
	CAutoLock cAutoLock(&m_cSharedState);
	m_rtDuration = m_rtStop = 0;
}

CImageStream::~CImageStream()
{
	CAutoLock cAutoLock(&m_cSharedState);
}

STDMETHODIMP CImageStream::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return (riid == IID_IMediaSeeking) ? CSourceSeeking::NonDelegatingQueryInterface(riid, ppv)
		: CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

void CImageStream::UpdateFromSeek()
{
	if (ThreadExists()) {
		// next time around the loop, the worker thread will
		// pick up the position change.
		// We need to flush all the existing data - we must do that here
		// as our thread will probably be blocked in GetBuffer otherwise

		m_bFlushing = TRUE;

		DeliverBeginFlush();
		// make sure we have stopped pushing
		Stop();
		// complete the flush
		DeliverEndFlush();

		m_bFlushing = FALSE;

		// restart
		Run();
	}
}

HRESULT CImageStream::SetRate(double dRate)
{
	if (dRate <= 0) {
		return E_INVALIDARG;
	}

	{
		CAutoLock lock(CSourceSeeking::m_pLock);
		m_dRateSeeking = dRate;
	}

	UpdateFromSeek();

	return S_OK;
}

HRESULT CImageStream::OnThreadStartPlay()
{
	m_bDiscontinuity = TRUE;
	return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}

HRESULT CImageStream::ChangeStart()
{
	{
		CAutoLock lock(CSourceSeeking::m_pLock);
		m_rtSampleTime = 0;
		m_rtPosition = m_rtStart;
	}

	UpdateFromSeek();

	return S_OK;
}

HRESULT CImageStream::ChangeStop()
{
	{
		CAutoLock lock(CSourceSeeking::m_pLock);
		if (m_rtPosition < m_rtStop) {
			return S_OK;
		}
	}

	// We're already past the new stop time -- better flush the graph.
	UpdateFromSeek();

	return S_OK;
}

HRESULT CImageStream::OnThreadCreate()
{
	CAutoLock cAutoLockShared(&m_cSharedState);

	m_rtSampleTime = 0;
	m_rtPosition = m_rtStart;

	return CSourceStream::OnThreadCreate();
}

HRESULT CImageStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
	//CAutoLock cAutoLock(m_pFilter->pStateLock());

	ASSERT(pAlloc);
	ASSERT(pProperties);

	HRESULT hr = NOERROR;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_nBufferSize;

	ALLOCATOR_PROPERTIES Actual;
	if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	if (Actual.cbBuffer < pProperties->cbBuffer) {
		return E_FAIL;
	}
	ASSERT(Actual.cBuffers == pProperties->cBuffers);

	return NOERROR;
}

HRESULT CImageStream::FillBuffer(IMediaSample* pSample)
{
	HRESULT hr;

	{
		CAutoLock cAutoLockShared(&m_cSharedState);

		if (m_rtPosition >= m_rtStop) {
			return S_FALSE;
		}

		BYTE* pDataIn = m_pFrameBuffer;
		BYTE* pDataOut = nullptr;
		if (!pDataIn || FAILED(hr = pSample->GetPointer(&pDataOut)) || !pDataOut) {
			return S_FALSE;
		}

		AM_MEDIA_TYPE* pmt;
		if (SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt) {
			CMediaType mt(*pmt);
			SetMediaType(&mt);

			DeleteMediaType(pmt);
		}

		int w, h, bpp;
		if (m_mt.formattype == FORMAT_VideoInfo2) {
			w = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biWidth;
			h = abs(((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biHeight);
			bpp = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biBitCount;
		} else {
			return S_FALSE;
		}

		{
			// copy frame to buffer
		}

		pSample->SetActualDataLength(m_nBufferSize); // TODO

		REFERENCE_TIME rtStart, rtStop;
		// The sample times are modified by the current rate.
		rtStart = static_cast<REFERENCE_TIME>(m_rtSampleTime / m_dRateSeeking);
		rtStop  = rtStart + static_cast<int>(m_AvgTimePerFrame / m_dRateSeeking);
		pSample->SetTime(&rtStart, &rtStop);

		m_rtSampleTime += m_AvgTimePerFrame;
		m_rtPosition += m_AvgTimePerFrame;
	}

	pSample->SetSyncPoint(TRUE);

	if (m_bDiscontinuity) {
		pSample->SetDiscontinuity(TRUE);
		m_bDiscontinuity = FALSE;
	}

	return S_OK;
}

HRESULT CImageStream::GetMediaType(int iPosition, CMediaType* pmt)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	if (iPosition < 0) {
		return E_INVALIDARG;
	}
	if (iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}

	pmt->SetType(&MEDIATYPE_Video);
	pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
	pmt->SetFormatType(&FORMAT_VideoInfo2);
	pmt->SetTemporalCompression(TRUE);

	VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
	memset(vih2, 0, sizeof(VIDEOINFOHEADER2));
	vih2->AvgTimePerFrame = m_AvgTimePerFrame;
	vih2->bmiHeader.biSize = sizeof(vih2->bmiHeader);
	vih2->bmiHeader.biWidth = 640; // TODO
	vih2->bmiHeader.biHeight = -780; // TODO
	vih2->bmiHeader.biPlanes = 1;
	vih2->bmiHeader.biBitCount = 32;
	vih2->bmiHeader.biCompression = BI_RGB;
	vih2->bmiHeader.biSizeImage = m_nBufferSize;

	pmt->SetSampleSize(vih2->bmiHeader.biSizeImage);

	return NOERROR;
}

HRESULT CImageStream::CheckMediaType(const CMediaType* pmt)
{
	if (pmt->majortype == MEDIATYPE_Video
		&& pmt->subtype == MEDIASUBTYPE_RGB32
		&& pmt->formattype == FORMAT_VideoInfo2) {
		return S_OK;
	}

	return E_INVALIDARG;
}

STDMETHODIMP CImageStream::Notify(IBaseFilter* pSender, Quality q)
{
	return E_NOTIMPL;
}

