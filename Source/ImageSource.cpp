/*
 * (C) 2020-2024 see Authors.txt
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
#include "PropPage.h"
#include "../Include/Version.h"
#include "ImageSource.h"

#define OPT_REGKEY_IMAGESOURCE L"Software\\MPC-BE Filters\\MPC Image Source"
#define OPT_ImageDuration      L"ImageDuration"
#define OPT_MaxDimension       L"MaxDimension"

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

	DLog(L"Windows {}", GetWindowsVersion());
	DLog(GetNameAndVersion());

	if (phr) {
		*phr = S_OK;
	}

	// read settings

	CRegKey key;
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, OPT_REGKEY_IMAGESOURCE, KEY_READ)) {
		DWORD dw;
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_ImageDuration, dw)) {
			m_Sets.iImageDuration = discard((int)dw, 0, 0, 10);
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_MaxDimension, dw)) {
			m_Sets.iMaxDimension = ALIGN(std::clamp(dw, 4096ul, 16384ul), 4096);
		}
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
	if (!(new(std::nothrow) CImageStream(pszFileName, this, &hr))) {
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

	size_t nCount = m_fn.size() + 1;
	*ppszFileName = (LPOLESTR)CoTaskMemAlloc(nCount * sizeof(WCHAR));
	if (!(*ppszFileName)) {
		return E_OUTOFMEMORY;
	}

	wcscpy_s(*ppszFileName, nCount, m_fn.c_str());

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

	pPages->pElems[0] = __uuidof(CISMainPPage);

	return S_OK;
}

// IImageSource

STDMETHODIMP_(bool) CMpcImageSource::GetActive()
{
	return (GetPinCount() > 0);
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
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, OPT_REGKEY_IMAGESOURCE)) {
		key.SetDWORDValue(OPT_ImageDuration, m_Sets.iImageDuration);
		key.SetDWORDValue(OPT_MaxDimension,  m_Sets.iMaxDimension);
	}

	return S_OK;
}

// IExFilterConfig

STDMETHODIMP CMpcImageSource::Flt_GetInt64(LPCSTR field, __int64 *value)
{
	CheckPointer(value, E_POINTER);

	if (!strcmp(field, "version")) {
		*value  = ((uint64_t)VER_MAJOR << 48)
				| ((uint64_t)VER_MINOR << 32)
				| ((uint64_t)VER_BUILD << 16)
				| ((uint64_t)REV_NUM);
		return S_OK;
	}

	return E_INVALIDARG;
}

//
// CImageStream
//

void CImageStream::SetPixelFormats(IWICBitmapFrameDecode* pFrameDecode)
{
	ASSERT(m_pWICFactory);
	ASSERT(pFrameDecode);

	WICPixelFormatGUID pixelFormat = GUID_NULL;
	HRESULT hr = pFrameDecode->GetPixelFormat(&pixelFormat);

	m_DecodePixFmtDesc = *GetPixelFormatDesc(pixelFormat);
	CComPtr<IWICPalette> pPalette;
	UINT colorCount = 0;
	hr = m_pWICFactory->CreatePalette(&pPalette);
	hr = pFrameDecode->CopyPalette(pPalette);
	hr = pPalette->GetColorCount(&colorCount);

	if (m_DecodePixFmtDesc.cstype == CS_IDX) {
		ASSERT(colorCount > 0);
		// specify PixelFormatDesc for indexed format
		BOOL blackwhite = FALSE;
		BOOL grayscale = FALSE;
		BOOL alpha = FALSE;
		hr = pPalette->IsBlackWhite(&blackwhite);
		hr = pPalette->IsGrayscale(&grayscale);
		hr = pPalette->HasAlpha(&alpha);

		m_DecodePixFmtDesc.alpha = (alpha == TRUE);
		m_DecodePixFmtDesc.cstype = ((blackwhite || grayscale) && !alpha) ? CS_GRAY : CS_RGB;
	}

	if (m_DecodePixFmtDesc.cstype == CS_RGB) {
		if (m_DecodePixFmtDesc.cdepth <= 8) {
			if (m_DecodePixFmtDesc.alpha) {
				m_OuputPixFmt1 = GUID_WICPixelFormat32bppPBGRA;
				m_subtype1 = MEDIASUBTYPE_ARGB32;
			}
			else {
				m_OuputPixFmt1 = GUID_WICPixelFormat32bppBGR;
				m_subtype1 = MEDIASUBTYPE_RGB32;
			}
		}
		else {
			if (m_DecodePixFmtDesc.alpha) {
				m_OuputPixFmt1 = GUID_WICPixelFormat64bppPBGRA;
				m_subtype1 = MEDIASUBTYPE_BGRA64;
			}
			else {
				if (m_DecodePixFmtDesc.wicpfguid == GUID_WICPixelFormat48bppBGR) {
					m_OuputPixFmt1 = GUID_WICPixelFormat48bppBGR;
					m_subtype1 = MEDIASUBTYPE_BGR48;
				}
				else {
					m_OuputPixFmt1 = GUID_WICPixelFormat48bppRGB;
					m_subtype1 = MEDIASUBTYPE_RGB48;
				}
			}
		}
	}
	else if (m_DecodePixFmtDesc.cstype == CS_GRAY) {
		if (m_DecodePixFmtDesc.cdepth <= 8) {
			m_OuputPixFmt1 = GUID_WICPixelFormat8bppGray;
			m_subtype1 = MEDIASUBTYPE_Y800;
		}
		else {
			m_OuputPixFmt1 = GUID_WICPixelFormat16bppGray;
			m_subtype1 = MEDIASUBTYPE_Y16;
		}
	}
	else {
		m_OuputPixFmt1 = GUID_WICPixelFormat32bppPBGRA;
		m_subtype1 = MEDIASUBTYPE_ARGB32;
	}

	// subtype for compatibility with EVR, Haali VR and madVR
	m_OuputPixFmt2 = GUID_WICPixelFormat32bppBGR;
	m_subtype2 = MEDIASUBTYPE_RGB32;
}

CImageStream::CImageStream(const WCHAR* name, CSource* pParent, HRESULT* phr)
	: CSourceStream(name, phr, pParent, L"Output")
	, CSourceSeeking(name, (IPin*)this, phr, &m_cSharedState)
{
	CAutoLock cAutoLock(&m_cSharedState);

	// get a copy of the settings
	const Settings_t Sets = static_cast<CMpcImageSource*>(pParent)->m_Sets;

	CComPtr<IWICBitmapDecoder>     pDecoder;
	CComPtr<IWICBitmapFrameDecode> pFrameDecode;

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(LPVOID*)&m_pWICFactory
	);

#ifdef _DEBUG
	if (SUCCEEDED(hr)) {
		std::wstring dbgstr(L"WIC Decoders:");
		CComPtr<IEnumUnknown> pEnum;
		DWORD dwOptions = WICComponentEnumerateDefault;
		HRESULT hr2 = m_pWICFactory->CreateComponentEnumerator(WICDecoder, dwOptions, &pEnum);
		if (SUCCEEDED(hr2)) {
			WCHAR buffer[256]; // if not enough will be truncated
			ULONG cbFetched = 0;
			CComPtr<IUnknown> pElement = nullptr;
			while (S_OK == pEnum->Next(1, &pElement, &cbFetched)) {
				UINT cbActual = 0;
				CComQIPtr<IWICBitmapCodecInfo> pCodecInfo(pElement);
				// Codec name
				hr2 = pCodecInfo->GetFriendlyName(std::size(buffer)-1, buffer, &cbActual);
				if (SUCCEEDED(hr2)) {
					dbgstr += std::format(L"\n  {}", buffer);
					// File extensions
					hr2 = pCodecInfo->GetFileExtensions(std::size(buffer) - 1, buffer, &cbActual);
					if (SUCCEEDED(hr2)) {
						dbgstr += std::format(L" {}", buffer);
					}
				}
				pElement = nullptr;
			}
		}

		DLog(dbgstr);
	}
#endif

	if (SUCCEEDED(hr)) {
		hr = m_pWICFactory->CreateDecoderFromFilename(
			name,
			nullptr,
			GENERIC_READ,
			// Specify WICDecodeMetadataCacheOnDemand or some JPEGs will fail to load with a WINCODEC_ERR_PROPERTYUNEXPECTEDTYPE error.
			WICDecodeMetadataCacheOnDemand,
			&pDecoder
		);
		DLogIf(FAILED(hr), L"CreateDecoderFromFilename failed with error {}", HR2Str(hr));
	}

	if (SUCCEEDED(hr)) {
		GUID containerFormat = GUID_NULL;
		pDecoder->GetContainerFormat(&containerFormat);
		m_ContainerFormat = ContainerFormat2Str(containerFormat);
		DLog(L"Container format: {}", m_ContainerFormat);

		hr = pDecoder->GetFrame(0, &pFrameDecode);
	}

#ifdef _DEBUG
	if (SUCCEEDED(hr)) {
		UINT frameCount = 0;
		hr = pDecoder->GetFrameCount(&frameCount);
		if (SUCCEEDED(hr)) {
			DLog(L"Frame count: {}", frameCount);
		}
	}
#endif

	if (SUCCEEDED(hr)) {
		SetPixelFormats(pFrameDecode);
		DLog(L"Decode pixel format: {}", m_DecodePixFmtDesc.str);
		DLog(L"Convert pixel format: {}", GetPixelFormatDesc(m_OuputPixFmt1)->str);
	}

	if (SUCCEEDED(hr)) {
		m_pBitmap1 = pFrameDecode;

		hr = m_pBitmap1->GetSize(&m_Width, &m_Height);
		DLogIf(SUCCEEDED(hr), L"Image frame size: {} x {}", m_Width, m_Height);
	}

	if (SUCCEEDED(hr)) {
		if (!IsEqualGUID(m_DecodePixFmtDesc.wicpfguid, m_OuputPixFmt1)){
			IWICBitmapSource *pFrameConvert = nullptr;
			hr = WICConvertBitmapSource(m_OuputPixFmt1, m_pBitmap1, &pFrameConvert);
			if (SUCCEEDED(hr)) {
				m_pBitmap1 = pFrameConvert;
				pFrameConvert->Release();
			}
			DLogIf(FAILED(hr), L"WICConvertBitmapSource failed with error {}", HR2Str(hr));
		}

		UINT dimension = std::max(m_Width, m_Height);
		if (dimension > Sets.iMaxDimension) {
			IWICBitmapScaler* pScaler;
			hr = m_pWICFactory->CreateBitmapScaler(&pScaler);
			if (SUCCEEDED(hr)) {
				UINT divider = (dimension + Sets.iMaxDimension - 1) / Sets.iMaxDimension;
				UINT w = m_Width / divider;
				UINT h = m_Height / divider;

				hr = pScaler->Initialize(m_pBitmap1, w, h, WICBitmapInterpolationModeFant);
				if (SUCCEEDED(hr)) {
					m_Width = w;
					m_Height = h;
					m_pBitmap1 = pScaler;
				}
				pScaler->Release();
			}
		}

		if (!IsEqualGUID(m_OuputPixFmt1, m_OuputPixFmt2)){
			hr = WICConvertBitmapSource(m_OuputPixFmt2, m_pBitmap1, &m_pBitmap2);
			DLogIf(FAILED(hr), L"WICConvertBitmapSource failed with error {}", HR2Str(hr));
		}
	}

	if (SUCCEEDED(hr)) {
		if (Sets.iImageDuration > 0) {
			m_rtDuration = m_rtStop = UNITS * Sets.iImageDuration;
			if (m_AvgTimePerFrame > m_rtDuration) {
				m_AvgTimePerFrame = m_rtDuration;
			}
		} else {
			m_rtDuration = 0;
		}

		const UINT bitdepth1   = GetPixelFormatDesc(m_OuputPixFmt1)->depth;
		const UINT stride1     = ALIGN(m_Width * bitdepth1 / 8, 4);
		const UINT bufferSize1 = stride1 * m_Height;

		CMediaType mt;
		mt.SetType(&MEDIATYPE_Video);
		mt.SetSubtype(&m_subtype1);
		mt.SetFormatType(&FORMAT_VideoInfo2);
		mt.SetTemporalCompression(FALSE);
		mt.SetSampleSize(bufferSize1);

		VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource                = { 0, 0, (long)m_Width, (long)m_Height};
		vih2->rcTarget                = vih2->rcSource;
		vih2->AvgTimePerFrame         = m_AvgTimePerFrame;
		vih2->bmiHeader.biSize        = sizeof(vih2->bmiHeader);
		vih2->bmiHeader.biWidth       = m_Width;
		vih2->bmiHeader.biPlanes      = 1;
		vih2->bmiHeader.biBitCount    = bitdepth1;
		vih2->bmiHeader.biSizeImage   = bufferSize1;

		if (m_subtype1 == FOURCCMap(m_subtype1.Data1)) { // {xxxxxxxx-0000-0010-8000-00AA00389B71}
			vih2->bmiHeader.biHeight      = m_Height;
			vih2->bmiHeader.biCompression = m_subtype1.Data1;
		} else {
			vih2->bmiHeader.biHeight      = -(long)m_Height;
			vih2->bmiHeader.biCompression = BI_RGB;
		}

		m_mts.push_back(mt);

		m_maxBufferSize = bufferSize1;

		if (m_subtype2 != m_subtype1) {
			const UINT bitdepth2   = GetPixelFormatDesc(m_OuputPixFmt2)->depth;
			const UINT stride2     = ALIGN(m_Width * bitdepth2 / 8, 4);
			const UINT bufferSize2 = stride2 * m_Height;

			// using RGB32 for additional media format
			ASSERT(m_subtype2 == MEDIASUBTYPE_RGB32 && bitdepth2 == 32);

			mt.SetSubtype(&m_subtype2);
			mt.SetSampleSize(bufferSize2);

			vih2->bmiHeader.biHeight      = -(long)m_Height;
			vih2->bmiHeader.biBitCount    = bitdepth2;
			vih2->bmiHeader.biCompression = BI_RGB;
			vih2->bmiHeader.biSizeImage   = bufferSize2;

			m_mts.push_back(mt);

			if (bufferSize2 > m_maxBufferSize) {
				m_maxBufferSize = bufferSize2;
			}
		}
	}

	*phr = hr;
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

		if (m_rtStop > 0 && m_rtStop < _I64_MAX / 2) {
			m_rtDuration = m_rtStop;
		} else {
			m_rtDuration = 0;
		}

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
	pProperties->cbBuffer = m_maxBufferSize;

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

		if (m_rtPosition >= m_rtStop || !m_pBitmap) {
			return S_FALSE;
		}

		BYTE* pDataOut = nullptr;
		hr = pSample->GetPointer(&pDataOut);
		if (FAILED(hr) || !pDataOut) {
			return S_FALSE;
		}

		long outSize = pSample->GetSize();

		AM_MEDIA_TYPE* pmt;
		if (SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt) {
			CMediaType mt(*pmt);
			SetMediaType(&mt);

			DeleteMediaType(pmt);
		}

		if (m_mt.formattype != FORMAT_VideoInfo2) {
			return S_FALSE;
		}

		VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)m_mt.Format();
		const UINT w = vih2->bmiHeader.biWidth;
		const UINT h = abs(vih2->bmiHeader.biHeight);
		const UINT bpp = vih2->bmiHeader.biBitCount;
		const UINT stride = ALIGN(w * bpp / 8, 4);

		if (w < m_Width || h != m_Height || outSize < (long)vih2->bmiHeader.biSizeImage) {
			return S_FALSE;
		}

		hr = m_pBitmap->CopyPixels(nullptr, stride, outSize, pDataOut);
		if (FAILED(hr)) {
			DLog(L"CopyPixels failed with error {}", HR2Str(hr));
			return S_FALSE;
		}

		pSample->SetActualDataLength(stride * m_Height);

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
	if (iPosition >= (int)m_mts.size()) {
		return VFW_S_NO_MORE_ITEMS;
	}

	*pmt = m_mts[iPosition];

	return S_OK;
}

HRESULT CImageStream::CheckMediaType(const CMediaType* pmt)
{
	if (pmt->majortype == MEDIATYPE_Video
		&& (pmt->subtype == m_subtype1 || pmt->subtype == m_subtype2)
		&& pmt->formattype == FORMAT_VideoInfo2) {

		VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)pmt->Format();
		if (vih2->bmiHeader.biWidth >= (long)m_Width && (UINT)std::abs(vih2->bmiHeader.biHeight) == m_Height) {
			return S_OK;
		}
	}

	return E_INVALIDARG;
}

HRESULT CImageStream::SetMediaType(const CMediaType* pMediaType)
{
	HRESULT hr = __super::SetMediaType(pMediaType);

	if (SUCCEEDED(hr)) {
		DLog(L"SetMediaType with subtype {}", GUIDtoWString(m_mt.subtype));

		WICPixelFormatGUID pixelFormat = GUID_NULL;
		if (m_pBitmap) {
			m_pBitmap->GetPixelFormat(&pixelFormat);
		}

		if (m_mt.subtype == m_subtype1) {
			if (pixelFormat != m_OuputPixFmt1) {
				m_pBitmap.Release();
				hr = m_pWICFactory->CreateBitmapFromSource(m_pBitmap1, WICBitmapCacheOnLoad, &m_pBitmap);
				DLogIf(FAILED(hr), L"CreateBitmapFromSource failed with error {}", HR2Str(hr));
			}
		}
		else if (m_mt.subtype == m_subtype2) {
			if (pixelFormat != m_OuputPixFmt2) {
				m_pBitmap.Release();
				hr = m_pWICFactory->CreateBitmapFromSource(m_pBitmap2, WICBitmapCacheOnLoad, &m_pBitmap);
				DLogIf(FAILED(hr), L"CreateBitmapFromSource failed with error {}", HR2Str(hr));
			}
		}
		else {
			hr = E_FAIL;
		}
	}

	if (FAILED(hr)) {
		m_pBitmap.Release();
	}

	return hr;
}

STDMETHODIMP CImageStream::Notify(IBaseFilter* pSender, Quality q)
{
	return E_NOTIMPL;
}
