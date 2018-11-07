


function Initial() {
	var map = { 
	'pm1' : 'MSFT', 
	'pm2':'AAPL',
	'pm3':'AMZN',
	'pm4':'NVDA',
	'pm5':'VGT',
	'pm6':'TTWO',
	'pm7':'SPYG',
	'pm8':'DGRW',
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
	//return "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol="+cd+"&apikey=[your api key]&datatype=csv";
	
	return "https://github.com/sugarontop/UWPFRM/tree/master/sybil/FrameTestApp1/" + cd + ".csv";
}