#pragma once

std::wstring InetGet( std::wstring url, std::map<std::wstring,std::wstring>& header );
bool InetGetResult( std::wstring key, sybil::ResponseData** ret );



int CreateRectBox( V4::D2DControls* ctrl, const FRectFBM& rc );


struct XST
{
	int tag;
	BSTR data;
};