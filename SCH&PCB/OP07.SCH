*version 1.0 1567504152
lib E:\msim\lib\\abm.slb 0
lib E:\msim\lib\\analog.slb 0
lib E:\msim\lib\\anlg_dev.slb 0
lib E:\msim\lib\\bipolar.slb 0
lib E:\msim\lib\\breakout.slb 0
lib E:\msim\lib\\burr_brn.slb 0
lib E:\msim\lib\\cd4000.slb 0
lib E:\msim\lib\\dig_1.slb 1709796238
lib E:\msim\lib\\dig_2.slb 0
lib E:\msim\lib\\dig_3.slb 0
lib E:\msim\lib\\dig_4.slb 0
lib E:\msim\lib\\dig_ecl.slb 0
lib E:\msim\lib\\dig_gal.slb 0
lib E:\msim\lib\\dig_misc.slb 0
lib E:\msim\lib\\dig_pal.slb 0
lib E:\msim\lib\\diode.slb 0
lib E:\msim\lib\\europe.slb 0
lib E:\msim\lib\\global.slb 0
lib E:\msim\lib\\harris.slb 0
lib E:\msim\lib\\jfet.slb 0
lib E:\msim\lib\\lin_tech.slb 0
lib E:\msim\lib\\marker.slb 0
lib E:\msim\lib\\misc.slb 0
lib E:\msim\lib\\mosfet.slb 0
lib E:\msim\lib\\opamp.slb 0
lib E:\msim\lib\\opto.slb 0
lib E:\msim\lib\\pwrbjt.slb 0
lib E:\msim\lib\\source.slb 0
lib E:\msim\lib\\special.slb 0
lib E:\msim\lib\\swit_rav.slb 0
lib E:\msim\lib\\swit_reg.slb 0
lib E:\msim\lib\\tex_inst.slb 0
lib E:\msim\lib\\thyristr.slb 0
lib E:\msim\lib\\xtal.slb 0
u 70
U? 2
R? 5
V? 4
@analysis
.TRAN 1 0 0 0
+0 2us
+1 100us
+4 10k
+5 V(out)
@attributes
@index
pageloc 1 0 2180 
@status
n 0 99:05:12:00:18:03;929171883 e 
s 0 99:05:12:00:18:25;929171905 e 
*page 1 0 970 720 iA
@ports
@parts
part  1 OP-07/LT 200 170 h
a 0 s 13 0 50 60 hln 100 PART=OP-07/LT
a 0 a 13 0 50 10 hcn 100 refdes=U1
part  2 r 200 280 h
a 0 u 13 0 15 25 hln 100 value=100k
a 0 x 9 0 15 0 hln 100 REFDES=R1
part  4 VSRC 240 160 u
a 1 u 0 0 0 0 hcn 100 DC=5v
a 1 x 9 0 20 10 hcn 100 refdes=V1
part  6 VSRC 240 260 u
a 1 u 0 0 0 0 hcn 100 DC=5v
a 1 x 9 0 20 10 hcn 100 refdes=V2
part  9 r 310 240 v
a 0 u 13 0 15 25 hln 100 value=1000k
a 0 x 9 0 15 0 hln 100 REFDES=R3
part  10 VSIN 80 220 h
a 1 u 0 0 0 0 hcn 100 voff=0
a 1 u 0 0 0 0 hcn 100 vampl=1
a 1 u 0 0 0 0 hcn 100 freq=20k
a 1 x 9 0 20 10 hcn 100 refdes=V3
a 1 u 0 0 0 0 hcn 100 AC=1v
part  11 r 100 170 h
a 0 u 13 0 15 25 hln 100 value=600
a 0 x 9 0 15 0 hln 100 REFDES=R4
part  0 titleblk 970 720 h
a 1 s 13 0 350 10 hcn 100 PageSize=A
a 1 s 13 0 180 60 hcn 100 PageTitle=
a 1 s 13 0 300 95 hrn 100 PageNO=1
a 1 s 13 0 340 95 hrn 100 PageCount=1
@conn
w 13
s 80 220 80 170 12
s 80 170 100 170 15
w 19
a 0 s 3 0 0 0 hln 100 LABEL=in
s 140 170 200 170 18
a 0 s 3 0 170 168 hln 100 LABEL=in
w 23
a 0 s 3 0 0 0 hln 100 LABEL=0
s 110 280 80 280 22
s 80 280 80 260 25
s 240 120 340 120 54
s 340 120 340 290 57
s 340 290 310 290 59
s 80 290 80 280 61
s 240 260 310 260 48
s 310 260 310 240 51
s 310 290 80 290 69
a 0 s 3 0 195 288 hln 100 LABEL=0
s 310 260 310 290 65
w 29
s 150 280 200 280 28
s 200 280 200 210 32
w 37
a 0 s 3 0 0 0 hln 100 LABEL=out
s 240 280 280 280 36
s 280 280 280 190 39
s 280 190 310 190 42
a 0 s 3 0 295 188 hln 100 LABEL=out
s 310 190 310 200 45
@junction
j 240 220
+ p 6 -
+ p 1 V-
j 240 160
+ p 4 +
+ p 1 V+
j 80 220
+ p 10 +
+ w 13
j 100 170
+ p 11 1
+ w 13
j 200 170
+ p 1 +
+ w 19
j 140 170
+ p 11 2
+ w 19
j 80 260
+ p 10 -
+ w 23
j 200 280
+ p 2 1
+ w 29
j 200 210
+ p 1 -
+ w 29
j 240 280
+ p 2 2
+ w 37
j 280 190
+ p 1 5
+ w 37
j 310 200
+ p 9 2
+ w 37
j 240 260
+ p 6 +
+ w 23
j 310 240
+ p 9 1
+ w 23
j 240 120
+ p 4 -
+ w 23
j 80 290
+ w 23
+ w 23
j 80 280
+ w 23
+ w 23
j 310 260
+ w 23
+ w 23
j 310 290
+ w 23
+ w 23
@attributes
a 0 s 15 0 0 0 hln 100 PAGETITLE=
a 0 s 15 0 0 0 hln 100 PAGENO=1
a 0 s 15 0 0 0 hln 100 PAGESIZE=A
a 0 s 15 0 0 0 hln 100 PAGECOUNT=1
@text
