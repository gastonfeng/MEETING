program title;

uses
  Forms,
  make in 'make.pas' {makeform},
  mabout in 'mabout.pas' {AboutBox};

{$R *.RES}

begin
  Application.Initialize;
  Application.HelpFile := 'E:\meeting\voten\Vote.hlp';
  Application.Title := '��Ļ��������';
  Application.CreateForm(Tmakeform, makeform);
  Application.CreateForm(TAboutBox, AboutBox);
  Application.Run;
end.
