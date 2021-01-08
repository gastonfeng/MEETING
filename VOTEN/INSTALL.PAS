unit install;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls, ExtCtrls, Buttons,inifiles;

type
  Tinstallform = class(TForm)
    PageControl1: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    prev: TButton;
    next: TButton;
    ok: TButton;
    Memo1: TMemo;
    Memo2: TMemo;
    Memo3: TMemo;
    Label2: TLabel;
    TabSheet4: TTabSheet;
    Memo4: TMemo;
    Label3: TLabel;
    Memo5: TMemo;
    procedure nextClick(Sender: TObject);
    procedure prevClick(Sender: TObject);
    procedure okClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  installform: Tinstallform;

implementation

uses main;

{$R *.DFM}

procedure Tinstallform.nextClick(Sender: TObject);
var i:byte;
begin
  pagecontrol1.ActivePage:=pagecontrol1.FindNextPage(pagecontrol1.activepage,true,false);
  if pagecontrol1.ActivePage=tabsheet2 then
    begin
      prev.Enabled:=true;
      for i:=0 to 128 do controller[i]:=0;
      mainform.commportdriver1.sendbyte($ab);
      mainform.commportdriver1.sendbyte($ee);
      mainform.commportdriver1.sendbyte($ef);
      mainform.commportdriver1.sendbyte($cc);
      memo5.Lines.Add('等待控制器响应...');
    end;
  if pagecontrol1.ActivePage=tabsheet3 then
  begin
    for i:=controllernum to 128 do
      if controller[i]=1 then break;
    controllernum:=i;
    label3.Caption:='按下'+inttostr(controllernum+1)+'#控制器连接的所有表决器的任意键';
    mainform.CommPortDriver1.SendByte($ab);
    mainform.CommPortDriver1.SendByte(controllernum);
    mainform.CommPortDriver1.SendByte($e0);
    mainform.CommPortDriver1.SendByte($cc);
  end;
  if pagecontrol1.ActivePage=tabsheet4 then
  begin
    ok.Caption:='完成';
    ok.Enabled:=true;
    next.Enabled:=false;
  end;
end;

procedure Tinstallform.prevClick(Sender: TObject);
begin
  pagecontrol1.ActivePage:=pagecontrol1.FindNextPage(pagecontrol1.activepage,false,false);
  ok.Caption:='取消';
  if pagecontrol1.ActivePage=tabsheet1 then
  begin
    prev.Enabled:=false;
    next.Enabled:=true;
  end;
end;

procedure Tinstallform.okClick(Sender: TObject);
var myinifile:Tinifile;
    i:byte;
begin
  if pagecontrol1.ActivePage=tabsheet4 then
  begin
    myinifile:=tinifile.create(mypath+'vote.ini');
    myinifile.WriteInteger('CONTROLLER','NUMBER',controllernum);
    for i:=0 to 128 do
    begin
      if controller[i]=1 then myinifile.WriteInteger('CONTROLLER','CONTROLLER'+inttostr(i),voter[i]);
    end;
  end;
  modalresult:=1;
end;

end.
