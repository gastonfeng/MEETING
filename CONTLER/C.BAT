set C51INC=e:\c51\inc
set C51LIB=e:\c51\lib
\c51\bin\c51 %1.c small db
\c51\bin\l51 %1.obj to %1.abs
\3h\oh %1.abs
\3h\cvf51 %1.abs