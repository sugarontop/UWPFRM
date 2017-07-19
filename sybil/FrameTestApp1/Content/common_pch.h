#include <MsXml6.h>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <set>
#include <functional>
#include <stack>
#include <ppltasks.h>






#define NONAME L"__NONAME"
#define OUT
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define D2RGBA(r,g,b,a) ColorF(r/255.0f, g/255.0f, b/255.0f, a/255.0f ) //  light(0) <- alpah <- deep(255)
#define D2RGB(r,g,b) ColorF(r/255.0f, g/255.0f, b/255.0f, 1.0f )
#define D2DRGB(dw) ColorF(((dw&0xFF0000)>>16)/255.0f, ((dw&0x00FF00)>>8)/255.0f, (dw&0xFF)/255.0f, 1.0f )
#define D2DRGBA(dw) ColorF(((dw&0xFF000000)>>24)/255.0f, ((dw&0x00FF0000)>>16)/255.0f, ((dw&0xFF00)>>8)/255.0f, (dw&0xFF)/255.0f )
#define D2DRGBADWORD(R,G,B,A) (LONG)((R<<24)+(G<<16)+(B<<8)+A)
