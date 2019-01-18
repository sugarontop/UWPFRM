#include "pch.h"
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include "IBinary.h"
#include "httputil.h"
#include "comptr.h"
#include <bcrypt.h>

//#include <ntstatus.h>

#pragma comment(lib, "bcrypt.lib") // over VISTA

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)Status) >= 0)
#endif

#define KEYLENGTH_128   (128 * 0x10000)        // 128-bit長
#define KEYLENGTH_256   (256 * 0x10000)        // 256-bit長

#define ENCRYPT_BLOCK_SIZE 16 // aes size

namespace sybil {


bool aes128ex( bool IsEncrypt, byte* pwd, int pwdlen, IStream* src, IStream** dst );

static IBinary _hash( BYTE* p, DWORD plen, int typ )
{
	BYTE* ret = nullptr;
	
	const WCHAR* hash_type[] = { BCRYPT_MD5_ALGORITHM, BCRYPT_SHA1_ALGORITHM, BCRYPT_SHA256_ALGORITHM, BCRYPT_SHA384_ALGORITHM, BCRYPT_SHA512_ALGORITHM };
	DWORD len[] = { 16,20,32,48,64 };

	_ASSERT( len > 0 );
	_ASSERT( 0<=typ && typ <=4);

	BCRYPT_ALG_HANDLE hAlgorithm;// CNGのアルゴリズムプロバイダのハンドル
	BCRYPT_HASH_HANDLE hHash;// ハッシュ値算出用オブジェクトのハンドル
	unsigned long ObjectSize= 0;// ハッシュオブジェクトのサイズ
	unsigned long Result = 0; // 作業用
	unsigned int HashLength = 0; // ハッシュ値のサイズ
	


	BCryptOpenAlgorithmProvider(&hAlgorithm, hash_type[typ], MS_PRIMITIVE_PROVIDER, 0);

	// ハッシュ値算出用のオブジェクトのサイズを取得する
	BCryptGetProperty( hAlgorithm, BCRYPT_OBJECT_LENGTH, (PUCHAR)(&ObjectSize), sizeof(ObjectSize),&Result,0);
	auto pHashObject = new byte[ObjectSize];
	// ハッシュ値のサイズを取得する
	BCryptGetProperty(hAlgorithm, BCRYPT_HASH_LENGTH, (PUCHAR)(&HashLength), sizeof(HashLength), &Result, 0);
	auto pHashValue = new byte[HashLength];
	
	BCryptCreateHash(hAlgorithm, &hHash, pHashObject, ObjectSize, nullptr, 0, 0);
	BCryptHashData(hHash, p,plen, 0);
	BCryptFinishHash(hHash, pHashValue, HashLength, 0);
	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlgorithm, 0);

	delete [] pHashObject;
	return IBinary(IBinaryMk(pHashValue, HashLength));
}
bool Hash( const IBinary& src, int typ, IBinary* ret )
{
	*ret = _hash( IBinaryPtr(src), IBinaryLen(src), typ );

	return true;
}

IBinary Password16byte(LPCWSTR pwd)
{	
	LPCSTR cb = ToUtf8(pwd);
	IBinary r =  _hash( (byte*)cb, strlen(cb), 0 );
	delete [] cb;
	return r;
}


bool EncryptAES128( IBinary* src,  LPCWSTR pwd, IBinary* ret )
{
	IStream* dst;
	auto _pwd = Password16byte(pwd);
	
	IStream* ssm = ToIStream( *src );
	bool bl = false;
	if ( aes128ex( true, (byte*)IBinaryPtr(_pwd), IBinaryLen(_pwd), ssm, &dst ))
	{
		*ret = ToIBinary(dst);
		dst->Release();		
		bl = true;
	}
	
	ssm->Release();
	return bl;
}
bool DecrpytAES128( IBinary* src,  LPCWSTR pwd, IBinary* ret )
{
	IStream* dst;
	auto _pwd = Password16byte(pwd);
	
	IStream* ssm = ToIStream( *src );
	bool bl = false;
	if ( aes128ex( false, (byte*)IBinaryPtr(_pwd), IBinaryLen(_pwd), ssm, &dst ))
	{
		*ret = ToIBinary(dst);
		dst->Release();		
		bl = true;
	}
	
	ssm->Release();
	return bl;
}

