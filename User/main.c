#include "sys.h"
#include "delay.h"
#include "gpio.h"
#include "OLED_I2C.h"
#include "timer.h"
#include "dht11.h"
#include "esp8266.h"
#include "usart2.h"
#include "usart3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define STM32_RX2_BUF       Usart2RecBuf 
#define STM32_Rx2Counter    Rx2Counter
#define STM32_RX2BUFF_SIZE  USART2_RXBUFF_SIZE

#define STM32_RX3_BUF       Usart3RecBuf 
#define STM32_Rx3Counter    Rx3Counter
#define STM32_RX3BUFF_SIZE  USART3_RXBUFF_SIZE

#define  RATIO  800		//系数，建议选择800-1000
u16 PM25_Value = 0;     		//PM = ((pmBuf[1]<<8)+pmBuf[2])/1024*8*ratio
u16 PM25_Value_max = 200; //PM2.5报警值
u8 pmBuf[7];

char display[16];
unsigned char setn=0;//记录设置键按下的次数
unsigned char temperature=0;
unsigned char humidity=0;
unsigned int  CH2O_mgvalue = 0;
unsigned char setTempValue=35;        //温度设置值
unsigned char setHumiValue=75;        //湿度设置值
unsigned int setCH2OValue=80;        //甲醛设置值
unsigned char i=0;

bool shuaxin  = 1;
bool shanshuo = 0;
bool sendFlag = 1;

void displayInitFrame(void)  //显示主页面
{
		for(i=0;i<2;i++)OLED_ShowCN(i*16+0,0,i+0,0);//显示中文：温度
		for(i=0;i<2;i++)OLED_ShowCN(i*16+0,2,i+2,0);//显示中文：湿度
		for(i=0;i<2;i++)OLED_ShowCN(i*16+0,4,i+4,0);//显示中文：甲醛
		OLED_ShowStrPm(0, 6, 2,0);
	  OLED_ShowStr(16, 6, "2.5:", 2,0);
		OLED_ShowChar(32,0,':',2,0);
		OLED_ShowChar(32,2,':',2,0);
		OLED_ShowChar(32,4,':',2,0);
	  OLED_ShowCentigrade(76, 0);
	  OLED_ShowChar(76,2,'%',2,0);
}

void Get_PM2_5(void)
{
    char i = 0;
    char j = 0;

	  STM32_Rx3Counter = 0;
    if(B_RX_OK == 1)
    {
        for(i = 0; i<8; i++)
        {
            if((STM32_RX3_BUF[i] == 0xAA)&&(STM32_RX3_BUF[i+6]==0xFF))
            {
                goto find2;
            }
        }
        goto end2;
find2:
        for(j = 0; j<7; j++)
        {
            pmBuf[j] = STM32_RX3_BUF[i+j];
        }

        PM25_Value = (unsigned int)((pmBuf[1]*256)+pmBuf[2])*5/2048.0*RATIO;
        B_RX_OK = 0;
    }
end2:
    return;
}

void Get_CH2O(void)  //获取甲醛
{
    char i = 0;

		if(STM32_RX2_BUF[i+5] == STM32_RX2_BUF[i]+STM32_RX2_BUF[i+1]+STM32_RX2_BUF[i+2]+STM32_RX2_BUF[i+3]+STM32_RX2_BUF[i+4])//校验
		{
				CH2O_mgvalue = STM32_RX2_BUF[1]*256 + STM32_RX2_BUF[2];//计算甲醛
		}
		STM32_Rx2Counter = 0;
}

void displaySetValue(void)  //显示设置的值
{
		if(setn == 1)
		{
				OLED_ShowChar(52,4,setTempValue/10+'0',2,0);
				OLED_ShowChar(60,4,setTempValue%10+'0',2,0);
				OLED_ShowCentigrade(70, 4);
		}
		if(setn == 2)
		{
				OLED_ShowChar(52,4,setHumiValue/10+'0',2,0);
				OLED_ShowChar(60,4,setHumiValue%10+'0',2,0);
				OLED_ShowChar(68,4,'%',2,0);
			  OLED_ShowChar(76,4,' ',2,0);
		}
		if(setn == 3)
		{
				sprintf(display,"%4.2fmg/m3",(float)setCH2OValue/100);
			  OLED_ShowStr(28, 4, (u8 *)display, 2,0);
		}
		if(setn == 4)
		{
				sprintf(display,"%03dug/m3 ",PM25_Value_max);
				OLED_ShowStr(28, 4, (u8 *)display, 2,0);
		}
}

