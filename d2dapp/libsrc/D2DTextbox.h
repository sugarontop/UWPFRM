#pragma once
namespace V4 {

class FontInfo;

class Caret
{
	private :
		Caret();

	public :
		static Caret& GetCaret();

		virtual void Activate( TextInfo& ti );
		virtual void UnActivate();

		virtual void ShowCaret();
		virtual void HideCaret();
		
		virtual bool Draw(D2DContext& cxt );
		virtual FRectF GetCaretRect();

		int bShow_;

		int is_start_change_;
	private :
		TextInfo* ti_;
		Windows::System::Threading::ThreadPoolTimer^ timer_;


};



class FontInfo
{
	public :
		FontInfo();

		ComPTR<IDWriteTextFormat> CreateFormat( IDWriteFactory* wfac ) const;
		
	public :
		float height;
		std::wstring fontname;
		int weight;

};

class D2DTextbox : public D2DControl
{
	
	public :
		enum TYP { SINGLELINE,MULTILINE,PASSWORD };
		D2DTextbox(D2CoreTextBridge& bridge,Caret& ca);
		D2DTextbox(D2CoreTextBridge& bridge,TYP ty,Caret& ca);

		void Create(D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
		virtual void OnReleaseCapture(int layer) override;
		
		void SetFont( const FontInfo& cf, int typ=-1 );
		void SetBackColor( ColorF back ){ back_ = back; }
		void SetForeColor( ColorF fore ){ fore_ = fore; }
		void SetAlign( int typ );
		void SetReadonly( bool IsReadOnly );
		

		virtual void SetText( LPCWSTR txt );
		virtual std::wstring GetText() const { return ti_.text; }
		virtual void UnActivate();

		static void s_SetAlign( IDWriteTextFormat* fmt, int typ );
		void Activate(int init_pos=0);
		static FRectFBoxModel Rect1(const FRectF& rc );
	protected :
		
		
		void DrawSelectArea(D2DContext& cxt);
		void OnTextUpdated();
		
	protected :
		TYP typ_;
		D2CoreTextBridge& bridge_;
		
		TextInfo ti_;
		int shift_control_key_;
		ColorF back_,fore_;
		bool IsReadOnly_;
		
		//static bool bMouseSelectMode_;
		ComPTR<IDWriteTextLayout> layout_;
				
		FRectF _rc() const;
		Caret& caret_;
		bool bMouseSelectMode_;
	public :
		INT_PTR opt_;
};


};