bool EncryptAES128( IStream* ssm,  LPCWSTR pwd, IStream** ret )
{
	ComPTR<IStream> dst;

	auto _pwd = Password16byte(pwd);
	
	
	IStreamSeekToTop(ssm);

	if ( aes128ex( true, (byte*)IBinaryPtr(_pwd), IBinaryLen(_pwd), ssm, &dst ))
	{
		dst.p->AddRef();

		*ret = dst;
		return true;
	}
	
	return false;
}




bool aes128ex( bool IsEncrypt, byte* pwd, int pwdlen, IStream* src, IStream** dst )
{
	bool ret = false;
	BCRYPT_ALG_HANDLE hAlgorithm;// CNGのアルゴリズムプロバイダのハンドル
	BCRYPT_KEY_HANDLE hKey;
	DWORD             dwKeyObjectSize;
	LPBYTE            lpKeyObject;
	LPBYTE            lpData;
	DWORD             dwDataSize;
	DWORD             dwResult;
	NTSTATUS          status;
	//TCHAR             szFileName[] = TEXT("sample.dat");
	//TCHAR             szData[] = TEXT("sample-data");
	//TCHAR             szPassword[] = TEXT("password");
	//BOOL              bEncrypt = TRUE;

	


	_ASSERT( pwdlen == 16 ); // 32:AES256 , 16:AES128

	//struct AesKeyBlob
	//{
	//	BLOBHEADER hdr;
	//	DWORD keySize;
	//	BYTE bytes[16];
	//};


	//AesKeyBlob blob;
	//memset( &blob, 0, sizeof(blob));
	//blob.hdr.bType = PLAINTEXTKEYBLOB;
	//blob.hdr.bVersion = CUR_BLOB_VERSION;
	//blob.hdr.aiKeyAlg = CALG_AES_128;
	//blob.keySize = pwdlen;
	//memcpy(blob.bytes, pwd, pwdlen);
	
	unsigned long IVLength;

	status = BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_AES_ALGORITHM, nullptr, 0);

	status = BCryptGetProperty(hAlgorithm, BCRYPT_OBJECT_LENGTH, (LPBYTE)&dwKeyObjectSize, sizeof(DWORD), &dwResult, 0);

	status = BCryptGetProperty(hAlgorithm, BCRYPT_BLOCK_LENGTH, (unsigned char*)(&IVLength), sizeof(IVLength), &dwResult, 0);

	BCRYPT_KEY_LENGTHS_STRUCT KeyLength = {0};
	status = BCryptGetProperty(hAlgorithm, BCRYPT_KEY_LENGTHS, (unsigned char*)(&KeyLength), sizeof(KeyLength), &dwResult,0);

	LPBYTE pKeyObject = new byte[dwKeyObjectSize];
	LPBYTE pIV = new byte[IVLength];
	for( UINT i = 0; i < IVLength; i++ ) pIV[i] = pwd[i]; //memset(pIV,0, IVLength);

	lpKeyObject = new byte[dwKeyObjectSize];

	status = BCryptGenerateSymmetricKey(hAlgorithm, &hKey, pKeyObject, dwKeyObjectSize, pwd, pwdlen,0 ); // KeyLength.dwMaxLength, 0); // KeyLength.dwMaxLength,0);

	if (IsEncrypt)
	{
		IBinary bins = ToIBinary(src);
		lpData = IBinaryPtr(bins);
		dwDataSize = IBinaryLen(bins);
		
		DWORD  dwEncryptDataSize = ((dwDataSize - 1) / ENCRYPT_BLOCK_SIZE + 1) * ENCRYPT_BLOCK_SIZE;
		LPBYTE lpEncryptData = new byte[dwEncryptDataSize];
				
		status = BCryptEncrypt(hKey, lpData, dwDataSize, NULL, pIV, IVLength, lpEncryptData, dwEncryptDataSize, &dwEncryptDataSize, BCRYPT_BLOCK_PADDING|BCRYPT_PAD_PKCS1);

		if (NT_SUCCESS(status))
		{
			IBinary encryptBin(IBinaryMk(lpEncryptData, dwEncryptDataSize));
	
			*dst = ToIStream(encryptBin);			
			ret = true;
		}
		else
			delete [] lpEncryptData;
	}
	else
	{			
		//DWORD  dwReadByte;

		LPBYTE lpEncryptData;
		DWORD  dwEncryptDataSize;

		LPBYTE lpData;
		DWORD  dwDataSize, dwDecryptDataSize;

		IBinary bins = ToIBinary(src);
		lpEncryptData = IBinaryPtr(bins);
		dwEncryptDataSize = IBinaryLen(bins);
		dwDataSize = dwEncryptDataSize;
	
		//status = BCryptDecrypt(hKey, lpEncryptData, dwEncryptDataSize, NULL, pIV, IVLength, NULL, 0, &dwDataSize, 0);
		lpData = new byte[dwEncryptDataSize];
		status = BCryptDecrypt(hKey, lpEncryptData, dwEncryptDataSize, NULL, pIV, IVLength, lpData, dwDataSize, &dwDecryptDataSize, BCRYPT_BLOCK_PADDING| BCRYPT_PAD_PKCS1);

		if (NT_SUCCESS(status)) 
		{
			IBinary encryptBin(IBinaryMk(lpData, dwDecryptDataSize));
			*dst = ToIStream(encryptBin);
			ret = true;
			
		}	
		else
		{
			DWORD xx = status;
			if (0xC000003EL == xx )
			{
				int a = 0;
			}

			delete[] lpData;
		}
	}
	
	delete[] lpKeyObject;
	delete[] pIV;
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlgorithm, 0);

	
	return ret;
}



