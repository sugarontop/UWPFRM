#pragma once

namespace V4 {


	class D2DDatagridLight : public D2DControls
	{
		public :
			D2DDatagridLight(){}
			~D2DDatagridLight();
			virtual int WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp) override;
			void Create(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
			
			void Clear();

			struct ColInfo
			{				
				BSTR title;
				float width;
				int order;
			};	

			struct Row
			{
				BSTR* values;
				int colcnt;				
			};

			struct RowLayout
			{
				IDWriteTextLayout** values;
				int colcnt;
			};

			Row* NewRow();
			void AddRow(Row* row );
			void AddCol( ColInfo& col );

			int RowsCount();
			virtual void OnDXDeviceLost() override;
			virtual void OnDXDeviceRestored() override;

			void Update();

		private :
			virtual void UpdateScrollbar(D2DScrollbar* bar) override;
			RowLayout* GenRowLayout( Row* r, std::vector<ColInfo>& coli );
			void GenRowsLayout(int col=-1);
			void ClearRowsLayout();

			void DrawTitle(D2DContext& cxt );
			void DrawList(D2DContext& cxt );

			std::vector<ColInfo> cols_;
			std::vector<Row*> rows_;

			std::vector<RowLayout*> rows_layout_;

			int selected_row_;
			D2DMat matrow_;
			float col_sum_width_;

			std::shared_ptr<D2DControl> Vscbar_;
			FSizeF scrollbar_off_;

			FRectF GetRowsArea();
		
	};











};
