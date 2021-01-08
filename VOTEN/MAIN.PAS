unit main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Menus, ToolWin, ComCtrls, Buttons, ExtCtrls, StdCtrls, CPDrv,inifiles,
  port,global,junor, MShape;

type
  Tmainform = class(TForm)
    MainMenu1: TMainMenu;
    N1: TMenuItem;
    N3: TMenuItem;
    N4: TMenuItem;
    N6: TMenuItem;
    StatusBar1: TStatusBar;
    Image1: TImage;
    ToolBar: TToolBar;
    baodao: TSpeedButton;
    dmuu: TSpeedButton;
    bcjt: TSpeedButton;
    tsji: TSpeedButton;
    tools: TSpeedButton;
    closebtn: TSpeedButton;
    N10: TMenuItem;
    N14: TMenuItem;
    N15: TMenuItem;
    N16: TMenuItem;
    N17: TMenuItem;
    N18: TMenuItem;
    N19: TMenuItem;
    N21: TMenuItem;
    N20: TMenuItem;
    CommPortDriver1: TCommPortDriver;
    Panel1: TPanel;
    ch1: TSpeedButton;
    ch2: TSpeedButton;
    ch3: TSpeedButton;
    ch4: TSpeedButton;
    memo1: TMemo;
    N22: TMenuItem;
    N23: TMenuItem;
    N24: TMenuItem;
    N25: TMenuItem;
    N2: TMenuItem;
    SpeedButton2: TSpeedButton;
    SpeedButton3: TSpeedButton;
    SpeedButton4: TSpeedButton;
    Button1: TButton;
    Button2: TButton;
    Timer1: TTimer;
    SpeedButton5: TSpeedButton;
    procedure closebtnClick(Sender: TObject);
    procedure dmuuClick(Sender: TObject);
    procedure bcjtClick(Sender: TObject);
    procedure tsjiClick(Sender: TObject);
    procedure baodaoClick(Sender: TObject);
    procedure toolsClick(Sender: TObject);
    procedure CommPortDriver1ReceivePacket(Sender: TObject;
      Packet: Pointer; DataSize: Cardinal);
    procedure ch1Click(Sender: TObject);
    procedure ch2Click(Sender: TObject);
    procedure ch3Click(Sender: TObject);
    procedure ch4Click(Sender: TObject);
    procedure SpeedButton1Click(Sender: TObject);
    procedure N24Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure SpeedButton2Click(Sender: TObject);
    procedure SpeedButton3Click(Sender: TObject);
    procedure SpeedButton4Click(Sender: TObject);
    procedure N21Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure SpeedButton5Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure N2Click(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

type buftype=record
  boot:byte;
  address:byte;
  num:byte;
  status:byte;
  baodao:byte;
  zancheng:byte;
  fandui:byte;
  qiquan:byte;
  weian:byte;
  ch:byte;
  checksum:byte;
end;

procedure loadconfig;

var
  mainform: Tmainform;
  buf:buftype;
  Hdll:THandle;
  DLL:boolean;
  BMP0:array[0..11] of char ='SYSTEM0.BMP';
  BMP1:array[0..11] of char ='SYSTEM1.BMP';
  BMP2:array[0..11] of char ='SYSTEM2.BMP';
  BMP3:array[0..11] of char ='SYSTEM3.BMP';
  BMPT:array[0..11] of char ='SYSTEMT.BMP';
  bitmap:Tpicture;
  man:integer;
  address:byte=0;
  controller:array[0..128] of byte;
  voter:array[0..128] of byte;
  controllernum:byte=0;
  mypath:string;
  controllerpoint:byte;
  start,prompt,sum,begintitle,endtitle,user1,user2,user3:string;
  inifile:tinifile;
  com:string;
  CARD_EXIST:boolean;
  ratio:boolean;
  defaultfont:string;
  defaultcolor:integer;
implementation

  uses QTTHUNKU, option, install, cover, about, bkdk;

{$R *.DFM}

procedure loadconfig;
begin
  inifile:=TIniFile.Create(mypath+inifilename);
  com:=inifile.readstring('PORT','COM','COM2');
  start:=inifile.readString('TITLE','START','');
  user1:=inifile.readString('TITLE','USER1','');
  user2:=inifile.readString('TITLE','USER2','');
  user3:=inifile.readString('TITLE','USER3','');
  prompt:=inifile.readString('TITLE','PROMPT','');
  sum:=inifile.readString('TITLE','SUM','');
  begintitle:=inifile.readString('TITLE','BEGIN','');
  endtitle:=inifile.readString('TITLE','END','');
  ratio:=inifile.readbool('OPTION','ratio',true);
  defaultfont:=inifile.readstring('OPTION','FONT','');
  defaultcolor:=inifile.readinteger('OPTION','COLOR',$ff0000);
  mainform.timer1.interval:=inifile.ReadInteger('PORT','WAIT',5000);
  inifile.free;
end;

procedure display(filename:string);
var f:Tfilestream;
    s:string[7];
    i:integer;
    p:Tpanel;
    m:tmultishape;
begin
  filename:=lowercase(filename);
  s:=ExtractFileExt(filename);
  if s='.bmp' then
  begin
    bitmap:=tpicture.create;
    bitmap.LoadFromFile(filename);
    bitmap.Bitmap.Height:=480;
    bitmap.Bitmap.Width:=800;
    bitmap.Bitmap.PixelFormat:=pf8bit;
    mainform.image1.Picture.Bitmap.Canvas.StretchDraw(mainform.image1.canvas.cliprect,bitmap.graphic);
    bitmap.SaveToFile(mypath+BMPT);
    bitmap.free;
    if CARD_EXIST then call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPAscal,[0,0,BMPT],[2,2,succ(strlen(BMPT))]);
  end
  else
  begin
    if s='.vdf' then
    begin
      f:=Tfilestream.Create(filename,fmopenread);
      f.Read(s,6);
      s[0]:=chr(5);
      if s='DF1.0' then
      begin
       bitmap:=tpicture.create;
       bitmap.Bitmap.Height:=480;
       bitmap.Bitmap.Width:=800;
       bitmap.Bitmap.PixelFormat:=pf8bit;
       p:=tpanel.Create(Application);
       f.ReadComponent(p);
       bitmap.Bitmap.Canvas.brush.Color:=clblue;
       bitmap.Bitmap.Canvas.FillRect(bitmap.bitmap.canvas.cliprect);
       f.Read(i,sizeof(integer));
       m:=tmultishape.create(Application);
       for i:=i downto 1 do
       begin
         f.ReadComponent(m);
         bitmap.Bitmap.Canvas.Font.Name:=m.font.name;
         bitmap.Bitmap.Canvas.Font.Size:=(m.font.size*80)div 48;
         bitmap.Bitmap.Canvas.Font.Color:=m.font.color;
         bitmap.Bitmap.Canvas.TextOut(m.left*80 div 48,m.Top*80 div 48,m.text);
       end;
       mainform.image1.Picture.Bitmap.Canvas.StretchDraw(mainform.image1.canvas.cliprect,bitmap.graphic);
       bitmap.SaveToFile(BMPT);
       bitmap.free;
       if CARD_EXIST then call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPAscal,[0,0,BMPT],[2,2,succ(strlen(BMPT))]);
       m.free;
       p.free;
      end
      else showmessage('文件格式错误!');
      f.free;
    end
    else showmessage(filename+'未能打开或者格式不对!');
  end;
