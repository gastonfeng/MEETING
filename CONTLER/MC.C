/*==========================*/
/*	File Name:MC.C			*/
/*	Start:1999.03.08		*/
/*==========================*/

/*==========================*/
/*	Include Head File		*/
/*==========================*/
#include <AT89x51.h>
/*#include<\3h\cpx51.h>
sbit WR=P3^5;
sbit RD=P3^6;
sbit INT1=P3^7;
*/

/*----------------------*/
/*	ERROR CODE	*/
/*----------------------*/
#define ERROR_NONUMBER		128     /*  no useable number   */
#define	ERROR_NUMBER		129

/*----------------------*/
/*      EPROM操作代码   */
/*----------------------*/
#define EWEN    0x0100
#define EWDS    0
#define ERAL    0x0300
#define ERASE   0x0200

sbit senable=P3^4;          /* RS485 send enable */
sbit cs=P3^3;
sbit Do=P3^5;
sbit di=P0^0;
sbit clk=P1^3;
/*
sbit cs=P1^0;
sbit clk=P1^1;
sbit di=P1^2;
sbit Do=P1^3;
*/
/*==========================*/
/*	8250 Reg Define			*/
/*==========================*/
#define Baudlow 	0
#define Baudhigh	1
#define Intid		2
#define Linecontrol	3
#define Modemcontrol	4
#define Linestatus	5
#define Modemstatus	6

/*==========================*/
/*	Hardware IO Define		*/
/*==========================*/
#define comport 	P2          /* use P2.0-P2.2 to select 8250 register    */
#define dataport	P0          /* data bus between 8250 */
/*#define WR			P36
#define RD			P37*/

/*------------------------------*/
/*  keyboard value table        */
/*------------------------------*/
#define NO  0xf
#define k1 0x70 
#define k2 0xb0
#define k3 0xd0
#define k4 0xe0

sbit L1=P1^4;       /*LED 1-4*/
sbit L2=P1^5;
sbit L3=P1^6;
sbit L4=P1^7;
sbit s1=P1^0;      /* Video&Audio Channel select */
sbit s2=P1^1;
sbit s3=P1^2;

sbit cs8250=P2^3;
bit received=0,fault=0;
unsigned char buf[4],PCbuf[4];

unsigned char addr;
unsigned char num;
unsigned char status;
unsigned char baodao;
unsigned char zancheng;
unsigned char fandui;
unsigned char qiquan;
unsigned char weian;
unsigned char ch;

void Write8250(unsigned char reg,unsigned char com);
unsigned char Read8250(unsigned char reg);
unsigned char Receivedpc(void);
void send(unsigned char com);
void tdelay(unsigned char);
void sendframe(void);

DELAY()             /* 93C46 cycle clock    */
{
    clk=1;
    clk=0;
}
 
/*----------------------*/
/*  EPROM设置及擦除程序 */
/*  command=00H,EWDS  */
/*  command=01H,EWEN  */
/*----------------------*/
void set_eprom(int command)
{
    char k;
    union a{
        int com;
        char opr[2];
    }set;
    set.com=command;
    cs=clk=di=0;
    Do=1;
    cs=1;
    while(!Do);
    di=1;
    DELAY();
    switch(set.opr[0]){
        case 0:{
            set.opr[1]=0;
            break;
        }
        case 1:{
            set.opr[1]=0x30;
            break;
        }
        case 2:{
            set.opr[1]|=0xc0;
            break;
        }
        case 3:{
            set.opr[1]=0x20;
            break;
        }
    }
    for(k=0;k<8;k++){
        di=set.opr[1]>>7;
        DELAY();
        set.opr[1]<<=1;
    }
    cs=clk=di=0;
}

void write_eprom(unsigned char address,int Data)
{
    char k;
    set_eprom(EWEN);
    address|=0x40;
    cs=clk=di=0;
    Do=1;
    cs=1;
    while(!Do);
    di=1;
    DELAY();
    for(k=0;k<8;k++){
        di=address>>7;
        DELAY();
        address<<=1;
    }
    for(k=0;k<16;k++){
        di=Data>>15;
        DELAY();
        Data<<=1;
    }
    cs=clk=di=0;
    set_eprom(EWDS);
}

/*------------------*/
/*  EPROM读出程序   */
/*------------------*/
int eprom(unsigned char address)
{
    char k,i;
    int result;
    address|=0x80;
    for(i=0;i<10;i++){
        cs=clk=di=0;
        Do=1;
        cs=1;
        while(!Do);
        cs=1;
        di=1;
        DELAY();
        for(k=0;k<8;k++){
            di=address>>7;
            DELAY();
            address<<=1;
        }
        if(Do==0)goto OK;
    }
    cs=clk=di=0;
    return -1;
OK:
    for(k=0;k<16;k++){
        result<<=1;
        DELAY();
        result=result|Do;
    }
    cs=clk=di=0;
    return result;
}

unsigned char kbhit()       /* key press ?  */
{
    if((P2&0xf0)==0xf0)return 0;else return 1;
}

