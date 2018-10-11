#pragma once
#include "D2DMisc.h"
#include "comptr.h"
#include "..\Common\DeviceResources.h"


#ifdef ENGLISH
#define DEFAULTFONT	L"Segoe UI"
#define DEFAULTFONT_HEIGHT	16
#define DEFAULTLOCALE	L"en-US"
#define DEFAULTWEIGHT	DWRITE_FONT_WEIGHT_NORMAL
#else
#define DEFAULTFONT	L"Yu Gothic UI"
#define DEFAULTFONT_HEIGHT	16
#define DEFAULTLOCALE	L"ja-JP"
#define DEFAULTWEIGHT	DWRITE_FONT_WEIGHT_NORMAL
#endif



namespace V4
{
	struct D2DContextBase
	{
		ComPTR<ID2D1RenderTarget>  cxt;
		IDWriteTextFormat* textformat;
		IDWriteFactory* wfactory;
	};


	// SingletonD2DInstanceは独立した存在なので、HWNDに関わるリソースはもたない。
	struct SingletonD2DInstance
	{
		ComPTR<IDWriteFactory> wrfactory;
		ComPTR<ID2D1Factory>  factory;
		ComPTR<IDWriteTextFormat> textformat; // IDWriteTextFormat1 is from Win8.1.

		static SingletonD2DInstance& Init();

	};

#define STOCKSIZE 16
	struct D2DContext;



	struct D2DContextText
	{
		bool Init(D2DContext& inshw, float height, LPCWSTR fontname);

		UINT GetLineMetrics(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics);
		UINT GetLineMetric(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric);
		UINT GetLineMetric(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics);
		UINT GetLineMetric(const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics);

		ComPTR<IDWriteTextFormat> textformat;
		ComPTR<IDWriteFactory> wfactory;

		float xoff;			// １行表示の左端の余幅
		float line_height;	// １行表示の高さ

	};
	struct D2DContext : public D2DContextBase
	{
		/*
		struct D2DContextBase
		{
		ComPTR<ID2D1RenderTarget>  cxt;
		IDWriteTextFormat* text;
		IDWriteFactory* wfactory;
		};
		*/


		DWORD tickcount_;
		SingletonD2DInstance* insins;


		operator ID2D1RenderTarget*() const { return cxt.p; }
#ifdef USE_ID2D1DEVICECONTEXT
		ComPTR<IDXGISwapChain1> dxgiSwapChain;
		D2D1_SIZE_U RenderSize_;
#endif

		ComPTR<ID2D1SolidColorBrush> ltgray;
		ComPTR<ID2D1SolidColorBrush> black;
		ComPTR<ID2D1SolidColorBrush> white;
		ComPTR<ID2D1SolidColorBrush> red;
		ComPTR<ID2D1SolidColorBrush> gray;
		ComPTR<ID2D1SolidColorBrush> bluegray;
		ComPTR<ID2D1SolidColorBrush> transparent;
		ComPTR<ID2D1SolidColorBrush> halftone;
		ComPTR<ID2D1SolidColorBrush> halftoneRed;
		ComPTR<ID2D1SolidColorBrush> tooltip;

		ComPTR<ID2D1LinearGradientBrush> silver_grd;

		ComPTR<ID2D1StrokeStyle> dot4_;
		ComPTR<ID2D1StrokeStyle> dot2_;
		ComPTR<ID2D1DrawingStateBlock>  m_stateBlock;

		ComPTR<ID2D1Factory> factory() { return insins->factory; }
		D2DContextText cxtt;

		LPVOID free_space;

		//void Init(SingletonD2DInstance& ins, HWND hWnd);
		void Destroy();

		//void Init(SingletonD2DInstance& ins, const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Init(SingletonD2DInstance& ins, ComPTR<ID2D1Factory> fac, ComPTR<IDWriteFactory> wfac, ComPTR<ID2D1DeviceContext> cxt );

		void CreateResourceOpt();

		void CreateDeviceContextRenderTargetTest(HWND hWnd, UINT width);


		void DestroyRenderTargetResource();

		void DestroyAll();

		void SetAntiAlias(bool bl) { cxt->SetAntialiasMode(bl ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED); }


		HRESULT CreateFont(LPCWSTR fontnm, float height, IDWriteTextFormat** ret);


		void CreateRenderResource(HWND hWnd);


		void CreateDeviceResources(ID2D1RenderTarget* t);


		ComPTR<IDWriteTextFormat> textformats[3];

	protected:
		void CreateHwndRenderTarget(HWND hWnd);
		void CreateDeviceContextRenderTarget(HWND hWnd);
		
		
	};

