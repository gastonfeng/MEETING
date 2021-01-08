unit input;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ColorGrd, Buttons,mshape;

type
  Tinputform = class(TForm)
    Memo1: TMemo;
    ComboBox1: TComboBox;
    ColorGrid1: TColorGrid;
    Button1: TButton;
    Button2: TButton;
    ComboBox2: TComboBox;
    Label1: TLabel;
    Label2: TLabel;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure ComboBox2Change(Sender: TObject);
    procedure ColorGrid1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormActivate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  inputform: Tinputform;

implementation

uses make;

{$R *.DFM}

procedure Tinputform.Button2Click(Sender: TObject);
begin
  modalresult:=1;
end;

procedure Tinputform.Button1Click(Sender: TObject);
var bitmap:tbitmap;
    i:integer;
begin
  filechanged:=true;
  if memo1.Text='' then
  begin
    if memo1.tag<>-1 then
    begin
      tmultishape(clist.Items[memo1.tag]).free;
      clist.remove(clist.items[memo1.tag]);
      for i:=0 to clist.Count-1 do
        tmultishape(clist.Items[i]).tag:=i;
    end;
  end
  else begin
  if memo1.tag=-1 then
    i:=clist.add(Tmultishape.create(self))
  else i:=memo1.tag;
  with Tmultishape(clist.items[i]) do
  begin
    tag:=i;
    parent:=makeform.Panel1;
    shapetype:=mstext;
    font.color:=colorgrid1.foregroundcolor;
    font.name:=combobox1.text;
    font.size:=strtoint(combobox2.text);
    text:=memo1.lines.text;
    bitmap:=tbitmap.Create;
    bitmap.canvas.Font.Name:=combobox1.text;
    bitmap.canvas.Font.Size:=strtoint(combobox2.text);
    height:=bitmap.canvas.Textextent(memo1.lines.text).cy;
    width:=bitmap.canvas.Textextent(memo1.lines.text).cx;
    bitmap.Free;
    repeatmode:=rpnone;
    left:=xx;
    top:=yy;
    Onmousedown:=makeform.mousedown;
    onmousemove:=makeform.mousemove;
    onmouseup:=makeform.mouseup;
    cursor:=crhandpoint;
  end;
  end;
//  makeform.image1.canvas.Brush.Color:=colorgrid1.BackgroundColor;
//  makeform.image1.canvas.FillRect(makeform.image1.canvas.ClipRect);
//  makeform.image1.canvas.font.color:=colorgrid1.ForegroundColor;
//  makeform.Image1.Canvas.Font.Name:=combobox1.Text;
//  makeform.Image1.Canvas.Font.Size:=strtoint(combobox2.text);
//  makeform.Image1.Canvas.TextOut(xx,yy,memo1.lines.text);
  draopflag:=false;
  modalresult:=1;
end;

procedure Tinputform.ComboBox1Change(Sender: TObject);
begin
  memo1.Font.Name:=combobox1.Text;
end;

procedure Tinputform.ComboBox2Change(Sender: TObject);
begin
  memo1.Font.Size:=strtoint(combobox2.text);
end;

procedure Tinputform.ColorGrid1Click(Sender: TObject);
begin
  memo1.Font.Color:=colorgrid1.ForegroundColor;
end;

procedure Tinputform.FormCreate(Sender: TObject);
begin
  combobox1.Items:=screen.Fonts;
  combobox1.Style:=cssimple;
  combobox1.Text:='ËÎÌו';
  combobox1.Style:=csdropdownlist;
  memo1.Font.Name:=combobox1.text;
  combobox2.ItemIndex:=5;
  memo1.Font.Size:=strtoint(combobox2.text);
  memo1.Font.Color:=colorgrid1.ForegroundColor;
end;

procedure Tinputform.FormActivate(Sender: TObject);
begin
  memo1.SetFocus;
end;

end.
