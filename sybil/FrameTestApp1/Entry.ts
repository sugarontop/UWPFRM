
import * as Lib from "./init";


var U = Lib.U;

var gY = 560;
var gS = "one line ";
function test(obj: any): void {
	
	gY += 40;
	var s = gS + "Y:" + gY;

	var t2 = new U.Textbox(s, 100,gY,300 );
}

var obj =  new U.Button("Create Textbox",100, 500, 300 );
obj.Click = test;