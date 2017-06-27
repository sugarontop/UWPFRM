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
		thread_scope(){}
		BOOL lock( thread_gui_lock se)
		{
			se_ = se.se_;
			return TryEnterCriticalSection( &se_ );
		}
		thread_scope( thread_gui_lock se ):se_(se.se_)
		{
			EnterCriticalSection( &se_ );
		}
		~thread_scope()
		{
			LeaveCriticalSection( &se_ );
		}

		
	private :
		CRITICAL_SECTION se_;
		
};