unsigned char getch()       /* read key     */
{
    unsigned char j;
    j=P2&0xf0;
    if(j==0xf0)return NO;
    tdelay(20);
    if(j==(P2&0xf0)){
        return j;
    }
    return NO;
}
	
void serial() interrupt 4 using 2       /* serial receive form rs485 bus */
{
	static unsigned char ibuf[4],ipoint;
	static bit rec;
/*	unsigned char i;*/
	if(RI){
		RI=0;
/*		Write8250(0,SBUF);*/
		if(SBUF==0xaa){             /* frame format: 0xaa,data1,data2,0xbb */
			ipoint=0;
			rec=0;
		}
		if(!rec){
			ibuf[ipoint]=SBUF;
			ipoint++;
			if(ipoint>=4){
				if(ibuf[3]==0xbb){
					buf[1]=ibuf[1];
					buf[2]=ibuf[2];
					received=1;
					ipoint=0;
					rec=1;
				}
				else ipoint=0;
			}
    	}
    }
    else{
    	senable=0;
    	TI=0;
    }
}

sendtoclient(unsigned char cc)
{
	while(senable);
	senable=1;
	SBUF=cc;
/*	tdelay(10);*/
}

unsigned char readclient()
{
	unsigned char i;
	for(i=0;i<20;i++){
		if(received){
			received=0;
			return 1;
		}
		tdelay(10);
	}
	return 0;
}

void main(void)
{
	unsigned char i,j;
	bit flag;
	INT1=0;
	L1=L2=L3=L4=0;
	s1=s2=s3=0;
	L1=1;
	senable=0;
	SCON=0x50;
	PCON=0;
	TMOD=0x20;
	TH1=0xe8;
	IE=0x90;
	TR1=1;
	cs8250=1;
	WR=RD=1;
	tdelay(250);
	Write8250(Linecontrol,0x80);
	Write8250(Baudlow,0xc);
	Write8250(Baudhigh,0);
	Write8250(Linecontrol,3);
	Write8250(Modemcontrol,0xb);
	Write8250(Linecontrol,Read8250(Linecontrol)&0x7f);
	Write8250(Modemcontrol,2);
	addr=eprom(63);
	while(1){
		if(received){
			received=0;
			switch(buf[2]){
				case 'R':{
					for(i=0;i<64;i++){
						if(eprom(i)!=0x1111){
							j=0;
							flag=1;
							while(flag){
								sendtoclient(0xaa);
								sendtoclient(i);
								sendtoclient(i);
								sendtoclient(0xcc);
								/*if(readclient()){
									send(buf[1]+48);
									if(buf[2]==0xdd)send('d');else send('e');
									if((buf[1]==i)&&(buf[2]==0xdd)){*/
										write_eprom(i,0x1111);
										num+=1;
										/*send('R');
										send('=');
										send(i/10+48);
										send((i%10)+48);*/
										sendframe();
										flag=0;;
									/*}
								}*/
								j++;
								if(j==10){
									status=ERROR_NUMBER;
									sendframe();
									flag=0;;
								}
							}
							break;
						}
					}
					if(i==64){
						status=ERROR_NONUMBER;
						sendframe();
					}
					break;
				}
			}
		}
		if(Receivedpc()){
			if(PCbuf[1]==addr){
				switch(PCbuf[2]){
					case 0xe0:{		/*	进入编号状态	*/
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient(0xe0);
						sendtoclient(0xcc);
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient(0xe0);
						sendtoclient(0xcc);
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient(0xe0);
						sendtoclient(0xcc);
						status=5;
						sendframe();
						break;
					}
					case 0xd0:{		/*	统计表决器数量	*/
						num=0;
						for(i=0;i<64;i++){
							if(eprom(i)==0x1111){
								sendtoclient(0xaa);
								sendtoclient(i);
								sendtoclient(0xc0);
								sendtoclient(0xcc);
								if(readclient()){
									if(buf[1]==i){
										if(buf[2]==0xc1){
											num+=1;
										}
										else write_eprom(i,0);
									}
								}
							}
						}
						status=6;
						sendframe();
						status=0;
						break;
					}
					case 0xe3:{		/*	进入报到状态	*/
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient('C');
						sendtoclient(0xcc);
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient('C');
						sendtoclient(0xcc);
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient('C');
						sendtoclient(0xcc);
						status=1;
						sendframe();
						break;
					}
					case 0xe4:{		/*	统计报到人数	*/
						baodao=0;
						for(i=0;i<64;i++){
							if(eprom(i)==0x1111){
								sendtoclient(0xaa);
								sendtoclient(i);
								sendtoclient(0xd0);
								sendtoclient(0xcc);
								if(readclient()){
									if(buf[1]==i){
										if(buf[2]=='0')baodao+=1;
									}
								}
							}
						}
						status=2;
						sendframe();
						status=0;
						break;
					}
					case 0xe5:{		/*	进入表决状态	*/
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient('B');
						sendtoclient(0xcc);
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient('B');
						sendtoclient(0xcc);
						sendtoclient(0xaa);
						sendtoclient(0xee);
						sendtoclient('B');
						sendtoclient(0xcc);
						status=3;
						sendframe();
						break;
					}
					case 0xe6:{		/*	统计表决结果	*/
						zancheng=fandui=qiquan=weian=0;
						for(i=0;i<64;i++){
							if(eprom(i)==0x1111){
								sendtoclient(0xaa);
								sendtoclient(i);
								sendtoclient(0xd0);
								sendtoclient(0xcc);
								if(readclient()){
									if(buf[1]==i){
										switch(buf[2]){
											case 'N':weian+=1;break;
											case '1':qiquan+=1;break;
											case '2':fandui+=1;break;
											case '3':zancheng+=1;break;
											default:fault=1;
										}
									}
								}
							}
						}
						status=4;
						sendframe();
						status=0;
						break;
					}
					case 0xf1:{
						L1=1;
						s1=s2=s3=0;
						L2=L3=L4=0;
						ch=1;
						sendframe();
						break;
					}
					case 0xf2:{
						L2=1;
						L1=L3=L4=0;
						s1=1;
						s2=s3=0;
						ch=2;
						sendframe();
						break;
					}
					case 0xf3:{
						L3=1;
						L1=L2=L4=0;
						s2=1;
						s1=s3=0;
						ch=3;
						sendframe();
						break;
					}
					case 0xf4:{
						L4=1;
						L1=L2=L3=0;
						s2=1;
						s1=1;
						s3=0;
						ch=4;
						sendframe();
						break;
					}
					default:;
				}
			}
			if(PCbuf[1]==0xee){
				switch(PCbuf[2]){
					case 0xef:{		/*	进入控制器编号状态	*/
						status=7;
						for(i=0;i<64;i++)write_eprom(i,0xffff);
						num=0;
						L1=L2=L3=L4=1;
   	 					while(!kbhit());
   	   				    L1=L2=L3=L4=0;
   	   		            tdelay(200);
   	   	             	L1=L2=L3=L4=1;
   	 					flag=1;
   	   		            while(flag){
   	 				   	    sendframe();
   	   	    			    for(i=0;i<255;i++){
   	   	                        if(Receivedpc()){
			                        if(PCbuf[2]==0xf0){
					                    addr=PCbuf[1];
   					                    status=8;
					                    sendframe();
					                    write_eprom(63,(int)addr);
					                    L1=L2=L3=L4=0;
					                    status=0;
   	                     				flag=0;
   	                                    break;
				                    }
			                    }
                       			tdelay(1);
   	   	                    }
   	   	                }        
   	   	                break;
					}
					default:;
				}
			}
				
		}
		if(kbhit()){
            switch(getch()){
				case k1:{
					L1=1;
					s1=s2=s3=0;
					L2=L3=L4=0;
					ch=1;
					sendframe();
					break;
				}
				case k2:{
					L2=1;
					L1=L3=L4=0;
					s1=1;
					s2=s3=0;
					ch=2;
					sendframe();
					break;
				}
				case k3:{
					L3=1;
					L1=L2=L4=0;
					s2=1;
					s1=s3=0;
					ch=3;
					sendframe();
					break;
				}
				case k4:{
					L4=1;
					L1=L2=L3=0;
					s2=1;
					s1=1;
					s3=0;
					ch=4;
					sendframe();
					break;
				}
				default:;
			}
		}
	}
}	