static BSTR HexString( BYTE* p, UINT len )
{
	std::wstring rval;

	WCHAR buf[16];

	for( UINT i = 0; i < len; i++ )
	{
		swprintf_s( buf,16, L"%02x",  *(p++) );
		rval += buf;
	}
	return ::SysAllocString( rval.c_str());			
}

void BinaryToStr( const IBinary& src, int type, BSTR* ret )
{
	_ASSERT( type == 0 );
	BSTR s = HexString( IBinaryPtr(src), IBinaryLen(src) );


	*ret = s;
}
void MD5( LPCWSTR text, BSTR* md5 )
{	

	IBinary src, d1,s1;
	
	StrToUtf8( text, wcslen(text), &s1 ); 
	Hash( s1, 0, &d1 );
		

	BinaryToStr( d1, 0, md5 );





}

LPCSTR ToUtf8( LPCWSTR str )
{
	int cblen = ::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), 0,0,0,0);
	
	byte* cb = new byte[cblen+1];

	::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), (LPSTR)cb,cblen,0,0);

	cb[cblen]=0;

	return (LPCSTR)cb;
}

bool StrToUtf8( LPCWSTR str, int len, IBinary* pbin )
{
	int cblen = ::WideCharToMultiByte( CP_UTF8, 0, str,len, 0,0,0,0);
	byte* cb = new byte[cblen+1];
	::WideCharToMultiByte( CP_UTF8, 0, str,wcslen(str), (LPSTR)cb,cblen,0,0);
	cb[cblen]=0;
	IBinary bin( IBinaryMk(cb,cblen));
	*pbin =bin;
	return true;
}
bool Utf8ToBSTR( const IBinary& utf8, BSTR* ret )
{
	int cblen = ::MultiByteToWideChar( CP_UTF8,0, (LPSTR)IBinaryPtr(utf8), IBinaryLen(utf8), nullptr,0);
	BSTR r = ::SysAllocStringLen(NULL,cblen);
	cblen = ::MultiByteToWideChar( CP_UTF8,0, (LPSTR)IBinaryPtr(utf8), IBinaryLen(utf8), r, cblen);
	*ret = r;
	return (cblen >= 0);
}
bool AsciiToBSTR( const IBinary& asc, BSTR* ret )
{
	int cblen = ::MultiByteToWideChar( CP_ACP,0, (LPSTR)IBinaryPtr(asc), IBinaryLen(asc), nullptr,0);
	BSTR r = ::SysAllocStringLen(NULL,cblen);
	cblen = ::MultiByteToWideChar( CP_ACP,0, (LPSTR)IBinaryPtr(asc), IBinaryLen(asc), r, cblen);
	*ret = r;
	return (cblen >= 0);
}