	struct D2DRectFilter
	{
		D2DRectFilter(D2DContext& cxt1, const FRectF& rc) :cxt(cxt1)
		{
			cxt.cxt->PushAxisAlignedClip(rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			cnt = 1;
		}

		D2DRectFilter(D2DContext& cxt1, FRectF&& rc) :cxt(cxt1)
		{
			cxt.cxt->PushAxisAlignedClip(std::move(rc), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			cnt = 1;
		}

		~D2DRectFilter()
		{
			if (cnt == 1)
				cxt.cxt->PopAxisAlignedClip();
		}
		void Off()
		{
			if (cnt == 1)
			{
				cxt.cxt->PopAxisAlignedClip();
				cnt = 0;
			}
		}

	private:
		D2DContext& cxt;
		int cnt;
	};

	

	/////////////////////////////////////////////////////////////////////////////////////////////
	//class D2DError
	//{
	//public:
	//	explicit D2DError(HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm) :hr_(hr), msg_(msg), line_(line), fnm_(fnm)
	//	{
	//		auto msg1 = V4::Format(L"%s :%d行 HR=%x %s\n", (LPCWSTR) fnm_.c_str(), line_, hr_, (LPCWSTR) msg_.c_str());

	//		::OutputDebugString(msg1.c_str());
	//	}

	//public:
	//	std::wstring msg_;
	//	std::wstring fnm_;
	//	UINT line_;
	//	HRESULT hr_;

	//};
	inline void ThrowIfFailed(HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm)
	{
		if (FAILED(hr))
		{
			//throw D2DError(hr, msg, line, fnm);
			//General access denied error 0x80070005 
		}
	};
#define THROWIFFAILED(hr,msg) ThrowIfFailed(hr,msg, __LINE__, __FILE__)



	
	struct TextInfo
	{
		friend class D2CoreTextBridge;
		TextInfo()
		{
			sel_start_pos = sel_end_pos =0;
			decoration_start_pos = decoration_end_pos =0;
			decoration_typ = 0;
			line_height = 0;
			
		}
		std::wstring text;
		int sel_start_pos;
		int sel_end_pos;
		FRectF rcTextbox; // device zahyou
		int line_cnt; 
		float line_height;

		FRectF rcTextboxLog;
		D2D1_MATRIX_3X2_F mat;

		std::function<void(void)> ontext_updated_;
		
		
		std::shared_ptr<FRectF> rcChar()
		{
			return rcChar_;
		}
		int rcCharCnt;

		int decoration_start_pos;
		int decoration_end_pos;
		int decoration_typ;

		ComPTR<IDWriteFactory> wfac_;
		ComPTR<IDWriteTextFormat> fmt_;

		void clear()
		{
			line_cnt = 0;
			rcChar_.reset();
			rcCharCnt = 0;
			text.clear();
			sel_start_pos = sel_end_pos =0;
			decoration_start_pos = decoration_end_pos =0;
			decoration_typ = 0;
			//line_height = 0;
		}
		void init()
		{
			sel_start_pos = sel_end_pos =0;
			decoration_start_pos = decoration_end_pos =0;
			decoration_typ = 0;
			
		}

		private :
			std::shared_ptr<FRectF> rcChar_;
	};


	class D2CoreTextBridge
	{
		public :
			D2CoreTextBridge():edcxt_(nullptr),target_(nullptr){}
			void Set( Windows::UI::Text::Core::CoreTextEditContext^ cxt)
			{	
				edcxt_ = cxt;
				edSelection_.StartCaretPosition = 0;
				edSelection_.EndCaretPosition = 0;
			}
			void Activate(TextInfo* inf, void* target)
			{
				target_ = target;
				info_ = inf;
				edcxt_->NotifyFocusEnter();

				Windows::UI::Text::Core::CoreTextRange rng;
				Windows::UI::Text::Core::CoreTextRange sel;

				sel.StartCaretPosition = info_->sel_start_pos;
				sel.EndCaretPosition = info_->sel_end_pos;


				edcxt_->NotifyTextChanged(rng,0,sel);
			}
			void NotifyTextChanged( int cnt )
			{				
				Windows::UI::Text::Core::CoreTextRange rng;
				Windows::UI::Text::Core::CoreTextRange sel;

				sel.StartCaretPosition = info_->sel_start_pos;
				sel.EndCaretPosition = info_->sel_end_pos;
				
				edcxt_->NotifyTextChanged(rng,cnt,sel);
			}
			void UnActivate()
			{
				target_ = nullptr;
				info_ = nullptr;
				edSelection_.StartCaretPosition = 0;
				edSelection_.EndCaretPosition = 0;
						

				edcxt_->NotifyFocusLeave();
			}

			void SetCaret(int pos)
			{
				Windows::UI::Text::Core::CoreTextRange sel;
				sel.StartCaretPosition = pos;
				sel.EndCaretPosition = pos;

				edcxt_->NotifySelectionChanged(sel);
			}
			void SetCaret(int spos, int epos)
			{
				Windows::UI::Text::Core::CoreTextRange sel;
				sel.StartCaretPosition = spos;
				sel.EndCaretPosition = epos;

				sel.StartCaretPosition = min(epos,spos);
				sel.EndCaretPosition = max(spos,epos);

				edcxt_->NotifySelectionChanged(sel);
			}

			void SetNewSelection( Windows::UI::Text::Core::CoreTextRange& ed )
			{
				edSelection_ = ed;
				info_->sel_start_pos = edSelection_.StartCaretPosition ;
				info_->sel_end_pos = edSelection_.EndCaretPosition ;
			}

			void CompositionStarted(){}
			void CompositionCompleted()
			{
				info_->decoration_typ = 0;
				info_->decoration_start_pos = info_->decoration_end_pos = 0;
				
			}

			Windows::UI::Text::Core::CoreTextRange GetSelection()
			{
				edSelection_.EndCaretPosition =	info_->sel_end_pos;
				edSelection_.StartCaretPosition = info_->sel_start_pos;

				return edSelection_;
			}
			void* GetTarget()
			{
				return target_;
			}
			/*
			Index 3 and 7 is \n. The width is zero.

			 ----+---+---+
			 | 0 | 1 | 2 | 
			-+---+---+---+
			3| 4 | 5 | 6 |
 			-+---+---+---+
			7| 8 | 9 |10 |
			-+---+---+---+

			Position 0 is left side of 0.
			Position 3 is left side of 3.
			Position 10 is left side of 10.

			*/
			void UpdateTextRect( FSizeF rcMaxText )
			{
				int len = info_->text.length();

				if (len == 0)
				{
					info_->clear();
					return;
				}

				ComPTR<IDWriteTextLayout> layout;		

				info_->wfac_->CreateTextLayout(info_->text.c_str(), len, info_->fmt_, rcMaxText.width, rcMaxText.height, &layout);
				info_->rcChar_ = std::shared_ptr<FRectF>(new FRectF[len], std::default_delete<FRectF[]>());

				FRectF* prc = info_->rcChar_.get();
				FRectF rc;

				info_->line_cnt = 1;
				info_->rcCharCnt = len;
				float prtop = 0;

				for (int i = 0; i < len; i++)
				{
					DWRITE_HIT_TEST_METRICS tm;
					float x1 = 0, y1 = 0;
					layout->HitTestTextPosition(i, false, &x1, &y1, &tm);
					
					rc.SetRect( tm.left, tm.top, tm.left+tm.width, tm.top+tm.height);

					
					
					if ( rc.Width() == 0 && info_->text[i] == L'\n' )
					{
						rc.Offset(0,rc.Height());
						rc.left = rc.right = 0;
					}
				
					prc[i] = rc;			

					if ( prtop < rc.top )
					{
						info_->line_cnt++;
						prtop = rc.top;
					}							
				}
				//::OutputDebugString( FString::Format(L"UpdateTextRect len=%d linecnt=%d\n", len, info_->line_cnt ).c_str());
			}

			TextInfo* info_;
			

			private :
				void* target_;
				Windows::UI::Text::Core::CoreTextEditContext^ edcxt_;
				Windows::UI::Text::Core::CoreTextRange edSelection_;			
				
	};

	class D2DColor
	{
		public :
			D2DColor():clr_(D2RGB(0,0,0)){}
			D2DColor( int r, int g, int b, int a ):clr_(D2RGBA(r,g,b,a)){}
			D2DColor( DWORD dw ):clr_( ColorF(((dw&0xFF0000)>>16)/255.0f, ((dw&0x00FF00)>>8)/255.0f, (dw&0xFF)/255.0f, 1.0f )){}
		
			operator ColorF() const { return clr_; }
				
		private :
			ColorF clr_;
	};

	class ColorPtr
	{
		protected :
			ID2D1SolidColorBrush* br_;
			

			ColorPtr():br_(nullptr){}

		public :
			ColorPtr(int r, int g, int b, int a=255 )
			{
				cxt_->CreateSolidColorBrush(D2RGBA(r,g,b,a), &br_ );
			}
			ColorPtr( const ColorF& clr )
			{
				cxt_->CreateSolidColorBrush(clr, &br_ );
			}

			~ColorPtr(){ br_->Release(); };
		
			operator ID2D1SolidColorBrush*() const { return br_; }
			static ID2D1RenderTarget* cxt_;
	};




#ifdef _DEBUG
#define TRACE V4::Trace
#else
#define TRACE
#endif

};



