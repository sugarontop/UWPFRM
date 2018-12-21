


function Initial() {
	var map = { 
	'ticker1' : 'MSFT', 
	'ticker2':'AAPL',
	'ticker3':'AMZN',
	'ticker4':'NVDA',
	'ticker5':'VGT',
	'ticker6':'TTWO',
	'ticker7':'SPYG',
	'ticker8':'DGRW',
	'_font':'arial',
	'_font_size':'12',
	'_padding_h' : '50',
	'_padding_l' : '50',
	'_cell_width' : [50,100]
	
	};

	return JSON.stringify(map);
}

function GroupProperty(){
	
	var map = {
		'no': 0,
		'typ':'textbox',
		'title':'スタティック',
		'value': '文字列',
		'readonly':true
	};
	var map2 = {
		'no': 1,
		'typ':'textbox',
		'title':'テキスト',
		'value': 'ここに入力',
		'readonly':false
	};
	var map3 = {
		'no': 2,
		'typ':'listbox',
		'title':'リストボックス',
		'value': 0,
		'items': ['aaa','bbb','ccc'],
		'readonly':false
		
	};
	var map4 = {
		'no': 3,
		'typ':'listbox',
		'title':'リストボックス',
		'value': 1,
		'items': ['ddd','eee','fff'],
		'readonly':false
		
	};

   var k = [];
    k.push(map);
    k.push(map2);
    k.push(map3);
    k.push(map4);
   
	return JSON.stringify(k);
}



function GetUrl( cd ) {

	// realtimeデータを取得する場合は、alphavantageでapikeyを取得する必要がある。
	//return "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol="+cd+"&apikey=NJNKAPWMU598EUCU&datatype=csv";
	
	//return "https://raw.githubusercontent.com/sugarontop/UWPFRM/master/sybil/FrameTestApp1/" + cd + ".csv";

	return "http://localhost/" + cd + ".csv";
}