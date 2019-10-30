
#include "pch.h"
#include "D2DUniversalControl.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "Image.h"

using namespace V4;


Image::Image()
{

}
bool Image::LoadImage(LPCWSTR filenm)
{
	HRESULT hr;

	ComPTR<IWICImagingFactory> pWICFactory;
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory);
	if ( hr != S_OK ) return false;

	hr = pWICFactory->CreateStream(&Stream_);
	if (hr != S_OK) return false;

	hr = Stream_->InitializeFromFilename(filenm, GENERIC_READ); // bmp,gif,jpg,png OK
	return ( hr == S_OK );
}

void Image::Clear()
{
	if (Stream_ )	
	{
		Stream_->Release();
		Stream_ = nullptr;
	}
}

bool Image::GetImage(ID2D1RenderTarget* target, ID2D1Bitmap** bmp) const
{
	_ASSERT(Stream_);
	ComPTR<IWICImagingFactory> pWICFactory;
	if (S_OK != CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory))
		return false;

	return LoadImage2(target, pWICFactory, Stream_, bmp);
}

bool Image::LoadImage2(ID2D1RenderTarget* target, IWICImagingFactory* pWICFactory, IWICStream* pStream, ID2D1Bitmap** bmp)
{
	_ASSERT(pWICFactory);

	HRESULT hr;

	ComPTR<IWICBitmapDecoder> pDecoder;
	ComPTR<IWICBitmapFrameDecode> pSource;
	ComPTR<IWICFormatConverter> pConverter;

	hr = pWICFactory->CreateDecoderFromStream(pStream, 0, WICDecodeMetadataCacheOnLoad, &pDecoder); // jpeg,png:OK, bmp:88982f50のエラーになる, iconもエラー
	if (hr != S_OK) return false;
	hr = pDecoder->GetFrame(0, &pSource);
	if (hr != S_OK) return false;

	// Convert the image format to 32bppPBGRA
	// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	hr = pWICFactory->CreateFormatConverter(&pConverter);
	if (hr != S_OK) return false;
	hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
	if (hr != S_OK) return false;
	hr = target->CreateBitmapFromWicBitmap(pConverter, NULL, bmp);

	return (hr == S_OK);
}


