


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

function GetUrl( cd ) {

	// realtimeデータを取得する場合は、alphavantageでapikeyを取得する必要がある。
	//return "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol="+cd+"&apikey=&datatype=csv";
	
	return "https://raw.githubusercontent.com/sugarontop/UWPFRM/master/sybil/FrameTestApp1/" + cd + ".csv";

	//return "http://localhost/" + cd + ".csv";
}