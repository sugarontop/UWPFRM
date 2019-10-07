#pragma once

namespace V4 {


struct D2DScrollbarInfo
{
	D2D1_RECT_F rc;	
	bool bVertical;
	float total_height;		// 全表示領域		
	float button_height;	// UP DOWNのボタン	
	bool auto_resize;
	
	float position;			// スクロールバーの位置

	float row_height;
	float thumb_step_c;		// center scollbar step,  scrollbar_off_.height = info.position / info.thumb_step_c;
	int rowno;				// 行番号方式の場合の現在の先頭行番号,  scrollbar_off_.height = info.rowno * info.row_height;

	enum STAT { NONE,ONTHUMB=0x1,ONBTN1=0x2,ONBTN2=0x4,ONSPC1=0x8,ONSPC2=0x10,CAPTURED=0x20 };
	int stat;
	

	D2D1_POINT_2F ptprv; // テンポラリのポインタ
	D2D1_RECT_F thumb_rc;	// 計算される

	ID2D1SolidColorBrush* clr[3]; // 0:back, 1:, 2:

};
struct D2DJson
{
	void* sender;
	BSTR json;
};

struct D2DMenuItem
{
	BSTR name;
	int id;

};

void FillRectangle(ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br);
void CenterTextOut( ID2D1RenderTarget* p, const D2D1_RECT_F& rc, LPCWSTR str, int length, IDWriteTextFormat* tf, ID2D1Brush* br  );
D2D1_RECT_F ScrollbarRect( D2DScrollbarInfo& info, int typ );

void DrawScrollbar( D2DContext& cxt, D2DScrollbarInfo& info );


bool MenuItemsJsonParse( LPCWSTR json, D2DMenuItem** head, int* itemscnt );
void MenuItemsClose( D2DMenuItem* head, int itemscnt );

std::wstring Format(LPCWSTR fm, ... );
void Trace( LPCWSTR fm, ... );
std::wstring str_append( const std::wstring& str , int s, WCHAR ch );
std::wstring str_remove( const std::wstring& str , int s, int e );

std::wstring CStrHex( DWORD dw );

ColorF HexToColorF( LPCWSTR rgb /*ex.#FFFFFF*/ );
};