void keyscan(void)   //按键扫描
{
	 if(KEY1 == 0) //设置
	 {
			delay_ms(20);
		  if(KEY1 == 0)
			{
				  while(KEY1 == 0);
				  BEEP=0;
				  setn ++;
				  if(setn == 1)
					{
						  OLED_CLS(0);              //清屏
						
						  OLED_Fill_Row(0xFF,0);   //填充白色
						  OLED_Fill_Row(0xFF,1);   //填充白色
							for(i=0;i<4;i++)OLED_ShowCN(i*16+32,0,i+6,1);//显示中文：设置温度
						  /* 显示直线框 */
							OLED_DrawHorizLine(63,0,127);
							OLED_DrawVerticaLine(0,2,8);
							OLED_DrawVerticaLine(127,2,8);
						
						  delay_ms(150);
					}
					if(setn == 2)
					{
							for(i=0;i<4;i++)OLED_ShowCN(i*16+32,0,i+10,1);//显示中文：设置湿度  
					}
					if(setn == 3)
					{
							for(i=0;i<4;i++)OLED_ShowCN(i*16+32,0,i+14,1);//显示中文：设置甲醛  
					}
					if(setn == 4)
					{
							for(i=0;i<2;i++)OLED_ShowCN(i*16+24,0,i+14,1);//显示中文：设置
						  OLED_ShowStrPm(56, 0, 2,1);
						  OLED_ShowStr(72, 0, "2.5", 2,1);
					}
					if(setn >= 5)
					{
							setn = 0;
						  OLED_CLS(0);              //清屏
						  displayInitFrame();
					}
					displaySetValue();
			}
	 }
	 if(KEY2 == 0) //加
	 {
			delay_ms(80);
		  if(KEY2 == 0)
			{
					if(setTempValue<99 && setn==1)setTempValue++;
					if(setHumiValue<99 && setn==2)setHumiValue++;
				  if(setCH2OValue<999 && setn==3)setCH2OValue++;
					if(PM25_Value_max<999 && setn==4)PM25_Value_max++;	
				  displaySetValue();
			}
	 }
	 if(KEY3 == 0) //减
	 {
			delay_ms(80);
		  if(KEY3 == 0)
			{
					if(setTempValue>0 && setn==1)setTempValue--;
					if(setHumiValue>0 && setn==2)setHumiValue--;
				  if(setCH2OValue>0 && setn==3)setCH2OValue--;
					if(PM25_Value_max>0 && setn==4)PM25_Value_max--;	
				  displaySetValue();
			}
	 }
}

