﻿2019/10 

* 	MIT LICENCE
* 	C++, UWP (DirectX11 application), Windows Store
* 	Direct2D
* 	using namespace Windows::UI::Text::Core;
* 	NO XAML, NO HWND
* 	Windows10
* 	VisualStudio2019


2019/10 
*   sybil以下終了、新たにD2DAppを開始


2018/11
*   XApp1.dllでD2DControlを拡張


2018/03
*	Chakra(JavaScript)+TypeScript


//Windowsメッセージを受け取る基本クラス

class D2DCaptureObject
{
	
	public:
		virtual ~D2DCaptureObject(){}
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)=0;
		virtual void OnReleaseCapture(int layer)=0;
		virtual void OnSetCapture(int layer)=0;
	
};

// クラス構成の概略

* class D2DControl : public D2DCaptureObject
* class D2DControls : public D2DControl 
* class D2DTextbox : public D2DControl
* class D2DChildFrame : public D2DControls


// FrameTestApp1のスタート時

void OnEntry(D2DWindow* parent,FSizeF iniSz, D2CoreTextBridge* imebridge)

{

	D2DMainWindow* main = dynamic_cast<D2DMainWindow*>(parent);
	main->imebridge_ = imebridge;

	D2DControls* ls = dynamic_cast<D2DControls*>(parent);
	D2DControls* lstop = ls;

	FRectF rc(50,50,FSizeF(900,800));
	D2DChildFrame* f1 = new D2DChildFrame();
	f1->Create( parent, lstop, rc, VISIBLE, D2DChildFrame::WINSTYLE::DEFAULT,  L"f1" );
	f1->SetCanvasSize(1000,2000);


	FRectF rcx(100,50,FSizeF(200,26));
	D2DTextbox* tx = new D2DTextbox(*imebridge);
	tx->Create(parent, f1, rcx, VISIBLE, L"noname" );
	tx->SetText( L"すべてDirect2Dによる描画");
	
	....
	
	OnEntryJavascript(); // load "init.js".
}
