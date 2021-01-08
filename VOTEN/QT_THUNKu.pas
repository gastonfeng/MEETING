unit QT_THUNKu;

{$R-,S-,Q-}

interface

  uses WINDOWS;
  type
    TShortHandle=Word;
    function WOWGetVDMPointer(vp,dwBytes:DWord;fProtectedMode:bool):pointer;stdcall;
    function WOWGetVDMPointerFix(vp,dwBytes:DWord;fProtectedMode:Bool):Pointer;stdcall;
    procedure WOWGetVDMPointerUnfix(vp:Dword);stdcall;
    function GlobalAlloc16(flags:integer;Bytes:Longint):Tshorthandle;stdcall;
    function Globallock16(MEM:tshorthandle):pointer;stdcall;
    function Globalunlock16(mem:tshorthandle):wordbool;stdcall;
    function globalfree16(mem:tshorthandle):tshorthandle;stdcall;
    function loadlibrary16(libfilename:pchar):thandle;stdcall;
    function GetProcAddress16(module:HModule;ProcName:PChar):TFarProc;stdcall;
    procedure freelibrary16(libmodule:thandle);stdcall;
    procedure QT_Thunk;

implementation

  uses
    sysutils,classes;

  type
    EVERSIONError=class(Exception);

    function WOWGetVDMPointer;external 'wow32.dll';
    function WOWGetVDMPointerFix;external 'wow32.dll';
    procedure WOWGetVDMPointerUnfix;external 'wow32.dll';

    function GlobalAlloc16;external 'kernel32.dll' index 24;
    function GlobalLock16;external 'kernel32.dll' index 25;
    function globalunlock16;external 'kernel32.dll' index 26;
    function globalfree16;external 'kernel32.dll' index 31;
    function loadlibrary16;external 'kernel32.dll' index 35;
    procedure freelibrary16;external 'kernel32.dll' index 36;
    function getprocaddress16;external 'kernel32.dll' index 37;

    procedure QT_Thunk; external 'kernel32.dll' index 559;

  initialization
    if win32platform<>ver_platform_win32_windows then
      raise eversionerror.Create('QT_Thunk only works under Windows 95!');
      
end.
 