bool IBinaryToBSTR( const IBinary& bin, LPCWSTR content_type, BSTR* ret )
{
	std::wstring ct = content_type;

	if ( (int)ct.find(L"text") > -1 )
	{
		if ( (int)ct.find(L"utf8") > -1 )
			return Utf8ToBSTR(bin,ret);
		else
			return AsciiToBSTR(bin,ret);
	}
	return false;
}
//bool Base64ToBinary( LPCWSTR str, int len, IBinary* pbin )
//{
//	DWORD blen;
//	BOOL bl = CryptStringToBinary(str,len,CRYPT_STRING_BASE64, NULL, &blen, NULL, NULL);
//	_ASSERT(bl);
//	IBinary bin( IBinaryMk(0,blen));
//	bl = CryptStringToBinary(str, len,CRYPT_STRING_BASE64, IBinaryPtr(bin), &blen, NULL, NULL);
//	_ASSERT(bl);
//	*pbin =bin;
//	return true;
//}
//bool BinaryToBase64( const IBinary& bin, BSTR* ret )
//{
//	DWORD len;
//	BOOL bl = CryptBinaryToString(IBinaryPtr(bin), IBinaryLen(bin),CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &len );
//	_ASSERT(bl);
//	BSTR r = ::SysAllocStringLen(NULL,len);
//	bl = CryptBinaryToString(IBinaryPtr(bin), IBinaryLen(bin),CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, (LPWSTR)r, &len );
//	_ASSERT(bl);
//	*ret = r;	
//	return bl;
//}

IBinary Clone( const IBinary& src )
{
	DWORD len = IBinaryLen(src);
	BYTE* p = new BYTE[len];
	memcpy( p, IBinaryPtr(src), len);

	return IBinary( IBinaryMk(p,len));
}

bool ReadFile( LPCWSTR fnm, IBinary* ret )
{
	HANDLE h = ::CreateFile2( fnm, GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING,nullptr );//,FILE_ATTRIBUTE_NORMAL,NULL );	

	if ( INVALID_HANDLE_VALUE != h )
	{		
		ComPTR<IStream> is;
		auto r = CreateStreamOnHGlobal(NULL, FALSE, &is);

		DWORD dw,dw2;
				
		byte cb[256];
		DWORD len = 256;
		while( ::ReadFile(h, cb, len, &dw, nullptr ) && dw > 0 )
			is->Write(cb, dw, &dw2);

		::CloseHandle(h);

		*ret = ToIBinary(is);
		
		return true;
	}
	return false;
}
bool WriteFile( LPCWSTR fnm, const IBinary& src, int typ )
{
	bool bNew  = false;
	CREATEFILE2_EXTENDED_PARAMETERS pms = {0};
	pms.dwSize = sizeof(pms);
	pms.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

	HANDLE h = ::CreateFile2( fnm, GENERIC_WRITE,0,OPEN_EXISTING,&pms );
	if ( INVALID_HANDLE_VALUE != h )
	{
		LARGE_INTEGER x;
		x.QuadPart = 0;
		SetFilePointerEx(h,x,nullptr, FILE_END);
	}
	else if ( ERROR_FILE_NOT_FOUND == ::GetLastError())
	{
		h = ::CreateFile2( fnm, GENERIC_WRITE,0,CREATE_ALWAYS,&pms );
		bNew = true;
	}
	
	DWORD dw = 0;

	if ( INVALID_HANDLE_VALUE != h )
	{
		if ( typ == 1 && bNew )
		{
			// BOM utf8
			BYTE bom[] = {0xEF, 0xBB, 0xBF};
			::WriteFile(h, bom, 3, &dw, nullptr );
		}		
		::WriteFile(h, IBinaryPtr(src), IBinaryLen(src), &dw, nullptr );
		::CloseHandle(h);
		return true;
	}
	return false;
}
IStream* ToIStream( const IBinary& src )
{
	ComPTR<IStream> is;
	auto r = CreateStreamOnHGlobal(NULL,FALSE,&is );
	
	DWORD dw;
	is->Write(IBinaryPtr(src), IBinaryLen(src), &dw );
	
	IStreamSeekToTop(is);
	is.p->AddRef();

	return is;
}
IBinary ToIBinary( IStream* bin )
{
	DWORD cblen = IStreamLength(bin);
	IStreamSeekToTop( bin );
	
	BYTE* cb = new BYTE[cblen];
	ULONG read;

	bin->Read(cb, cblen, &read);


	return IBinary(IBinaryMk(cb, read));
}
void IStreamSeekToTop(IStream* sm )
{
	ULARGE_INTEGER len;
	LARGE_INTEGER s;
	s.QuadPart=0;

    sm->Seek(s,STREAM_SEEK_SET,&len);

	
}
DWORD IStreamLength(IStream* sm )
{
	ULARGE_INTEGER len;
	LARGE_INTEGER s;
	s.QuadPart=0;

    sm->Seek(s,STREAM_SEEK_END,&len);


	return (DWORD)(0xFFFFFFFF & len.QuadPart);
}


