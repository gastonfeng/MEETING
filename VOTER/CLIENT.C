/*======================*/
/*	File NAme:CLIENT.C	*/
/*	1999.3.18			*/
/*======================*/
#include<reg51.H>

/*----------------------*/
/*      EPROM操作代码   */
/*----------------------*/
#define EWEN    0x0100
#define EWDS    0
#define ERAL    0x0300
#define ERASE   0x0200

#define REGREQU	'R'
sbit BAODAO=P1^4;
sbit QIQUAN=P1^5;
sbit FANDUI=P1^6;
sbit ZANCHENG=P1^7;
sbit senable=P3^7;
sbit cs=P3^5;
sbit Do=P3^2;
sbit di=P3^3;
sbit clk=P3^4;

bit received;

unsigned char buf[4],local;

void tdelay(unsigned char time)
{
	unsigned char i;
	for(;time>0;time--){
		for(i=0;i<115;i++);
	}
}

DELAY()
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

/*--------------------------*/
/*  EPROM写入程序           */
/*  address=-1,写整个芯片   */
/*  address<256,写单个数据  */
/*--------------------------*/
void write_eprom(int address,int Data)
{
    union a{
        int i;
        char addr[2];
    }b;
    char k;
    set_eprom(EWEN);
    b.i=address;
    cs=clk=di=0;
    Do=1;
    cs=1;
    while(!Do);
    di=1;
    DELAY();
    if(address==-1){
            b.addr[1]=0x10;
        }
        else{
            b.addr[1]|=0x40;
        }
    for(k=0;k<8;k++){
        di=b.addr[1]>>7;
        DELAY();
        b.addr[1]<<=1;
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
int eprom(char address)
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

unsigned char getch()
{
    unsigned char j;
    j=P1&0xf;
    if(j==0xf)return 0xff;
    tdelay(20);
    if(j==(P1&0xf)){
        return j;
    }
    return 0xff;
}

void serial() interrupt 4 using 2
{
    static unsigned char ibuf[4],ipoint;
    static bit rec;
/*    unsigned char i;*/
	if(RI){
		RI=0;
		if(SBUF==0xaa){
			ipoint=0;
			rec=0;
		}
		if(!rec){
			ibuf[ipoint]=SBUF;
			ipoint++;
			if(ipoint>=4){
				if(ibuf[3]==0xcc){
					buf[1]=ibuf[1];
					buf[2]=ibuf[2];
					received=1;
					ipoint=0;
					rec=1;
				}
				else{
					ipoint=0;
				}
			}
	    }
	}
    else{
    	senable=0;
    	TI=0;
    }
}

send(unsigned char cc)
{
	while(senable);
	senable=1;
	SBUF=cc;
}

getnumber(){	
	while(getch()==0xff){
		QIQUAN=1;
		tdelay(50);
		QIQUAN=0;
		tdelay(50);
	}
	while(1){
		send(0xaa);
		send(REGREQU);
		send(REGREQU);
		send(0xbb);
		QIQUAN=~QIQUAN;
		tdelay(200);
		if(received){
			received=0;
			if(buf[1]==buf[2]){
				local=buf[1];
				send(0xaa);
				send(local);
				send(0xdd);
				send(0xbb);
				write_eprom(10,local);
				write_eprom(50,local);
				local=buf[1];
				if(eprom(10)!=local || eprom(50)!=local){
					QIQUAN=1;
					ZANCHENG=1;
					FANDUI=1;
					BAODAO=1;
					while(1);
				}
				QIQUAN=0;
				break;
			}
		}
	}
}
                   	
void main()
{
	unsigned char status,i,work;
	BAODAO=QIQUAN=FANDUI=ZANCHENG=0;
	work=0;
	received=0;
	senable=0;
	status='N';
	local=eprom(10);
	SCON=0x50;
	PCON=0;
	TMOD=0x20;
	TH1=0xe8;
	IE=0x90;
	TR1=1;
	if(local!=eprom(50)){
		getnumber();
	}
	else{
		BAODAO=1;
		tdelay(200);
		BAODAO=0;
		QIQUAN=1;
		tdelay(200);
		QIQUAN=0;
		FANDUI=1;
		tdelay(200);
		FANDUI=0;
		ZANCHENG=1;
		tdelay(200);
	}
	BAODAO=QIQUAN=FANDUI=ZANCHENG=0;
	while(1){
		if(received){
			received=0;
			if(buf[1]==local){
				switch(buf[2]){
					case 0xc0:{
						send(0xaa);
						send(local);
						send(0xc1);
						send(0xbb);
						BAODAO=1;
						tdelay(200);
						BAODAO=0;
						QIQUAN=1;
						tdelay(200);
						QIQUAN=0;
						FANDUI=1;
						tdelay(200);
						FANDUI=0;
						ZANCHENG=1;
						tdelay(200);
						ZANCHENG=0;
						break;
					}
					case 0xd0:{
						send(0xaa);
						send(local);
						send(status);
						send(0xbb);
						status='N';
						work=0;
						BAODAO=QIQUAN=FANDUI=ZANCHENG=0;
		            	break;
					}
					default:;
				}
			}
			if(buf[1]==0xee){
				switch(buf[2]){
					case 'C':{
						status='N';
						BAODAO=QIQUAN=FANDUI=ZANCHENG=0;
						work=1;
						break;
					}
					case 'B':{
						work=2;
						BAODAO=QIQUAN=FANDUI=ZANCHENG=0;
						status='N';
						break;
					}
					case 'D':{
						work=0;
						BAODAO=QIQUAN=FANDUI=ZANCHENG=0;
						status='N';
						break;
					}
					case 0xe0:{
						getnumber();
						break;
					}
					default:;
				}
			}
		}
		switch(getch()){
			case 0x7:{
				if(work==1){
					status='0';
					BAODAO=1;
					QIQUAN=0;
					FANDUI=0;
					ZANCHENG=0;
				}
				break;
			}			
			case 0xb:{
				if(work==2){
					status='1';
					BAODAO=0;
					QIQUAN=1;
					FANDUI=0;
					ZANCHENG=0;
				}
				break;
			}
			case 0xd:{
				if(work==2){
					status='2';
					BAODAO=0;
					QIQUAN=0;
					FANDUI=1;
					ZANCHENG=0;
				}
				break;
			}
			case 0xe:{
				if(work==2){
					status='3';
					BAODAO=0;
					QIQUAN=0;
					FANDUI=0;
					ZANCHENG=1;
				}
				break;
			}
			case 0:{
				for(i=0;i<100;i++){
					if(getch()!=0)break;
					tdelay(10);
				}
				if(i==100){
					write_eprom(10,0xff);
					getnumber();
				}
				break;
			}
			default:;
		}
	}
}
