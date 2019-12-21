
#pragma once

#define WM_D2D_RESTRUCT_RENDERTARGET			(WM_APP+1) // wp:0 release, wp:1 create
#define WM_D2D_OPEN_SLIDE_MENU					(WM_APP+20)
#define WM_D2D_OPEN_SLIDE_MENU_EX				(WM_APP+21)
#define WM_D2D_INIT_UPDATE						(WM_APP+22)
#define WM_D2D_COMBOBOX_CHANGED					(WM_APP+23)
#define WM_D2D_CLOSE_WINDOW						(WM_APP+24)

#define WM_D2D_PROPERTY_LIST_JSON				(WM_APP+25)

#define WM_D2D_SCROLLBAR_SIZE					(WM_APP+26)


#define WM_D2D_COMMAND							(WM_APP+27)
#define WM_D2D_INTERNET_GET_COMPLETE			(WM_APP+28)


#define WM_D2D_TEXTBOX_CHANGED					(WM_APP+29)
#define WM_D2D_TEXTBOX_SETTEXT					(WM_APP+30)
#define WM_D2D_TEXTBOX_GETTEXT					(WM_APP+31)

#define WM_D2D_PREVIOUS_STATE					(WM_APP+32)

#define WM_D2D_TAB_DETACH						(WM_APP+33)
#define WM_D2D_TAB_ACTIVE						(WM_APP+34)

#define WM_D2D_LB_ADDITEM						(WM_APP+35)
#define WM_D2D_LB_COUNT							(WM_APP+36)
#define WM_D2D_LB_GET_SELECT_IDX				(WM_APP+37)
#define WM_D2D_LB_SET_SELECT_IDX				(WM_APP+38)
#define WM_D2D_LB_CLEAR							(WM_APP+39)
#define WM_D2D_LB_GET_ITEM						(WM_APP+40)
#define WM_D2D_LB_ADD_ITEMS_JSON				(WM_APP+41)

#define WM_D2D_LB_EVNT_SELECT_CHANGE			(WM_APP+42)


#define WM_D2D_MENU_ITEM_INSERT					(WM_APP+43)		// JSON


#define WM_D2D_VSCROLLBAR_SHOW					(WM_APP+44)		// 1:VISIBLE 0:HIDE
#define WM_D2D_HSCROLLBAR_SHOW					(WM_APP+45)		// 1:VISIBLE 0:HIDE

#define WM_D2D_CELLS_CONTROLBAR_SHOW			(WM_APP+46)
#define WM_D2D_CELLS_SELECT_CHANGED				(WM_APP+47)

#define WM_D2D_ESCAPE_FROM_CAPTURED				(WM_APP+48)

#define WM_D2D_RESIZE							(WM_APP+49)

#define WM_D2D_NCHITTEST						(WM_APP+50)		// ret: HTCLIENT(1),HTCAPTION(2)...
#define WM_D2D_MOUSEACTIVATE					(WM_APP+51)
#define WM_D2D_SETCURSOR						(WM_APP+52)

#define WM_D2D_SETFOCUS							(WM_APP+53)
#define WM_D2D_KILLFOCUS						(WM_APP+54)
#define WM_D2D_THREAD_COMPLETE							(WM_APP+55)

#define WM_D2D_APP_INIT						(WM_APP+56)		// wp:window size

#define WM_D2D_TEXTBOX_PUSHED_OPTIONKEY			(WM_APP+57)

#define WM_D2D_TEXTBOX_PRE_CHANGE				(WM_APP+58)	// ret:1は変更不可

#define WM_D2D_ATTACH							(WM_APP+59)	
#define WM_D2D_DETACH							(WM_APP+60)	

#define WM_D2D_SHOW_TOOLTIP						(WM_APP+61)	
#define WM_D2D_HIDE_TOOLTIP						(WM_APP+62)	

#define WM_D2D_UI_LOCK							(WM_APP+63)	// 
#define WM_D2D_UI_UNLOCK						(WM_APP+64)	


#define WM_D2D_USERCD							(WM_APP+150000) // 
	

#define WM_D2D_APP_ON_CREATE					(WM_APP+169383) //	(WM_APP+16383)まで

/////////////////////////////////////////////////////////////////////////////////////////////

#define WP_D2D_MAINFRAME_CLOSE						9999




#define CURSOR_ARROW					0
#define CURSOR_IBeam					1
#define CURSOR_Hand						2
#define CURSOR_SizeWestEast				3
#define CURSOR_SizeNorthwestSoutheast	4