bool DecryptAES128( IStream* ssm,  LPCWSTR pwd, IStream** ret )
{
	ComPTR<IStream> dst;

	auto _pwd = Password16byte(pwd);
	
	IStreamSeekToTop(ssm);

	if ( aes128ex( false, IBinaryPtr(_pwd), IBinaryLen(_pwd), ssm, &dst ))
	{
		dst.p->AddRef();		
		*ret = dst;
		return true;
	}
	
	return false;
}

static BYTE EN_whex( BYTE ch )
{	
	if ( 48 <= ch && ch <=57 )		// '0' - '9'
		ch -= 48;
	else if ( 65 <= ch && ch <= 90 )	// 'A' - 'Z'
		ch -= 55;
	else if ( 97 <= ch && ch <= 122 )	// 'a' - 'z'
		ch -= 87;	
	return ch;
}
#define MAKEWCHAR(a,b)	(a|(b<<4))

static WCHAR No3Decode( WCHAR* pby )
{
	// (1110aaaa 10bbbbcc 10ccdddd)UTF-8 ... (aaaabbbb ccccdddd)UTF-16
	
	
	//WCHAR ch1 = ((*pby++) << 12);
	//WCHAR ch2 = ((*pby++)&~0X80) << 6;
	//WCHAR ch3 = ((*pby++)&~0X80);		
	
	WCHAR ch1 = (*pby++) & (wchar_t)0x00ff;   /* 00000000 1110aaaa */
	WCHAR ch2 = (*pby++) & (wchar_t)0x00ff;   /* 00000000 10bbbbcc */
	WCHAR ch3 = (*pby++) & (wchar_t)0x00ff;   /* 00000000 10ccdddd */

	ch1 <<= 12;                               /* aaaa???? ???????? */
	ch1 &= 0xf000;                            /* aaaa0000 00000000 */
	ch2 <<= 6;                                /* 0010bbbb cc?????? */
	ch2 &= 0x0fc0;                            /* 0000bbbb cc000000 */
	ch3 &= 0x003f;                            /* 00000000 00ccdddd */

		
	return (ch1 | ch2 | ch3);
}
static WCHAR No2Decode( WCHAR* pby )
{
	// (110aaabb 10bbcccc)UTF-8 ... (00000aaa bbbbcccc)UTF-16
	
//	WCHAR ch1 = ((*pby++)&~0xc0) << 6;
//	WCHAR ch2 = ((*pby++)&~0X80);
	
	WCHAR wcWork1 = (*pby++) & (wchar_t)0x00ff;   /* 00000000 110aaabb */
	WCHAR wcWork2 = (*pby++) & (wchar_t)0x00ff;   /* 00000000 10bbcccc */
	wcWork1 <<= 6;                                /* 00110aaa bb?????? */
	wcWork1 &= 0x07c0;                            /* 00000aaa bb000000 */
	wcWork2 &= 0x003f;                            /* 00000000 00bbcccc */
			
	return (wcWork1 | wcWork2);
}


bool PercentEncode( LPCWSTR str, int len, BSTR* ret )
{
	TCHAR* ps = (TCHAR*)str;
	WCHAR* buf = new WCHAR[len + 1];
	WCHAR* pby = buf;
	
	while( *ps )
	{
		if ( *ps == L'%' )
		{
			ps++;
			BYTE ch1 = EN_whex( (BYTE)*ps++ );
			BYTE ch2 = EN_whex( (BYTE)*ps++ );
			WCHAR outch = MAKEWCHAR( ch2,ch1 );
				
			*pby++ = outch;									
		}
		else if ( *ps == L'+' )
		{
			*pby++ = L' '; ps++;
		}	
		else
		{
			*pby++ = *ps++;
		}
	}
	*pby = 0;
	
	pby = buf;

	std::wstringstream wsm;
	while( *pby )
	{
		if ( *pby <= 127 )
		{			
			wsm << (WCHAR)((*pby++)&(wchar_t)0x00ff);			
		}
		else if ( 0xDF < *pby && *pby <= 0xEF )
		{				
			wsm << No3Decode( pby );
			pby += 3;				
		}
		else if ( *pby <= 0xDF )
		{
			wsm << No2Decode( pby ); // NO2のデコードに該当する日本語はないので、ここにはこない
			pby += 2;											
		}
	}
	
	delete [] buf;

	*ret = ::SysAllocString( wsm.str().c_str() );
	return true;
}