end;

procedure Tmainform.closebtnClick(Sender: TObject);
begin
  close;
end;

procedure Tmainform.dmuuClick(Sender: TObject);
begin
  baodao.Enabled:=false;
  dmuu.Enabled:=false;
  bcjt.Enabled:=false;
  tsji.Enabled:=false;
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($e4);
  commportdriver1.SendByte($cc);
  memo1.Lines.Add('正在统计报到人数:');
//  bitmap.Bitmap.Canvas.Font.Size:=100;
//  bitmap.Bitmap.Canvas.Brush.Color:=clblue;
//  bitmap.Bitmap.Canvas.Font.Color:=clwhite;
//  bitmap.Bitmap.Canvas.FillRect(bitmap.Bitmap.canvas.cliprect);
//  bitmap.Bitmap.Canvas.TextOut(50,200,'正在统计报到人数...');
//  image1.Picture.Bitmap.Canvas.StretchDraw(image1.canvas.cliprect,bitmap.graphic);
//  call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPascal,[0,0,BMP1],[2,2,succ(strlen(BMP1))]);
  display(sum);
  timer1.Enabled:=true;
end;

procedure Tmainform.bcjtClick(Sender: TObject);
begin
  baodao.Enabled:=false;
  dmuu.Enabled:=false;
  bcjt.Enabled:=false;
  tsji.Enabled:=true;
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($e5);
  commportdriver1.SendByte($cc);
  memo1.Lines.Add('进入表决状态.');
  display(begintitle);
