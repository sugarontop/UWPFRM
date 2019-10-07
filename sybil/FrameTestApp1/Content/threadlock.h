#pragma once


class thread_gui_lock
{
	friend class thread_scope;
	public :
		thread_gui_lock(){}

		void Init()
		{
			InitializeCriticalSection(&se_);
		}

		~thread_gui_lock()
		{
			DeleteCriticalSection(&se_);
		}

	private :
		CRITICAL_SECTION se_;
};
class thread_scope
{
	public :
		thread_scope():pse_(0){}
		/*BOOL lock(thread_gui_lock& se)
		{			
			BOOL bl = TryEnterCriticalSection(&se.se_);
			if ( bl )
				pse_ = &se.se_;
			
			return bl;
		}*/
		thread_scope(thread_gui_lock& se):pse_(&se.se_)
		{
			EnterCriticalSection( pse_ );			
		}
		~thread_scope()
		{			
			unlock();
		}
		void unlock()
		{
			if (pse_) {
				LeaveCriticalSection(pse_);
				pse_ = nullptr;
			}
		}
		
	private :
		CRITICAL_SECTION* pse_;
		
};