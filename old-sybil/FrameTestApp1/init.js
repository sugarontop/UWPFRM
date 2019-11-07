"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var U;
(function (U) {
    var Button = /** @class */ (function () {
        function Button(title, x, y, cx) {
            CreateWindow(this, 'button', title, x, y, cx, 26);
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
})(U = exports.U || (exports.U = {}));
//------------------------------------------------------------------------
//# sourceMappingURL=init.js.map