//void BSTRPtr::split( LPCWSTR splitstr, BSTR* left, BSTR* right ) const
//{
//	int split_len = wcslen(splitstr );
//	_ASSERT( 0 < split_len && split_len <= 2 );
//	WCHAR ch1 = splitstr[0];
//	WCHAR* p = bs_;
//
//	int is=0 ,ie = 0;
//	while( *p )
//	{
//		if ( split_len == 2 && *p == ch1)
//		{
//			if ( *(p+1) == splitstr[1] )
//			{
//				ch1 = splitstr[1];
//				p++;
//			}
//		}
//		
//		if ( *p == ch1 )
//		{
//			*left = substring(is,ie);
//
//			is = ie + split_len;
//			int elen = length()-is;
//			*right = substring(is,elen);
//			return;
//		}
//		ie++;
//		p++;
//	}
//
//	*left = ::SysAllocString(bs_);
//	*right = ::SysAllocString(L"");
//}


DWORD thread::create_thread( std::function<void()> f )
{
	DWORD id;

	st* ps = new st();
	ps->fs_ = f;

	HANDLE h = ::CreateThread(NULL,NULL,Rap, (LPVOID)ps, 0,&id);
	
	hs_.push_back(h);

	return id;
}
DWORD CALLBACK thread::Rap( LPVOID p )
{
	std::unique_ptr<st> m((st*)p);
	
	m->fs_();
	
	return 0;
}

void thread::join_all()
{
	HANDLE* hs = new HANDLE[hs_.size()];
	int i = 0;
	for( auto& h : hs_ )
		hs[i++] = h;

	::WaitForMultipleObjects( i, hs, TRUE, INFINITE );

	delete [] hs;

}




#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)

IBinary HMACSHA256(const IBinary& _key, const IBinary& _data )
{
	const BYTE* key = IBinaryPtr(_key);
	DWORD keySize = IBinaryLen(_key);
	const BYTE* message = IBinaryPtr(_data);
	DWORD messageSize = IBinaryLen(_data);
	
	BCRYPT_ALG_HANDLE       hAlg = NULL;
	BCRYPT_HASH_HANDLE      hHash = NULL;
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD                   cbData = 0, cbHashObject = 0;
	PBYTE                   pbHashObject = NULL;
	PBYTE                   pbHash = NULL;
	bool                    error = false;
	DWORD					cbHash = 0;

	//open an algorithm handle
	if (0!=(status = BCryptOpenAlgorithmProvider(&hAlg,BCRYPT_SHA256_ALGORITHM,	NULL,BCRYPT_ALG_HANDLE_HMAC_FLAG)))
	{
		error = true;
		goto Cleanup;
	}

	//calculate the size of the buffer to hold the hash object
	if (0!=(status = BCryptGetProperty(hAlg,BCRYPT_OBJECT_LENGTH,(PBYTE)&cbHashObject,sizeof(DWORD),&cbData,0)))
	{
		error = true;
		goto Cleanup;
	}

	//allocate the hash object on the heap
	pbHashObject = new byte[cbHashObject];
	
	//calculate the length of the hash
	if (0!=(status = BCryptGetProperty(hAlg,BCRYPT_HASH_LENGTH,(PBYTE)&cbHash,sizeof(DWORD),&cbData,0)))
	{
		error = true;
		goto Cleanup;
	}

	////allocate the hash buffer on the heap
	pbHash = new byte[cbHash];
	
	//create a hash
	if (0!=(status = BCryptCreateHash(hAlg,&hHash,pbHashObject,cbHashObject,(PBYTE)key,keySize,	0)))
	{
		error = true;
		goto Cleanup;
	}

	//hash some data
	if (0!=(status = BCryptHashData(hHash,(PBYTE)message,messageSize,0)))
	{
		error = true;
		goto Cleanup;
	}

	//close the hash
	if (0!=(status = BCryptFinishHash(hHash,pbHash, cbHash,0)))
	{
		error = true;
		goto Cleanup;
	}

Cleanup:

	if (hAlg)
		BCryptCloseAlgorithmProvider(hAlg, 0);

	if (hHash)
		BCryptDestroyHash(hHash);
	
	delete [] pbHashObject;

	if (error)
	{
		delete[] pbHash;
	}

	IBinary bin(IBinaryMk(pbHash, cbHash));

	return bin;
}





} // sybil



