//  bitmap.LoadFromFile(BMP2);
//  image1.Picture.Bitmap.Canvas.StretchDraw(image1.canvas.cliprect,bitmap.graphic);
//  call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPascal,[0,0,BMP2],[2,2,succ(strlen(BMP2))]);
end;

procedure Tmainform.tsjiClick(Sender: TObject);
begin
  baodao.Enabled:=false;
  dmuu.Enabled:=false;
  bcjt.Enabled:=false;
  tsji.Enabled:=false;
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($e6);
  commportdriver1.SendByte($cc);
  display(endtitle);
//  display(begintitle);
//  memo1.Lines.Add('统计表决结果:');
//  bitmap.LoadFromFile(BMP3);
//  image1.Picture.Bitmap.Canvas.StretchDraw(image1.canvas.cliprect,bitmap.graphic);
//  call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPascal,[0,0,BMP3],[2,2,succ(strlen(BMP3))]);
  timer1.Enabled:=true;
end;

procedure Tmainform.baodaoClick(Sender: TObject);
begin
  baodao.Enabled:=false;
  dmuu.Enabled:=true;
  bcjt.Enabled:=false;
  tsji.Enabled:=false;
  memo1.Lines.Add('进入报到状态.');
//  bitmap.LoadFromFile(BMP1);
//  image1.Picture.Bitmap.Canvas.StretchDraw(image1.canvas.cliprect,bitmap.graphic);
//  call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPascal,[0,0,BMP1],[2,2,succ(strlen(BMP1))]);
  display(prompt);
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($e3);
  commportdriver1.SendByte($cc);
end;

procedure Tmainform.toolsClick(Sender: TObject);
begin
  optionform:=toptionform.create(application);
  optionform.showmodal;
end;

procedure Tmainform.CommPortDriver1ReceivePacket(Sender: TObject;
  Packet: Pointer; DataSize: Cardinal);
  var buftemp:^buftype;
      i:byte;
begin
  timer1.Enabled:=false;
  if datasize <>11 then exit;
  buftemp:=packet;
  if buftemp.boot<>$ab then exit;
  if byte(buftemp.boot+buftemp.address+buftemp.num+buftemp.status+buftemp.baodao+buftemp.zancheng+buftemp.fandui+buftemp.qiquan+buftemp.weian+buftemp.ch)<>buftemp.checksum then exit;
  messagebeep(0);
  buf.address:=buftemp.address;
  buf.ch:=buftemp.ch;
  buf.status:=buftemp.status;
  case buf.status of
    0:;
    1:;
    2:begin
        buf.baodao:=buftemp.baodao;
        memo1.Lines.Add('报到人数:'+inttostr(buf.baodao));
        bitmap:=tpicture.create;
        bitmap.Bitmap.Height:=480;
        bitmap.Bitmap.Width:=800;
        bitmap.Bitmap.PixelFormat:=pf8bit;
        bitmap.Bitmap.Canvas.Font.Size:=100;
        bitmap.Bitmap.Canvas.Font.Name:='楷体';
        bitmap.Bitmap.Canvas.Brush.Color:=clblue;
        bitmap.Bitmap.Canvas.Font.Color:=clwhite;
        bitmap.Bitmap.Canvas.FillRect(bitmap.Bitmap.canvas.cliprect);
        bitmap.Bitmap.Canvas.TextOut(50,200,'报到人数:'+inttostr(buf.baodao));
        image1.Picture.Bitmap.Canvas.StretchDraw(image1.canvas.cliprect,bitmap.graphic);
        bitmap.SaveToFile(BMPT);
        bitmap.Free;
        if CARD_EXIST then call16bitroutine('JUN_WRITEBITMAP4',hdll,ccPascal,[0,0,BMPT],[2,2,succ(strlen(BMPT))]);
        baodao.Enabled:=true;
        dmuu.Enabled:=false;
        bcjt.Enabled:=true;
        tsji.Enabled:=false;
        man:=buf.baodao;
        button2.Caption:='报到人数:'+inttostr(man)+'人';
