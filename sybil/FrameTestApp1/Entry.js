"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var Lib = require("./init");
var U = Lib.U;
var gY = 560;
var gS = "one line ";
function test(obj) {
    gY += 40;
    var s = gS + "Y:" + gY;
    var t2 = new U.Textbox(s, 100, gY, 300);
}
var obj = new U.Button("Create Textbox", 100, 500, 300);
obj.Click = test;
//# sourceMappingURL=Entry.js.map