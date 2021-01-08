unit make;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ExtCtrls, StdCtrls, ComCtrls, ColorGrd, Menus, Buttons, ToolWin, MShape;

type
  Tmakeform = class(TForm)
    ColorDialog1: TColorDialog;
    FontDialog1: TFontDialog;
    StatusBar1: TStatusBar;
    MainMenu1: TMainMenu;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    N4: TMenuItem;
    N5: TMenuItem;
    N6: TMenuItem;
    N11: TMenuItem;
    N13: TMenuItem;
    N14: TMenuItem;
    N15: TMenuItem;
    N16: TMenuItem;
    ToolBar1: TToolBar;
    open: TSpeedButton;
    save: TSpeedButton;
    N17: TMenuItem;
    SaveDialog1: TSaveDialog;
    OpenDialog1: TOpenDialog;
    Panel1: TPanel;
    SpeedButton1: TSpeedButton;
    SpeedButton2: TSpeedButton;
    SpeedButton3: TSpeedButton;
    SpeedButton4: TSpeedButton;
    procedure MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure MouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure Setbackgroungcolor(Sender: TObject);
    procedure filesave(Sender: TObject);
    procedure fileopen(Sender: TObject);
    procedure Panel1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure filenew(Sender: TObject);
    procedure N15Click(Sender: TObject);
    procedure callhelp(Sender: TObject);
    procedure fileclose(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  makeform: Tmakeform;
  xx,yy:integer;
  clist:tlist;
  DraopFlag,filechanged:Boolean;
  FLeft,FTop:integer;
  mypath:string;

implementation

uses input, mabout;

{$R *.DFM}

procedure Tmakeform.MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if button=mbleft then
    begin
        DraopFlag:=True;
        FLeft:=X;
        FTop:=Y;
    end;
  if button=mbright then
  begin
    draopflag:=false;
    inputform.ComboBox1.Style:=cssimple;
    inputform.ComboBox1.Text:=tmultishape(sender).font.name;
    inputform.ComboBox1.Style:=csdropdownlist;
    inputform.ComboBox2.Style:=cssimple;
    inputform.ComboBox2.Text:=inttostr(tmultishape(sender).font.size);
    inputform.ComboBox2.Style:=csdropdownlist;
    inputform.Memo1.Lines.Text:=tmultishape(sender).text;
    inputform.Memo1.tag:=tmultishape(sender).tag;
    inputform.Memo1.Font:=tmultishape(sender).font;
    inputform.Memo1.SelectAll;
    inputform.showmodal;
  end;
end;

procedure Tmakeform.MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  if DraopFlag=True then
  begin
    if Tmultishape(sender).Left+X-fleft+tmultishape(sender).width>panel1.width then tmultishape(sender).left:=panel1.width-tmultishape(sender).width
    else if Tmultishape(sender).Left+X-fleft<0 then tmultishape(sender).left:=0
    else Tmultishape(sender).Left:=Tmultishape(sender).Left+X-fleft;
    if tmultishape(sender).Top+Y-ftop+tmultishape(sender).height>panel1.height then tmultishape(sender).top:=panel1.height-tmultishape(sender).height
    else if tmultishape(sender).Top+Y-ftop<0 then tmultishape(sender).top:=0
    else tmultishape(sender).Top:=tmultishape(sender).Top+Y-ftop;
  end;
end;

procedure Tmakeform.MouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if Button=mbleft then
  begin
    DraopFlag:=False;
  end;
end;

procedure Tmakeform.Setbackgroungcolor(Sender: TObject);
begin
  if colordialog1.Execute then
  begin
    panel1.Color:=colordialog1.Color;
  end;
end;

procedure Tmakeform.filesave(Sender: TObject);
  var f:tfilestream;
      i:byte;
begin
  savedialog1.InitialDir:=mypath;
  if savedialog1.Execute then
  begin
    f:=tfilestream.create(savedialog1.filename,fmcreate);
    f.Write('VDF1.0',6);
    f.WriteComponent(panel1);
    f.Write(clist.count,sizeof(clist.count));
    for i:=0 to clist.count-1 do
      f.WriteComponent(tmultishape(clist.items[i]));
    f.Free;
  end;
  filechanged:=false;
end;

procedure Tmakeform.fileopen(Sender: TObject);
var f:tfilestream;
    ss:string[7];
    sss,sss1:integer;
    i:integer;
begin
  opendialog1.InitialDir:=mypath;
  if opendialog1.Execute then
  begin
    f:=Tfilestream.create(opendialog1.filename,fmopenread);
    f.read(ss,6);
//    showmessage(ss);
    ss[0]:=chr(5);
    if ss='DF1.0' then
    begin
      if clist.Count>0 then
      begin
        for i:=0 to clist.count-1 do
          tmultishape(clist.Items[i]).free;
        clist.clear;
      end;
      f.ReadComponent(panel1);
      f.Read(sss,sizeof(clist.count));
    try
        for sss1:=1 to sss do
        begin
        i:=clist.add(Tmultishape.create(panel1));
        f.ReadComponent(Tmultishape(clist.items[i]));
        with Tmultishape(clist.items[i])  do
          begin
            parent:=panel1;
            repaint;
            Onmousedown:=makeform.mousedown;
            onmousemove:=makeform.mousemove;
            onmouseup:=makeform.mouseup;
          end;
        end;
    finally
    end;
    end
    else showmessage('文件格式错误!');
    f.Free;
    panel1.Repaint;
  end;
  filechanged:=false;
end;

procedure Tmakeform.Panel1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if button=mbleft then
  begin
    xx:=x;
    yy:=y;
    inputform.Memo1.tag:=-1;
    inputform.Memo1.Clear;
    inputform.showmodal;
  end;
end;

procedure Tmakeform.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
var i:integer;
begin
  if filechanged then
  begin
    i:=messagedlg('当前编辑的文件还没有保存!是否保存?',mtconfirmation,mbyesnocancel,0);
    case i of
      idyes:filesave(self);
      idcancel:canclose:=false;
    end;
  end;
end;

procedure Tmakeform.filenew(Sender: TObject);
var i:integer;
begin
  if filechanged then
  begin
    i:=messagedlg('是否保存当前文件?',mtconfirmation,mbyesnocancel,0);
    case i of
      idyes:filesave(self);
      idno:
        begin
          for i:=0 to clist.count-1 do
            tmultishape(clist.Items[i]).free;
          clist.Clear;
        end;
      idcancel:abort;
    end;
  end;
  for i:=0 to clist.count-1 do
    tmultishape(clist.Items[i]).free;
  clist.Clear;
  filechanged:=false;
end;

procedure Tmakeform.N15Click(Sender: TObject);
begin
  aboutbox.showmodal;
end;

procedure Tmakeform.callhelp(Sender: TObject);
begin
  application.HelpCommand(help_contents,0);
end;

procedure Tmakeform.fileclose(Sender: TObject);
begin
  close;
end;

initialization
  mypath:=extractfilepath(application.exename);
  clist:=Tlist.create;
  inputform:=Tinputform.create(makeform);
  filechanged:=false;
finalization
  inputform.free;
  clist.free;

end.
