#include "pch.h"
#include "D2DContext.h"
#include "D2DCommon.h"

#define D2DAPI DLLEXPORT 

namespace V4 {


D2DAPI void WINAPI FillRectangle( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br )
{
	cxt->FillRectangle( rc, br );
}

D2DAPI void WINAPI CenterTextOut( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* br  )
{
	auto old = tf->GetTextAlignment();
	tf->SetTextAlignment( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER );	
	p->DrawText( str, length, tf, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	tf->SetTextAlignment( old );
}


std::wstring Format(LPCWSTR fm, ... )
{
	std::wstring rval;

	if ( fm )
	{
		va_list ap;
		va_start(ap, fm);			
		size_t len = _vscwprintf(fm,ap)+1;

		WCHAR cb[256];

		WCHAR* buffer = cb;
				
		if ( len > 256 )
			buffer = new WCHAR[len];

		int nWritten = _vswprintf_l(buffer,len, fm,0,ap);

		if (nWritten > 0)
		{					
			buffer[len-1] = 0;
			rval = buffer;
		}
				
		va_end(ap);
				
		if ( buffer != cb )
			delete [] buffer;
	}
	return rval;
}
void Trace( LPCWSTR fm, ... )
{
		{
			if ( fm )
			{
				va_list ap;
				va_start(ap, fm);			
				size_t len = _vscwprintf(fm,ap)+1;

				
				WCHAR* b = new WCHAR[len];
				WCHAR* buffer = b;


				int nWritten = _vswprintf_l(buffer,len, fm,0,ap);

				if (nWritten > 0)
				{					
					buffer[len-1] = 0;
					
				}
				
				va_end(ap);
				
				::OutputDebugString( buffer );

				delete [] b;
			}
		}
}
std::wstring str_remove( const std::wstring& str , int s, int e )
{
	if ( s == e )
		return str;

	_ASSERT( s < (int)str.length() );
	_ASSERT( e <= (int)str.length() );


	auto s1 = str.substr( 0, s );

	if ( str.length()-e > 0 )
	{
		auto s2 = str.substr( e, str.length()-e );
		return s1+s2;
	}
	return s1;
}
std::wstring str_append( const std::wstring& str , int s, WCHAR ch )
{
	if ( s < (int)str.length() )
	{

		auto s1 = str.substr( 0, s );

		auto s2 = str.substr( s, str.length()-s );

		s1 += ch;

		return s1+s2;
	}
	else if ( s == (int)str.length())
	{
		return str + ch;
	}
	else
		return L"str_append_error";
}
std::wstring CStrHex( DWORD dw )
{	
	return Format(L"0x%x", dw );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DLLEXPORT D2D1_RECT_F WINAPI V4::ScrollbarRect( D2DScrollbarInfo& info, int typ )
{
	if ( typ == 0 )
		return info.rc;
	
	if ( info.bVertical )
	{
	
		if ( typ == 1 )
		{
			FRectF rc = info.rc;
			rc.bottom = rc.top + info.button_height;
			return rc;
		}
		else if ( typ == 2 )
		{
			FRectF rc = info.rc;
			rc.bottom = info.rc.bottom;
			rc.top = info.rc.bottom - info.button_height;		
			return rc;
		}
		else if ( typ == 3 )
		{
			float min_thumb = 17; // �ŏ�thumb
			float y1 = info.rc.bottom-info.rc.top;
			float y2 = info.total_height - y1;

			float thumb_height = y1 - info.button_height*2;
			float rto = 1.0f;

			if ( y2 <= 0 )
			{
				// scrollbar �s�v
			}
			else if ( y1-y2-info.button_height*2-min_thumb >0 )
			{
				thumb_height = y1-y2-info.button_height*2;
			}
			else
			{
				thumb_height = min_thumb;
				float y2a = y1-thumb_height-info.button_height*2;

				rto = y2a / y2;

			}
		
			info.thumb_step_c = rto;
		
			FRectF thumbrc = info.rc;		
			thumbrc.top = info.position +  info.rc.top + info.button_height;
			thumbrc.bottom = thumbrc.top + thumb_height;

			if ( thumbrc.bottom > info.rc.bottom-info.button_height )
			{
				thumbrc.bottom = info.rc.bottom-info.button_height;
				thumbrc.top = thumbrc.bottom - thumb_height;

			}

			return thumbrc;
		}
	}
	else
	{
		float button_width = info.button_height;
		if ( typ == 1 )
		{
			FRectF rc = info.rc;
			rc.right = rc.left + button_width;
			return rc;
		}
		else if ( typ == 2 )
		{
			FRectF rc = info.rc;
			rc.right = info.rc.right;
			rc.left = rc.right - button_width;		
			return rc;
		}
		else if ( typ == 3 )
		{
			float min_thumb = 5; // �ŏ�thumb
			float w1 = info.rc.right-info.rc.left;
			float w2 = info.total_height - w1;

			float thumb_height = w1 - info.button_height*2;
			float rto = 1.0f;

			if ( w2 <= 0 )
			{
				// scrollbar �s�v
			}
			else if ( w1-w2-info.button_height*2-min_thumb >0 )
			{
				thumb_height = w1-w2-info.button_height*2;
			}
			else
			{
				thumb_height = min_thumb;
				float ywa = w1-thumb_height-info.button_height*2;

				rto = ywa / w2;

			}
		
			info.thumb_step_c = rto;
		
			FRectF thumbrc = info.rc;		
			thumbrc.left = info.position +  info.rc.left + info.button_height;
			thumbrc.right = thumbrc.left + thumb_height;

			if ( thumbrc.right > info.rc.right-info.button_height )
			{
				thumbrc.right = info.rc.right-info.button_height;
				thumbrc.left = thumbrc.left - thumb_height;

			}

			return thumbrc;
		}


	}
	return FRectF(0,0,0,0);
}
DLLEXPORT void WINAPI V4::DrawScrollbar( ID2D1RenderTarget* cxt, D2DScrollbarInfo& info )
{
	ComPTR<ID2D1SolidColorBrush> bkcolor,br1b,br2;
	cxt->CreateSolidColorBrush(D2RGBA(230,230,230,255 ), &bkcolor );
	cxt->CreateSolidColorBrush(D2RGBA(200,200,200,255 ), &br1b );
	cxt->CreateSolidColorBrush(D2RGBA(100,100,100,255 ), &br2 );
	
	cxt->FillRectangle( info.rc, bkcolor ); // �S��

	
	if ( info.bVertical )
	{
		// Upper button
		FRectF rc = ScrollbarRect( info, 1 );
				
		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3, info.rc.top+5 ) );				
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 0 );		
		

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3, info.rc.bottom-10 ) );				

		// Lower button
		rc = ScrollbarRect( info, 2 );		
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 2 );		

		cxt->SetTransform( &mat ); // �߂�



		// Thumb button
		rc = ScrollbarRect( info, 3 );
		rc.Offset( 1, 0 );
		

		info.thumb_rc = rc;

		bool bl = (bool)((info.stat & D2DScrollbarInfo::ONTHUMB) || (info.stat & D2DScrollbarInfo::CAPTURED));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	else
	{
		// left side button
		FRectF rc = ScrollbarRect( info, 1 );
				
		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+5, info.rc.top+5 ) );				
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 3 );		
		
		cxt->SetTransform( mat.CalcOffset( info.rc.right-10, info.rc.top+5 ) );				

		// right side button
		rc = ScrollbarRect( info, 2 );		
//		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 1 );		

		cxt->SetTransform( &mat ); // �߂�

		// Thumb button
		rc = ScrollbarRect( info, 3 );		
		rc.Offset( 0, 1 );

		info.thumb_rc = rc;

		bool bl = (bool)(((info.stat & D2DScrollbarInfo::ONTHUMB) > 0) | ((info.stat & D2DScrollbarInfo::CAPTURED) >0));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	
}
}