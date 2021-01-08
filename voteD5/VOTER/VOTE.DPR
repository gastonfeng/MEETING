program vote;

uses
  Forms,
  windows,
  main in 'main.pas' {mainform},
  option in 'option.pas' {optionform},
  about in 'about.pas' {AboutBox},
  install in 'install.pas' {installform},
  cover in 'cover.pas' {coverform},
  bkdk in 'bkdk.pas' {bkdkform},
  port in 'port.pas',
  global in 'global.pas',
  junor in 'junor.pas',
  yydk in 'yydk.pas' {yydkform};

{$E .EXE}

{$R *.RES}
var hwndprev:hwnd;

begin
  hwndprev:=findwindow('Tmainform','会议表决系统');
  if hwndprev<>0 then
  begin
    setforegroundwindow(hwndprev);
//    application.Terminate;
  end;
  coverform:=tcoverform.create(Application);
  coverform.show;
  coverform.Update;
  Application.Initialize;
  Application.Title := '会议表决系统';
  Application.HelpFile := 'Vote.hlp';
  Application.CreateForm(Tmainform, mainform);
  Application.CreateForm(Tyydkform, yydkform);
  Application.Run;
end.
