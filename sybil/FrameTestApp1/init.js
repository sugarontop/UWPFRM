var U;
(function (U) {
    var Button = /** @class */ (function () {
        function Button(title) {
            CreateWindow(this, 'button', title, 100, 500, 300, 26);
            //SetWindowText(this, title);
        }
        Button.prototype.OnClick = function () {
            if (this.Click)
                this.Click(this);
        };
        Button.prototype.SetText = function (s) {
            SetWindowText(this, s);
        };
        Button.prototype.GetText = function () {
            return GetWindowText(this);
        };
        return Button;
    }());
    U.Button = Button;
    var Textbox = /** @class */ (function () {
        function Textbox(title, x, y, cx) {
            CreateWindow(this, 'textbox', title, x, y, cx, 26);
        }
        Textbox.prototype.SetText = function (s) {
            SetWindowText(this, s);
            if (this.TextChanged)
                this.TextChanged(this, s);
        };
        Textbox.prototype.GetText = function () {
            return GetWindowText(this);
        };
        Object.defineProperty(Textbox.prototype, "TextLength", {
            get: function () {
                return GetWindowText(this).length;
            },
            enumerable: true,
            configurable: true
        });
        return Textbox;
    }());
    U.Textbox = Textbox;
})(U || (U = {}));
//------------------------------------------------------------------------
var gY = 560;
var gS = "one line ";
function test(obj) {
    gY += 40;
    var s = gS + "Y:" + gY;
    var t2 = new U.Textbox(s, 100, gY, 300);
}
var obj = new U.Button("new Textbox");
obj.Click = test;
//# sourceMappingURL=init.js.map