void Write8250(unsigned char reg,unsigned char com)
{
	comport=0xf8|reg;
	dataport=com;
	cs8250=0;
	WR=0;
	WR=1;
	cs8250=1;
}

unsigned char Read8250(unsigned char reg)
{
	unsigned char com;
	dataport=0xff;
	comport=0xf8|reg;
	cs8250=0;
	RD=0;
	com=dataport;
	RD=1;
	cs8250=1;
	return com;
}

unsigned char Receivedpc()
{
	static unsigned char pbuf[4],ppoint;
	static bit rec;
	unsigned char i;
	if((Read8250(Linestatus)&1)==0x1){
		i=Read8250(0);
		if(i==0xab){
			ppoint=0;
			rec=0;
		}
		if(!rec){
			pbuf[ppoint]=i;
			ppoint++;
			if(ppoint>=4){
				if(pbuf[3]==0xcc){
					PCbuf[1]=pbuf[1];
					PCbuf[2]=pbuf[2];
					ppoint=0;
					rec=1;
					return 1;
				}
			}
		}
	}
	return 0;
}

void tdelay(unsigned char i)
{
	unsigned char j;
	for(;i>0;i--){
		for(j=0;j<100;j++);
	}
}

void send(unsigned char com)
{
	while(!(Read8250(Linestatus)&0x20));
	Write8250(0,com);
}

void sendframe(void)
{
	unsigned char checksum;
	send(0xab);
	checksum=0xab;
	send(addr);
	checksum+=addr;
	send(num);
	checksum+=num;
	send(status);
	checksum+=status;
	send(baodao);
	checksum+=baodao;
	send(zancheng);
	checksum+=zancheng;
	send(fandui);
	checksum+=fandui;
	send(qiquan);
	checksum+=qiquan;
	send(weian);
	checksum+=weian;
	send(ch);
	checksum+=ch;
	send(checksum);	
}

