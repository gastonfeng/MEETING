unit bkdk;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, StdCtrls;

type
  Tbkdkform = class(TForm)
    Label1: TLabel;
    Edit1: TEdit;
    UpDown1: TUpDown;
    Button1: TButton;
    Button2: TButton;
    procedure FormActivate(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  bkdkform: Tbkdkform;

implementation

uses main;

{$R *.DFM}

procedure Tbkdkform.FormActivate(Sender: TObject);
begin
  edit1.Text:=inttostr(man);
  updown1.Max:=buf.num;
end;

procedure Tbkdkform.Button1Click(Sender: TObject);
begin
  if strtoint(edit1.Text)>buf.num then showmessage('�����������ܴ��ڱ���������!')
  else
  begin
    man:=strtoint(edit1.text);
    mainform.Button2.Caption:='��������:'+inttostr(man)+'��';
    DateTime:=time;
    mainform.RichEdit1.Lines.Add(timetostr(DateTime)+': ��������'+inttostr(man)+'��');
    mainform.bcjt.enabled:=true;
    modalresult:=mrok;
  end;
end;

end.