#pragma once

namespace V4 {
class Image
{
	public :
		Image();

		bool LoadImage(LPCWSTR filenm);

		bool GetImage(ID2D1RenderTarget* target, ID2D1Bitmap** bmp) const;
		


		void Clear();
	protected :
		ComPTR<IWICStream> Stream_;


		static bool LoadImage2(ID2D1RenderTarget* target, IWICImagingFactory* pWICFactory, IWICStream* pStream, ID2D1Bitmap** bmp);
};


};