//      display(sum);
    end;
    3:;
    4:begin
        buf.zancheng:=buftemp.zancheng;
        buf.fandui:=buftemp.fandui;
        buf.qiquan:=buftemp.qiquan;
        buf.weian:=buftemp.weian;
        if buf.zancheng+buf.fandui+buf.qiquan>man then showmessage('表决结果无效!票数不得超过报到人数.')
        else
        begin
          bitmap:=tpicture.create;
          bitmap.Bitmap.Height:=480;
          bitmap.Bitmap.Width:=800;
          bitmap.Bitmap.PixelFormat:=pf8bit;
          bitmap.Bitmap.Canvas.Font.Name:=defaultfont;
          bitmap.Bitmap.Canvas.Brush.Color:=defaultcolor;
          bitmap.Bitmap.Canvas.FillRect(bitmap.bitmap.canvas.cliprect);
          bitmap.Bitmap.Canvas.Font.Size:=72;
          bitmap.Bitmap.Canvas.font.Color:=clwhite;
          bitmap.Bitmap.Canvas.TextOut(180,5,'表决结果:');
          bitmap.Bitmap.Canvas.Font.Size:=64;
          bitmap.Bitmap.Canvas.Font.Color:=clgreen;
          if ratio then
          begin
            bitmap.bitmap.canvas.TextOut(60,105,'赞成:'+inttostr(buf.zancheng)+'票');
            bitmap.Bitmap.Canvas.TextOut(500,105,formatfloat('0.0',buf.zancheng*100/man)+'%');
            bitmap.Bitmap.Canvas.Font.Color:=clred;
            bitmap.Bitmap.Canvas.TextOut(60,190,'反对:'+inttostr(buf.fandui)+'票');
            bitmap.Bitmap.Canvas.TextOut(500,190,formatfloat('0.0',buf.fandui*100/man)+'%');
            bitmap.Bitmap.Canvas.Font.Color:=clyellow;
            bitmap.Bitmap.Canvas.TextOut(60,275,'弃权:'+inttostr(buf.qiquan)+'票');
            bitmap.Bitmap.Canvas.TextOut(500,275,formatfloat('0.0',buf.qiquan*100/man)+'%');
            bitmap.Bitmap.Canvas.TextOut(60,360,'未按:'+inttostr(man-buf.zancheng-buf.fandui-buf.qiquan)+'票');
            bitmap.Bitmap.Canvas.TextOut(500,360,formatfloat('0.0',(man-buf.zancheng-buf.fandui-buf.qiquan)*100/man)+'%');
           end
          else
          begin
            bitmap.bitmap.canvas.TextOut(210,105,'赞成:'+inttostr(buf.zancheng)+'票');
            bitmap.Bitmap.Canvas.Font.Color:=clred;
            bitmap.Bitmap.Canvas.TextOut(210,190,'反对:'+inttostr(buf.fandui)+'票');
            bitmap.Bitmap.Canvas.Font.Color:=clyellow;
            bitmap.Bitmap.Canvas.TextOut(210,275,'弃权:'+inttostr(buf.qiquan)+'票');
            bitmap.Bitmap.Canvas.TextOut(210,360,'未按:'+inttostr(man-buf.zancheng-buf.fandui-buf.qiquan)+'票');
          end;
          image1.Picture.Bitmap.Canvas.StretchDraw(image1.canvas.cliprect,bitmap.graphic);
          bitmap.SaveToFile(BMPT);
          bitmap.Free;
          if CARD_EXIST then call16bitroutine('JUN_WRITEBITMAP4',hdll,ccpascal,[0,0,BMPT],[2,2,succ(strlen(BMPT))]);
        end;
        baodao.Enabled:=true;
        dmuu.Enabled:=false;
        bcjt.Enabled:=true;
        tsji.Enabled:=false;
    end;
    5:begin
        buf.num:=buftemp.num;
        voter[buf.address]:=buf.num;
        installform.Label2.Caption:='表决器连接数量:'+inttostr(buf.num);
      end;
    6:begin
        buf.num:=buftemp.num;
        memo1.Lines.Add(inttostr(buf.address)+'#控制器连接表决器数量:'+inttostr(buf.num));
        button1.Caption:='表决器数量:'+inttostr(buf.num);
      end;
    7:begin
        for i:=0 to 128 do
          if controller[i]=0 then break;
        controllerpoint:=i;
        commportdriver1.sendbyte($ab);
        commportdriver1.sendbyte(i);
        commportdriver1.sendbyte($f0);
        commportdriver1.sendbyte($cc);
        installform.Memo5.Lines.Add('发送控制器编号'+inttostr(i));
      end;
    8:begin
        installform.Memo5.Lines.Add('控制器连接数量:'+inttostr(controllerpoint+1)+'L:'+inttostr(buf.address));
        controller[controllerpoint]:=1;
      end;
    128:begin
          showmessage('无可用的编号!');
        end;
    129:begin
          showmessage('编号错误!');
        end;
  end;
  case buf.ch of
    1:ch1.Down:=true;
    2:ch2.Down:=true;
    3:ch3.Down:=true;
    4:ch4.Down:=true;
  end;