void UsartSendReceiveData(void)
{
		unsigned char *dataPtr = NULL;
		char *str1=0,i;
	  int  setValue=0;
	  char setvalue[5]={0};
	  char SEND_BUF[120];
	
	  dataPtr = ESP8266_GetIPD(0);   //接收数据
		if(dataPtr != NULL)
		{
			  if(strstr((char *)dataPtr,"temp:")!=NULL)
				{
					  BEEP = 1;
						delay_ms(80);
					  BEEP = 0;
					
						str1 = strstr((char *)dataPtr,"temp:");
					  
					  while(*str1 < '0' || *str1 > '9')    //判断是不是0到9有效数字
						{
								str1 = str1 + 1;
								delay_ms(10);
						}
						i = 0;
						while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
						{
								setvalue[i] = *str1;
								i ++; str1 ++;
								if(*str1 == ',')break;            //换行符，直接退出while循环
								delay_ms(10);
						}
						setvalue[i] = '\0';            //加上结尾符
						setValue = atoi(setvalue);
						if(setValue>=0 && setValue<=99)
						{
							  setTempValue=setValue;
								displaySetValue();
						}
				} 
			
				if(strstr((char *)dataPtr,"humi:")!=NULL)
				{
						str1 = strstr((char *)dataPtr,"humi:");
					  
					  while(*str1 < '0' || *str1 > '9')    //判断是不是0到9有效数字
						{
								str1 = str1 + 1;
								delay_ms(10);
						}
						i = 0;
						while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
						{
								setvalue[i] = *str1;
								i ++; str1 ++;
								if(*str1 == ',')break;            //换行符，直接退出while循环
								delay_ms(10);
						}
						setvalue[i] = '\0';            //加上结尾符
						setValue = atoi(setvalue);
						if(setValue>=0 && setValue<=99)
						{
							  setHumiValue=setValue;
								displaySetValue();
						}
				}
				
				if(strstr((char *)dataPtr,"chIIo:")!=NULL)  //接收到设置甲醛上限的指令
				{
						str1 = strstr((char *)dataPtr,"chIIo:");
					  
					  while(*str1 < '0' || *str1 > '9')        //判断是不是0到9有效数字
						{
								str1 = str1 + 1;
								delay_ms(10);
						}
						i = 0;
						while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
						{
								setvalue[i] = *str1;
							  str1 ++;
							  i ++;
								if(*str1 == '.')break;            //小数点，直接退出while循环
								delay_ms(10);
						}
						if(*str1 == '.')
						{
								str1 = str1 + 1;
						}
						while(*str1 >= '0' && *str1 <= '9')
						{
								setvalue[i] = *str1;
								i ++; str1 ++;
						}
						setvalue[i] = '\0';            //加上结尾符
						setValue = atoi(setvalue);
						if(i==2)setValue=setValue*10;
						if(i==1)setValue=setValue*100;
						if(setValue>=0 && setValue<=999)
						{
								setCH2OValue = setValue;    //设置的甲醛上限
							  displaySetValue();
						}
				}
				
				if(strstr((char *)dataPtr,"pm:")!=NULL)
				{
						str1 = strstr((char *)dataPtr,"pm:");
					  
					  while(*str1 < '0' || *str1 > '9')    //判断是不是0到9有效数字
						{
								str1 = str1 + 1;
								delay_ms(10);
						}
						i = 0;
						while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
						{
								setvalue[i] = *str1;
								i ++; str1 ++;
								if(*str1 == '\r')break;            //换行符，直接退出while循环
								delay_ms(10);
						}
						setvalue[i] = '\0';            //加上结尾符
						setValue = atoi(setvalue);
						if(setValue>=0 && setValue<=999)
						{
							  PM25_Value_max=setValue;
								displaySetValue();
						}
				}
				ESP8266_Clear();									//清空缓存
		}
		if(sendFlag==1)    //1秒钟上传一次数据
		{
			  sendFlag = 0;		
			   
				memset(SEND_BUF,0,sizeof(SEND_BUF));   			//清空缓冲区
				sprintf(SEND_BUF,"$temp:%d#,$humi:%d#,$ch2o:%4.2f#,$pm2.5:%d#",temperature,humidity,(float)CH2O_mgvalue/100,PM25_Value);
			
			  ESP8266_SendData((u8 *)SEND_BUF, strlen(SEND_BUF));
			  ESP8266_Clear();
		}
}

