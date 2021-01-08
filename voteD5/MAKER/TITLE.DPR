program title;

uses
  Forms,
  make in 'make.pas' {makeform},
  mabout in 'mabout.pas' {AboutBox},
  input in 'input.pas' {inputform};

{$R *.RES}

begin
  Application.Initialize;
  Application.HelpFile := 'Vote.hlp';
  Application.Title := '×ÖÄ»ÖÆ×÷³ÌÐò';
  Application.CreateForm(Tmakeform, makeform);
  Application.CreateForm(TAboutBox, AboutBox);
  Application.CreateForm(Tinputform, inputform);
  Application.Run;
end.
