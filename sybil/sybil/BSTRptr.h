#pragma once

class BSTRPtr
{
	public :
		BSTRPtr():bs_(0){}
		BSTRPtr(LPCWSTR b):bs_( ::SysAllocString(b)){}
		~BSTRPtr(){ free(); }

		BSTRPtr& operator =(const BSTR b)
		{
			if ( b != bs_ )
				attach( b );
			return *this;
		}
		void free()
		{
			SysFreeString(bs_);
			bs_ = nullptr;
		}
		void attach(BSTR b)
		{
			if ( bs_ ) free();
			bs_ = b;
		}
		BSTR detach()
		{
			auto r = bs_;
			bs_ = nullptr;
			return r;
		}

		BSTR* operator&()
		{
			_ASSERT(bs_ == nullptr);
			return &bs_;
		}
		operator LPCWSTR()
		{
			return (LPCWSTR)bs_;
		}
		operator BSTR()
		{
			return bs_;
		}
		int length() const
		{
			return SysStringLen(bs_);
		}
		BSTR substring( int pos, int len ) const
		{
			int dlen = max(0,min( length()- pos, len ));
			return ::SysAllocStringLen( bs_ + pos, dlen );
		}
		BSTR left( int len ) const
		{
			return substring(0,len);
		}
		BSTR right( int len ) const
		{
			int pos = length() - len;
			return substring(pos,len);
		}

		void split( LPCWSTR splitstr, BSTR* left, BSTR* right ) const;


	private:
		BSTR bs_;
};