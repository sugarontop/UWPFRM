  
declare function CreateWindow(me:any, ty:string,title:string, x:number,y:number,cx:number,cy:number ):void;
declare function SetWindowText( me:any, text:string ): void;
declare function GetWindowText( me:any ): string;

type click_delegate =  (me: any) => void;
type prchanged_delegate = (me:any, s:string) => void;
type changed_delegate = (me:any, s:string) => void;

module U {
	export class Button {
		Click : click_delegate;
		
		constructor(title: string) {
			CreateWindow( this, 'button',title,100,500,300,26);
			//SetWindowText(this, title);
		}

		OnClick():void{ 
			if ( this.Click )
				this.Click(this);
		}

		SetText( s: string ): void {			
			SetWindowText(this, s);
		}

		GetText(): string {
			return GetWindowText(this);
		}
	} 

	export class Textbox {
		
		TextChanged : changed_delegate;

		constructor(title: string, x: number,y:number, cx:number) {
			CreateWindow(this, 'textbox', title, x, y, cx,26);
		}

		SetText( s: string ): void {			
			SetWindowText(this, s);

			if ( this.TextChanged )
				this.TextChanged( this, s );
		}
		GetText(): string {
			return GetWindowText(this);
		}

		get TextLength() : number {
			return GetWindowText(this).length;
		}
		
	}
}

//------------------------------------------------------------------------

var gY = 560;
var gS = "one line ";
function test(obj: any): void {
	
	gY += 40;
	var s = gS + "Y:" + gY;

	var t2 = new U.Textbox(s, 100,gY,300 );
}

var obj =  new U.Button("new Textbox");
obj.Click = test;





