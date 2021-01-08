unit option;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, StdCtrls,inifiles,main, ExtCtrls;

type
  Toptionform = class(TForm)
    PageControl1: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    ComboBox1: TComboBox;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Edit1: TEdit;
    Edit2: TEdit;
    Edit3: TEdit;
    Edit4: TEdit;
    Edit5: TEdit;
    Edit6: TEdit;
    Edit7: TEdit;
    Edit8: TEdit;
    Button4: TButton;
    Button5: TButton;
    Button6: TButton;
    Button7: TButton;
    Button8: TButton;
    Button9: TButton;
    Button10: TButton;
    Button11: TButton;
    OpenDialog1: TOpenDialog;
    TabSheet3: TTabSheet;
    GroupBox1: TGroupBox;
    CheckBox1: TCheckBox;
    GroupBox2: TGroupBox;
    ComboBox2: TComboBox;
    Label10: TLabel;
    Label11: TLabel;
    Panel1: TPanel;
    ColorDialog1: TColorDialog;
    Label12: TLabel;
    UpDown1: TUpDown;
    waittime: TEdit;
    Label13: TLabel;
    CheckBox2: TCheckBox;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Panel1Click(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  optionform: Toptionform;
  ComChange:boolean=false;
const
  inifilename:shortstring='VOTE.INI';

implementation

{$R *.DFM}

procedure Toptionform.Button2Click(Sender: TObject);
begin
  MOdalResult:=1;
end;

procedure Toptionform.Button1Click(Sender: TObject);
begin
  Button3Click(button1);
  ModalResult:=1;
end;

procedure Toptionform.FormCreate(Sender: TObject);
begin
  combobox2.Items:=screen.Fonts;
  panel1.Color:=defaultcolor;
  colordialog1.Color:=panel1.Color;
  checkbox1.Checked:=ratio;
  CheckBox2.Checked:=Sound;
  combobox1.Text:=com;
  edit1.Text:=start;
  edit2.Text:=user1;
  edit3.Text:=user2;
  edit4.Text:=user3;
  edit5.text:=prompt;
  edit6.text:=sum;
  edit7.Text:=begintitle;
  edit8.Text:=endtitle;
  combobox2.Text:=defaultfont;
  updown1.Position:=mainform.timer1.interval;
end;

procedure Toptionform.Button4Click(Sender: TObject);
begin
  opendialog1.InitialDir:=mypath;
  if opendialog1.Execute then
  begin
    if sender=button4 then edit1.Text:=opendialog1.FileName;
    if sender=button5 then edit2.Text:=opendialog1.FileName;
    if sender=button6 then edit3.Text:=opendialog1.FileName;
    if sender=button7 then edit4.Text:=opendialog1.FileName;
    if sender=button8 then edit5.Text:=opendialog1.FileName;
    if sender=button9 then edit6.Text:=opendialog1.FileName;
    if sender=button10 then edit7.Text:=opendialog1.FileName;
    if sender=button11 then edit8.Text:=opendialog1.FileName;
  end;
end;

procedure Toptionform.Button3Click(Sender: TObject);
begin
  inifile:=TIniFile.Create(mypath+inifilename);
  inifile.writestring('PORT','COM',combobox1.text);
  inifile.WriteString('TITLE','START',edit1.text);
  inifile.WriteString('TITLE','USER1',edit2.text);
  inifile.WriteString('TITLE','USER2',edit3.text);
  inifile.WriteString('TITLE','USER3',edit4.text);
  inifile.WriteString('TITLE','PROMPT',edit5.text);
  inifile.WriteString('TITLE','SUM',edit6.text);
  inifile.WriteString('TITLE','BEGIN',edit7.text);
  inifile.WriteString('TITLE','END',edit8.text);
  inifile.WriteBool('OPTION','ratio',checkbox1.Checked);
  inifile.WriteBool('OPTION','PROMPT',checkbox2.Checked);
  inifile.WriteString('OPTION','FONT',combobox2.text);
  inifile.Writeinteger('OPTION','COLOR',panel1.color);
  inifile.WriteInteger('PORT','WAIT',strtoint(waittime.text));
  inifile.free;
  main.loadconfig;
  if ComChange then
  begin
    if mainform.CommPortDriver1.Connected then mainform.CommPortDriver1.Disconnect;
    mainform.CommInit;
  end;
end;

procedure Toptionform.Panel1Click(Sender: TObject);
begin
  if colordialog1.Execute then panel1.Color:=colordialog1.Color;
end;

procedure Toptionform.ComboBox1Change(Sender: TObject);
begin

  ComChange:=true;
end;

end.
