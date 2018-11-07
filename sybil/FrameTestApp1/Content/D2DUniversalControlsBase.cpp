#include "pch.h"
#include "D2DUniversalControlBase.h"
#include "D2DWindowMessage.h"
#include "D2DCommon.h"
#include "D2DDriftDialog.h"
#include "D2DSplitControls.h"
using namespace V4;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int D2DControls::WndProc(D2DWindow* parent, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	// 直接、DefWndProcをよべ
	return 0;
}

int D2DControls::DefPaintWndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	_ASSERT( message == WM_PAINT );

	int ret = 0;
		
	if ( !capture_.empty() )
	{
		// paintは逆順でループ
		for( auto it = controls_.rbegin(); it != controls_.rend(); it++ )
		{
			if ( (*it)->IsCaptured() == false )
				ret = (*it)->WndProc(d,message,0,lp);
		}

		// captureされたものを最後に表示
		if ( !capture_.empty() )
			(*capture_.ar_.begin())->WndProc(d,message,1,lp);  // 1: paint in capturing		
	
		return ret;
	}
	else
	{
		// paintは逆順でループ
		for( auto it = controls_.rbegin(); it != controls_.rend(); it++ )
		{
			if ( 0 != ( ret= (*it)->WndProc(d,message,0,lp)) )
				break;
		}
	}
	return ret;
}
//int D2DControls::DefWndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
//{
//	_ASSERT( message != WM_PAINT );
//
//	int ret = 0;
//
//	if ( !capture_.empty() )
//	{		
//		auto cap = capture_.top();
//
//		auto pr = cap;
//			
//if ( message == WM_LBUTTONDOWN )
//{
//	TRACE( L"WM_LBUTTONDOWN capture wndproc %x %d\n", this, capture_.size() );
//
//
//}
//		
//		{
//
//			ret = cap->WndProc(d,message,wp,lp); // capture_.empty()になることがある
//
//			if ( ret == 0 ) //|| capture_.empty() )
//			{
//				for( auto& it : controls_ )
//				{
//					if ( it.get() != pr )
//						ret = it->WndProc(d,message,wp,lp);
//
//					if ( ret ) break;
//				}
//			}
//		}
//	}
//	else
//	{		
//		for( auto& it : controls_ )		
//		{
//			if ( 0 != ( ret= it->WndProc(d,message,wp,lp)) )
//				break;
//		}
//	}
//
//	return ret;
//}

static D2DControls* test_sender2 = nullptr;

bool IsInStack( std::stack<void*> s, void* p )
{
	while( !s.empty())
	{
		if ( s.top() == p )
			return true;
		s.pop();
	}
	return false;
}

int D2DControls::DefWndProc(D2DWindow* d, int message, INT_PTR wp, Windows::UI::Core::ICoreWindowEventArgs^ lp)
{
	_ASSERT( message != WM_PAINT );

	int ret = 0;

	bool bProcess = true;

	auto d2 = dynamic_cast<D2DMainWindow*>(d);

	if ( d2 &&  !d2->BCaptureIsEmpty() )
	{
		D2DCaptureObject* capured_target = d2->BGetCapture();

		if (this != capured_target ){		
			if ( test_sender2 == nullptr )
			{
				test_sender2 = this; // TopのFrameTestAppになる

				if ( message == WM_LBUTTONDOWN )
				{
					int a=0;
					int b = a;
				}

				ret = capured_target->WndProc(d,message,wp,lp);
				
				test_sender2 = nullptr;

				bProcess = false;
			}
		}
	}

	if ( bProcess )
	{		
		for( auto& it : controls_ )		
		{
			if ( 0 != ( ret= it->WndProc(d,message,wp,lp)) )
				break;
		}
	}

	return ret;

}
std::shared_ptr<D2DControl> D2DControls::Detach(D2DControl* target)
{
	for( auto it = controls_.begin(); it != controls_.end(); ++it )
	{
		if ( (*it).get() == target )
		{
			std::shared_ptr<D2DControl> ret = (*it);

			controls_.erase( it );

			ret->parent_control_ = nullptr;

			return ret;
		}
	}

	_ASSERT( false );

	return nullptr;
}


void D2DControls::SetCapture(D2DCaptureObject* p, int layer )
{
	_ASSERT( p != nullptr );

	auto mainw =  dynamic_cast<D2DMainWindow*>(parent_);
	
	if ( layer == 0 )
	{
		_ASSERT ( parent_ != nullptr );
		_ASSERT ( dynamic_cast<D2DControl*>(p)->GetParentControl() == this );


		if ( mainw->BGetCapture() == p )
			return;

		ReleaseCapture(0); // all objects are released.
		
		mainw->BAddCapture(p);
	}

	

	// 表示で最初に表示させるため、並び順を変える。 Capture obj move to top
	if (controls_.size() > 1)
	{
		auto it = controls_.begin();
		for( ; it != controls_.end(); it++ )
		{
			if ( (*it).get() == p )
			{
				auto obj = (*it);
				controls_.erase(it);
				controls_.insert( controls_.begin(), obj );
				break;
			}
		}
		
	}

	if ( parent_control_  )
	{
		// 上のparent_controlをすべてcapture対象
		mainw->BAddCapture(this);

		parent_control_->SetCapture(this, layer+1);		
	}
	

	p->OnSetCapture(layer);
}

D2DCaptureObject* D2DControls::ReleaseCapture( D2DCaptureObject* target, int layer )
{
	_ASSERT( layer == 0 || layer == -1  );

	auto mainw =  dynamic_cast<D2DMainWindow*>(parent_);
	
	auto c1 = mainw->BCopyCapture();

	if ( target == nullptr || c1.exists(target) )
	{
		mainw->BReleaseCapture(target);

		bool br = false;

		while( !c1.empty() )
		{
			auto p = c1.top();

			if ( target == p )
				br = true;
		
			c1.top()->OnReleaseCapture(layer);
			c1.pop();


			if ( br )
				break;
		}		
	}

	return (target==nullptr ? nullptr : mainw->BGetCapture());
}


D2DCaptureObject* D2DControls::GetCapture()
{
	auto mainw =  dynamic_cast<D2DMainWindow*>(parent_);
	auto r = mainw->BGetCapture();
	return r;
}

void D2DControls::OnDXDeviceLost() 
{ 
	for( auto& it : controls_ ) it->OnDXDeviceLost(); 
}
void D2DControls::OnDXDeviceRestored()  
{ 
	for( auto& it : controls_ ) it->OnDXDeviceRestored(); 
}

