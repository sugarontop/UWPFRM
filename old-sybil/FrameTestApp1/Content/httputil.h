#pragma once

#include "IBinary.h"

namespace sybil {

	bool EncryptAES128( IStream* ssm,  LPCWSTR pwd, IStream** ret );
	bool DecryptAES128( IStream* ssm,  LPCWSTR pwd, IStream** ret );


	void IStreamSeekToTop(IStream* sm );
	DWORD IStreamLength(IStream* sm );


	IBinary Password16byte(LPCWSTR pwd);

	void MD5( LPCWSTR text, BSTR* md5 );

	IStream* ToIStream( const IBinary& bin );
	IBinary ToIBinary( IStream* bin );

	LPCSTR ToUtf8( LPCWSTR str );
	void NewGuid( BSTR* ret );

	/////////////////////////////////////////////////////////////////////////////////////////////////
	bool EncryptAES128( IBinary* src,  LPCWSTR pwd, IBinary* ret );
	bool DecrpytAES128( IBinary* src,  LPCWSTR pwd, IBinary* ret );



	bool StrToUtf8( LPCWSTR str, int len, IBinary* ret ); 
	bool Utf8ToBSTR( const IBinary& utf8, BSTR* ret );

	bool AsciiToBSTR( const IBinary& asc, BSTR* ret );

	bool Base64ToBinary( LPCWSTR str, int len, IBinary* ret ); 
	bool BinaryToBase64( const IBinary& bin, BSTR* ret ); 

	bool PercentEncode( LPCWSTR str, int len, BSTR* ret ); 
	

	IBinary HMACSHA256( const IBinary& key, const IBinary& data );

	bool Hash( const IBinary& src, int typ, IBinary* ret );
		

	void BinaryToStr( const IBinary& src, int type, BSTR* ret ); // hex


	IBinary Clone( const IBinary& src );


	bool ReadFile( LPCWSTR fnm, IBinary* ret );
	bool WriteFile( LPCWSTR fnm, const IBinary& src, int typ=0 );



	void GenerateHeader( const std::map<std::wstring,std::wstring>& m, BSTR** header );
	void DisponseHeader( BSTR* header, UINT cnt );

	/////////////////////////////////////////////////////////////////////
	class thread
	{
		public :
			// thread m;
			// m.create_thread( std::bind(&CTest::func2, k, 100, L"hoisasa" ));
			
			DWORD create_thread( std::function<void()> f); 
			void join_all();
			
		private :
			static DWORD CALLBACK Rap( LPVOID p );
			
			std::vector<HANDLE> hs_;

			struct st
			{
				std::function<void()> fs_;
			};
	};

};













