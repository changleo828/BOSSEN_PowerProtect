/***************************************************************************/
/*
 * 项目名称：电源保护
 * 作    者：Leo Chang
 * 版    本：V1.0
 */
/***************************************************************************/

#include<pic.h>

__CONFIG(0x01CC);  

//数据类型定义
/***************************************************************************/
#define uchar unsigned char
/***************************************************************************/

//用户参数设置
/***************************************************************************/

#define R_I_OVER  (1.3)


#define U_HIGH  (13500)  //mV
#define U_LOW   (9500)   //mV
#define I_OVER  (1400)   //mA

#define U_VALUE_HH ((U_HIGH - 500) / 100 * 18 + 5)
#define U_VALUE_HL ((U_HIGH - 500) / 100 * 18 - 5)

#define U_VALUE_LH ((U_LOW - 500) / 100 * 18 + 5)
#define U_VALUE_LL ((U_LOW - 500) / 100 * 18 - 5)

#define I_VALUE_H  (I_OVER * R_I_OVER)
#define I_VALUE_L  (I_OVER * R_I_OVER - 10)

#if 0
#define U_VALUE_HH (2345)   //过压保护高
#define U_VALUE_HL (2335)   //过压保护低
#define U_VALUE_LH (1607)   //欠压保护高
#define U_VALUE_LL (1597)   //欠压保护低

#define I_VALUE_H (1820)
#define I_VALUE_L (1810)
#endif

#define U_VREF    (3300)
/***************************************************************************/


/***************************************************************************/
#define G1_bit 4 
#define G2_bit 5
#define PowerON   GPIO |= (1<<G1_bit)|(1<<G2_bit)
#define PowerOFF  GPIO &= ~((1<<G1_bit)|(1<<G2_bit))

#define Status_Bit_1 GPIO |= (1<<1)
#define Status_Bit_0 GPIO &= ~(1<<1)
/***************************************************************************/




/***************************************************************************/
//#define DEBUG
#ifdef DEBUG

#define SDA_bit 4 
#define SCL_bit 5
#define SDA_1 GPIO |= (1<<SDA_bit)
#define SDA_0 GPIO &= ~(1<<SDA_bit)
#define SCL_1 GPIO |= (1<<SCL_bit)
#define SCL_0 GPIO &= ~(1<<SCL_bit)
/***************************************************************************/
void Start()							//起始信号子程序
{
	SDA_1;									
	SCL_1;
	SDA_0;
	SCL_0;
}
/***************************************************************************/
void Stop()								//终止信号子程序
{
	SCL_0;
	SDA_0;
	SCL_1;
	SDA_1;
}
void ACK()								//终止信号子程序
{
	SDA_0;
	SCL_0;
	SCL_1;
	SCL_0;
}
void Write8Bit(uchar input)
{										//input为待发送的数据
	uchar temp;

    SCL_0;
	asm("nop");
	for(temp=8;temp!=0;temp--)				//循环移位，逐位发送数据
	{
		if((input&0x80)){				//取数据的最DI位
			SDA_1;
		}else{
			SDA_0;
		}
        SCL_1;
		asm("nop");
	    SCL_0;
		asm("nop");
		input=input<<1;						//right移一位
	}
}
#define ShowValue(x,y)  Write8Bit(x);\
						ACK();\
                        Write8Bit(y);\
                        ACK()



#endif


/***************************************************************************/
//延时函数
void delay()
{
	int i;
    int j;
    for(j = 0;j<1000;j++)
	    for(i=5000;i>0;i--){
		    CLRWDT();
	}
}
void init()
{
    TRISIO=0X05;                   //设置所有I/O口为输出
    OPTION=0X00;                 //设置所有I/O走做普通I/O口
    CMCON = 0xFF;
    INTCON = 0x00;
	PIE1 = 0x00;
    ANSEL = 0x05;
}

#define ADC_Start() (ADCON0 |= (1<<1))
#define IsADC_OK() (ADCON0 & (1<<1))

unsigned int GetADCValue(unsigned char ch)
{
    unsigned int tmp = 0;
    unsigned int value;
    unsigned char i;
	
    ADCON0 = 0x81 | (ch <<2);

    i = 8;
	while(i--)
        {asm("nop");};

    i = 5;
    while(i--){
	    ADC_Start();
		while(!IsADC_OK());
	    value = ADRESH;
	    value = ADRESL;
    }

    tmp = 0;
    i = 16;
    while(i--){
	    ADC_Start();
		while(!IsADC_OK());
	    value = ADRESH;
	    value <<= 8;
	    value += ADRESL;
	    tmp += value;
    }

	return tmp / (unsigned int)16;
}
unsigned int GetUValue(unsigned char ch)
{
	unsigned int value;
    unsigned long U_value_temp;
	value = GetADCValue(ch);
    U_value_temp = value * (unsigned long)U_VREF;
    U_value_temp /= (unsigned long)1024;
    return (unsigned int)U_value_temp;
}
//主函数
/***************************************************************************/
int main(void)
{
    unsigned int U_va;
    unsigned int I_va;
    unsigned char i;

	init();
    PowerOFF;
    i = 100;
    while(i--){
		Status_Bit_0;
		Status_Bit_1;
	}
	while(1)
	{
        Status_Bit_0;
        U_va = GetUValue(0);
		Status_Bit_1;
        I_va = GetUValue(2);
        if((U_va>U_VALUE_LH) && (U_va<U_VALUE_HL) && (I_va<I_VALUE_L)){
		PowerON;
        }
        if((U_va<U_VALUE_LL) || (U_va>U_VALUE_HH) || (I_va>I_VALUE_H)){
        PowerOFF;
        }
		if(I_va>I_VALUE_H){
			PowerOFF;
			delay();
        }
		CLRWDT();
#ifdef DEBUG
        Start();
		ShowValue((unsigned char)(U_va>>8),(unsigned char)U_va);
		ShowValue((unsigned char)(I_va>>8),(unsigned char)I_va);
        Stop();
#endif
	}
    return 0;
}



/***************************************************************************/



