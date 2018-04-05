  
declare function CreateWindow(me:any, ty:string,title:string, x:number,y:number,cx:number,cy:number ):void;
declare function SetWindowText( me:any, text:string ): void;
declare function GetWindowText( me:any ): string;

type click_delegate =  (me: any) => void;
type prchanged_delegate = (me:any, s:string) => void;
type changed_delegate = (me:any, s:string) => void;

export module U {
	export class Button {

		Click : click_delegate;
		
		constructor(title: string,x: number,y:number, cx:number) {
			CreateWindow( this, 'button',title,x,y,cx,26);
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






