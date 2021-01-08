unit yydk;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, inifiles,ComCtrls;

type
  Tyydkform = class(TForm)
    Label1: TLabel;
    Edit1: TEdit;
    UpDown1: TUpDown;
    Button1: TButton;
    Button2: TButton;
    procedure Button1Click(Sender: TObject);
    procedure FormActivate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  yydkform: Tyydkform;

implementation

uses main,option;
{$R *.DFM}

procedure Tyydkform.Button1Click(Sender: TObject);
begin
  shouldman:=strtoint(edit1.text);
  mainform.button3.caption:='Ӧ������:'+inttostr(shouldman)+'��';
  inifile:=TIniFile.Create(mypath+inifilename);
  inifile.writestring('HUMAN','YYDK',edit1.text);
  inifile.free;

end;

procedure Tyydkform.FormActivate(Sender: TObject);
begin
  edit1.Text:=inttostr(shouldman);
end;

end.
