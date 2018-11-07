#pragma once
namespace V4 {
namespace V4_XAPP1 {
class D2DInputTextbox : public D2DControl
{
	public :
		struct InputRow
		{
			LPCWSTR title;
			int typ;
			float height;
		};
		
		struct Init
		{
			float* width;
			int width_cnt;
			LPCWSTR fontnm;
			float title_font_height;
			float title_font_weight;
			D2DControl* textbox;

		};
	
	
	public :
		D2DInputTextbox(){};

		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, std::vector<InputRow>& rows, Init& init);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;

	private :
		void OnPaint(D2DContext& cxt);

		struct Row
		{
			ComPTR<IDWriteTextLayout> title;
			ComPTR<IDWriteTextLayout> value;
			int typ;
			int row;
			float height;
			
		};


		std::vector<Row> rows_;
		std::vector<float> cell_width_;

		D2DControl* tx_;
};


class D2DCells : public D2DControls
{
	public :
		D2DCells();
		
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR font, int fontheight, Script* sc);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;


		void SetData( BSTR json );
		void Reset();



	private :
		void OnPaint(D2DContext& cxt);
		void InnerSetting( void* p );

		struct Row
		{
			ComPTR<IDWriteTextLayout> title;
			ComPTR<IDWriteTextLayout> value;
			FRectF rc1,rc2;
			std::wstring val;
			int stat;
		};

		std::vector<Row> rows_;
		std::wstring fontnm_;
		float font_height_;
		float padding_l_, padding_h_;
		float width1_,width2_;
		Script* sc_;
		FRectF child_rc_;
		

};


#define ROSOKU_UNIT		(20.0f)

struct Rousoku
{
	float m1,m2,m3,m4;
};


class D2DChartView : public D2DControl
{
	public :
		D2DChartView();
		
		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat) ;
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)  override;
		void Load( LPCWSTR url );
		void TitleName( std::wstring nm, int typ );
	private :
		void OnPaint(D2DContext& cxt);

	public :
		class rousoku
		{
			public :
				Rousoku r_;
				

				void Draw(D2DContext& cxt)
				{
					float cpx = ROSOKU_UNIT/2;
					if ( r_.m1 < r_.m4 )
					{
						auto br = cxt.red;
						FRectF rc(0, r_.m1, ROSOKU_UNIT, r_.m4 );
						FPointF pt1(cpx,r_.m2), pt2(cpx,r_.m3);
						cxt.cxt->DrawLine(pt1,pt2,br);												
						cxt.cxt->FillRectangle( rc, br );
					}
					else
					{
						auto br = cxt.bluegray;
						FRectF rc(0, r_.m4, ROSOKU_UNIT, r_.m1 );
						FPointF pt1(cpx,r_.m2), pt2(cpx,r_.m3);
						cxt.cxt->DrawLine(pt1,pt2,br);												
						cxt.cxt->FillRectangle( rc, br );
					}

				}

		};


		std::vector<rousoku> ar_;
		D2DMat matchart_;
		float vmax,vmin;
		ComPTR<IDWriteTextLayout> title_;
		std::wstring stock_cd_;
		int chart_typ_;
};


};
};