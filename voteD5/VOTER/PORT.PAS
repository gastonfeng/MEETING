unit Port;
interface
function PortReadByte(Addr:Word) : Byte;
function PortReadWord(Addr:Word) : Word;
function PortReadWordLS(Addr:Word) : Word;
procedure PortWriteByte(Addr:Word; Value:Byte);
procedure PortWriteWord(Addr:Word; Value:Word);
procedure PortWriteWordLS(Addr:Word; Value:Word);
implementation
{*
* Port Read byte function
*Parameter:port address
*Return: byte value from given port
*}
function PortReadByte(Addr:Word) : Byte; assembler; register;
asm
MOV DX,AX
IN AL,DX
end;
{*
* HIGH SPEED Port Read Word function
* Parameter: port address
* Return: word value from given port
* Comment:may problem with some cards and computers that
can't to access whole word, usualy it works.
*}
function PortReadWord(Addr:Word) : Word; assembler; register;
asm
MOV DX,AX
IN AX,DX
end;
{*
* LOW SPEED Port Read Word function
* Parameter: port address
*Return:word value from given port
*Comment:work in cases,only to adjust DELAY if need
*}
function PortReadWordLS(Addr:Word) : Word; assembler; register;
const
Delay = 150;
// depending of CPU speed and cards speed
asm
MOV DX,AX
IN AL,DX
//read LSB port
MOV ECX,Delay
@1:
LOOP @1 //delay between two reads
XCHG AH,AL
INC DX
//port+1
IN AL,DX //read MSB port
XCHG AH,AL //restore bytes order
end;
{* Port Write byte function*}
procedure PortWriteByte(Addr:Word; Value:Byte); assembler; register;
asm
XCHG AX,DX
OUT DX,AL
end;
{*
* HIGH SPEED Port Write word procedure
* Comment:may problem with some cards and computers that
can't to access whole word, usualy it works.
*}
procedure PortWriteWord(Addr:word; Value:word); assembler; register;
asm
XCHG AX,DX
OUT DX,AX
end;
{*
* LOW SPEED Port Write Word procedure
*}
procedure PortWriteWordLS(Addr:word; Value:word); assembler; register;
const
Delay = 150;
// depending of CPU speed and cards speed
asm
XCHG AX,DX
OUT DX,AL
MOV ECX,Delay
@1:
LOOP @1
XCHG AH,AL
INC DX
OUT DX,AL
end;
end.