int main(void)
{
	  u16 timeCount=200;
	  bool delay_600ms=1;
	
		delay_init();	           //延时函数初始化	 
    NVIC_Configuration();	   //中断优先级配置
	  delay_ms(300); 
	  I2C_Configuration();     //IIC初始化
	  OLED_Init();             //OLED液晶初始化
	  OLED_CLS(1);              //清屏
	  KEY_GPIO_Init();        //按键引脚初始化    
	  OLED_ShowStr(0, 2, "   loading...   ", 2,1);//显示加载中
    ESP8266_Init();       //ESP8266初始化	
	  delay_ms(1000);
	  while(DHT11_Init())
		{
				OLED_ShowStr(0, 2, "  DHT11 Init!  ", 2,1);//显示DHT11初始化！
			  delay_ms(500);
		}
		OLED_CLS(0);              //清屏
		//USART2_Init(9600);
		USART2_Init(115200);
		USART3_Init(2400);
		displayInitFrame();
	  TIM3_Init(99,719);   //定时器初始化，定时1ms
		//Tout = ((arr+1)*(psc+1))/Tclk ; 
		//Tclk:定时器输入频率(单位MHZ)
		//Tout:定时器溢出时间(单位us)
		while(1)
		{ 
			   keyscan();  //按键扫描
			   
			   if(setn == 0)     //不在设置状态下
				 {
					   timeCount ++;
					   if(timeCount >= 180)
						 {
								timeCount = 0;
							  DHT11_Read_Data(&temperature,&humidity);   //读取温湿度
							
							 
								{	printf("\\{\"services\\\": [{\"service_id\":\"STM32\",\"properties\":{\"dty11t\":%d,\"dty11h\":%d,\"dty11i\":%d,\"PM2.5\":%f}}]}");
								}}
							  Get_CH2O();                                //读取甲醛
						 }
					 
						 if(shuaxin == 1)        //大概300ms刷新一下
						 { 
								 shuaxin = 0;
								 
							   delay_600ms = !delay_600ms;
							 
							   if(temperature>=setTempValue && shanshuo) 
							   {
									   OLED_ShowChar(56,0,' ',2,0);
										 OLED_ShowChar(64,0,' ',2,0);
								 } 
								 else
								 {
										 OLED_ShowChar(56,0,temperature/10+'0',2,0);    //显示温度
										 OLED_ShowChar(64,0,temperature%10+'0',2,0);
								 }
							   
							   if(humidity>=setHumiValue && shanshuo)
							   {
									   OLED_ShowChar(56,2,' ',2,0);
										 OLED_ShowChar(64,2,' ',2,0);
								 } 
								 else
								 {
										 OLED_ShowChar(56,2,humidity/10+'0',2,0);      //显示湿度
										 OLED_ShowChar(64,2,humidity%10+'0',2,0);
								 }
							   if(CH2O_mgvalue>=setCH2OValue && shanshuo)
							   {
									   OLED_ShowStr(48, 4, "    mg/m3", 2,0);
								 } 
								 else
								 {
										sprintf(display,"%4.2fmg/m3",(float)CH2O_mgvalue/100); 
									  OLED_ShowStr(48, 4, (u8 *)display, 2,0);       //显示甲醛
								 }
							   if(delay_600ms)
								 {
										 Get_PM2_5();            //读取PM2.5
										 if(PM25_Value>999)PM25_Value=999;
								 }
								 
								 if(PM25_Value>=PM25_Value_max && shanshuo)
							   {
									   OLED_ShowStr(56, 6, "   ug/m3", 2,0);
								 } 
								 else
								 {
										 sprintf(display,"%03dug/m3",PM25_Value);
										 OLED_ShowStr(56, 6, (u8 *)display, 2,0);    //显示PM2.5
								 }
								 
								 if(temperature>=setTempValue || humidity>=setHumiValue || CH2O_mgvalue>=setCH2OValue || PM25_Value>=PM25_Value_max)
								 {BEEP = 1; RELAY=1;}   //超出上限，蜂鸣器提醒。开启风扇
								 else
								 {BEEP = 0; RELAY=0;}   //关闭风扇和蜂鸣器
						 }
				 }
				 UsartSendReceiveData();   //串口发送接收数据
			   delay_ms(10);
		}


void TIM3_IRQHandler(void)//定时器3中断服务程序，用于记录时间
{ 
	  static u16 timeCount1 = 0;
	  static u16 timeCount2 = 0;
	
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{ 
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除中断标志位  

			  timeCount1 ++;
			  timeCount2 ++;
			  if(timeCount1 >= 300)  //300ms
				{
						timeCount1 = 0;
					  shanshuo = !shanshuo;
					  shuaxin = 1;
				}
			  if(timeCount2 >= 800)  //800ms
				{
						timeCount2 = 0;
					  sendFlag = 1;
				}
	  }
}

//#include "stm32f10x_hal.h"
//#define USART1_TX_PIN GPIO_PIN_1
//#define USART1_RX_PIN GPIO_PIN_2
//while(1)
//	if( dty_read())
//		{printf("{"services": [{"service_id":"STM32","properties":{"dty11t":%d,"dty11h":%d,"dty11i":%d,"PM2.5":%f}}]}")
//	}