end;

procedure Tmainform.ch1Click(Sender: TObject);
begin
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($f1);
  commportdriver1.SendByte($cc);
  timer1.Enabled:=true;
end;

procedure Tmainform.ch2Click(Sender: TObject);
begin
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($f2);
  commportdriver1.SendByte($cc);
  timer1.Enabled:=true;
end;

procedure Tmainform.ch3Click(Sender: TObject);
begin
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($f3);
  commportdriver1.SendByte($cc);
  timer1.Enabled:=true;
end;

procedure Tmainform.ch4Click(Sender: TObject);
begin
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($f4);
  commportdriver1.SendByte($cc);
  timer1.Enabled:=true;
end;

procedure Tmainform.SpeedButton1Click(Sender: TObject);
begin
  commportdriver1.SendByte($ab);
  commportdriver1.SendByte(address);
  commportdriver1.SendByte($d0);
  commportdriver1.SendByte($cc);
  timer1.Enabled:=true;
end;

procedure Tmainform.N24Click(Sender: TObject);
begin
  installform:=tinstallform.create(application);
  installform.showmodal;
end;

procedure Tmainform.FormCreate(Sender: TObject);
var i:integer;
begin
  inifile:=TIniFile.Create(mypath+inifilename);
  inifile.readInteger('POSITION','LEFT',mainform.left);
  inifile.readInteger('POSITION','TOP',mainform.top);
  inifile.readInteger('POSITION','WIDTH',mainform.width);
  inifile.readInteger('POSITION','HEIGHT',mainform.height);
  inifile.free;
  loadconfig;
  baodao.Enabled:=true;
  dmuu.Enabled:=false;
  bcjt.Enabled:=false;
  tsji.Enabled:=false;
  commportdriver1.PortName:=com;
  if commportdriver1.Connect then memo1.Lines.Add('串行口正常打开.')
  else showmessage('串行口未能打开,请检查设置!');
  i:=(smallint(call16bitroutine('JUN_Initialize',hdll,ccPascal,[0],[0])));
  if i=1 then memo1.Lines.Add('字幕卡正常初始化.')
    else memo1.lines.Add('字幕卡返回代码:'+inttostr(i));
  if JUN_JUNIOR_EXIST then CARD_EXIST:=true else CARD_EXIST:=false;
  display(start);
  speedbutton1click(mainform);
  coverform.hide;
  coverform.free;
end;

procedure Tmainform.SpeedButton2Click(Sender: TObject);
begin
  display(user1);
end;

procedure Tmainform.SpeedButton3Click(Sender: TObject);
begin
  display(user2);
end;

procedure Tmainform.SpeedButton4Click(Sender: TObject);
begin
  display(user3);
end;

procedure Tmainform.N21Click(Sender: TObject);
begin
  aboutbox:=taboutbox.create(application);
  aboutbox.showmodal;
end;

procedure Tmainform.Button2Click(Sender: TObject);
begin
  bkdkform:=tbkdkform.create(application);
  bkdkform.showmodal;
end;

procedure Tmainform.Timer1Timer(Sender: TObject);
begin
  Timer1.Enabled:=false;
  showmessage('控制器未响应!请检查电源是否打开.');
end;

procedure Tmainform.SpeedButton5Click(Sender: TObject);
begin
  display(mypath+'clear.vdf');
end;

procedure Tmainform.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  inifile:=TIniFile.Create(mypath+inifilename);
  inifile.WriteInteger('POSITION','LEFT',mainform.left);
  inifile.WriteInteger('POSITION','TOP',mainform.top);
  inifile.WriteInteger('POSITION','WIDTH',mainform.width);
  inifile.WriteInteger('POSITION','HEIGHT',mainform.height);
  inifile.free;
end;

procedure Tmainform.N2Click(Sender: TObject);
begin
  optionform:=toptionform.create(application);
  optionform.PageControl1.ActivePage:=optionform.TabSheet2;
  optionform.showmodal;
end;

procedure Tmainform.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  if Messagedlg('确实要退出系统吗?',mtconfirmation,[mbok,mbcancel],0)=mrcancel
  then canclose:=false;
end;



initialization
  mypath:=extractfilepath(application.exename);
  hdll := LoadLib16(mypath+'juniort.DLL');
finalization
  mainform.commportdriver1.Disconnect;
  {if CARD_EXIST then }call16bitroutine('JUN_EXIT',hdll,ccPascal,[0],[0]);
  FreeLibrary16(hdll);
end.

