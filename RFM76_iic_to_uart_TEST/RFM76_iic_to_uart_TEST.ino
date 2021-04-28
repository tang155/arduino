//#####################################IIC.C#############################################################
#define __nop() delayMicroseconds(1)
#define IIC_1_SCL 2//// NODEMCU
#define IIC_1_SDA 0// NODEMCU

#define IIC_2_SCL 5  //D1MINI 
#define IIC_2_SDA 4 //D1MINI

#define IIC_SCL_1 pinMode(IIC_1_SCL,OUTPUT);pinMode(IIC_2_SCL,OUTPUT);digitalWrite(IIC_1_SCL,HIGH);digitalWrite(IIC_2_SCL,HIGH)
#define IIC_SCL_0 pinMode(IIC_1_SCL,OUTPUT);pinMode(IIC_2_SCL,OUTPUT);digitalWrite(IIC_1_SCL,LOW);digitalWrite(IIC_2_SCL,LOW)
#define IIC_SDA_1 pinMode(IIC_1_SDA,INPUT);pinMode(IIC_2_SDA,INPUT) //INPUT_PULLUP

#define IIC_SDA_0 do{pinMode(IIC_1_SDA,OUTPUT);pinMode(IIC_2_SDA,OUTPUT);digitalWrite(IIC_1_SDA,LOW);digitalWrite(IIC_2_SDA,LOW);}while(0) //疑难问题，不用do while时放在后面的SDA不会运行？？

#define IIC_SDA_in digitalRead(IIC_1_SDA)&digitalRead(IIC_2_SDA)

//#define IIC_2_SCL 2//// NODEMCU
//#define IIC_2_SDA 0// NODEMCU
//
//#define IIC_SCL 5  //D1MINI 
//#define IIC_SDA 4 //D1MINI
//
//#define IIC_SCL_1 pinMode(IIC_SCL,OUTPUT);digitalWrite(IIC_SCL,HIGH);pinMode(IIC_2_SCL,OUTPUT);digitalWrite(IIC_2_SCL,HIGH)
//#define IIC_SCL_0 pinMode(IIC_SCL,OUTPUT);digitalWrite(IIC_SCL,LOW);pinMode(IIC_2_SCL,OUTPUT);digitalWrite(IIC_2_SCL,LOW)
//#define IIC_SDA_1 pinMode(IIC_SDA,INPUT);pinMode(IIC_2_SDA,INPUT) //INPUT_PULLUP
//#define IIC_SDA_0 pinMode(IIC_SDA,OUTPUT);digitalWrite(IIC_SDA,LOW);pinMode(IIC_2_SDA,OUTPUT);digitalWrite(IIC_2_SDA,LOW)
//#define IIC_SDA_in digitalRead(IIC_2_SDA)

//#define IIC_SCL 2//// NODEMCU
//#define IIC_SDA 0// NODEMCU
//
////#define IIC_SCL 5  //D1MINI 
////#define IIC_SDA 4 //D1MINI
//
//#define IIC_SCL_1 pinMode(IIC_SCL,OUTPUT);digitalWrite(IIC_SCL,HIGH)
//#define IIC_SCL_0 pinMode(IIC_SCL,OUTPUT);digitalWrite(IIC_SCL,LOW)
//#define IIC_SDA_1 pinMode(IIC_SDA,INPUT_PULLUP)
//#define IIC_SDA_0 pinMode(IIC_SDA,OUTPUT);digitalWrite(IIC_SDA,LOW)
//#define IIC_SDA_in digitalRead(IIC_SDA)

unsigned char _RegH,_RegL;
#define RXMAX 40
unsigned char rxBUF[RXMAX+1];
unsigned char datatest = 0;


// sda电平上升耗时 设置i的数值 由平台负载决定i的合理数值  == 高电平恢复所需时间
void iicdelay1(){
  //unsigned char i;
  //mcu 主频过低，不使用for 语句，直接 nop 时间更短 
  //for(i=0;i<1;i++) 
  //{
    __nop();
  //}
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
}

void iicdelay2(){//读写速度决定 设置i的数值 决定iic速度 == 时钟翻转速度
  unsigned char i;
  for(i=0;i<5;i++)
  {
    __nop();
  }

}

void i2c_start(void)
{
    IIC_SDA_1; 
      __nop();
      __nop();  
      __nop();  
    IIC_SCL_1;
  iicdelay2();// 高电平区
      __nop();
      __nop();
    IIC_SDA_0;
      __nop();
      __nop();
    IIC_SCL_0;
}

void i2c_stop(void)
{
 
    IIC_SDA_0;
      __nop();
      __nop();
      __nop();
    IIC_SCL_1;
    __nop();
    __nop();
    __nop();
  IIC_SDA_1;
  //此处根据系统时钟适当延迟，保证高电平有效，
   iicdelay1();//保证真正的高电平被捕捉
    IIC_SCL_0;
    __nop();
    __nop();
    __nop();
   IIC_SCL_1;
}

//写操作
char ACK(void)//slave->master  
{
    IIC_SDA_1;  //释放数据总线
      __nop();
      __nop();  

     iicdelay1();//保证真正的高电平被捕捉
    IIC_SCL_1;
    __nop();
    __nop();
    //ack判定
    if( IIC_SDA_in){
        IIC_SCL_0;
#ifdef  IICDEBUG
        yyy++;//错误统计 debug
#endif
        return 0;
          }
    else{
        IIC_SCL_0;
        return 1;
    
    }
}

void i2c_send_byte(u8 bytes)
{
    u8 ii = 0;
    IIC_SCL_0;

    for(ii = 0; ii < 8; ii++)
    {
        if(bytes & 0x80) 
        {
          IIC_SDA_1;
          iicdelay1();  //保证真正的高电平被捕捉   
        }
        else 
        IIC_SDA_0;
        //iicdelay2(); //ack调试点2
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        IIC_SCL_1;
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        //iicdelay2(); //ack调试点2 // 高电平
        IIC_SCL_0;
        bytes <<= 1;
    }
    IIC_SCL_0; 
  
    __nop();
    //iicdelay2(); //ack调试点2
}

//读操作
char ACK1(void)//master-->slave
{
#ifdef  IICDEBUG
  iicdelay1();
  if(IIC_SDA_in==0){
  xxx++;
  }
#endif
    IIC_SDA_0;
  //iicdelay2();//可忽略
  __nop();
  __nop();
    IIC_SCL_1;
  iicdelay2();//关键位置不可删除// 高电平必须维持一定时长
        __nop();
    IIC_SCL_0;
//  iicdelay2();
  __nop();
  __nop();
    IIC_SDA_1;
  return 1;
}

void NO_ACK(void)//master-->slave
{
#ifdef  IICDEBUG
  iicdelay1();
  if(IIC_SDA_in==0){
  xxx++;
  }
#endif
    IIC_SDA_1;
    iicdelay1();  //此处注意 电平回升
      __nop();
    IIC_SCL_1;
  iicdelay2();//关键位置 不可删除// 高电平必须维持一定时长
      __nop();
      __nop();
    IIC_SCL_0;
    __nop();
    //iicdelay2();
}

u8 i2c_read_byte(void)
{
    u8 temp = 0, ii = 0;
    while(ii < 8)
    {
        IIC_SCL_0;
        //此处保证高电平能够恢复适当延迟
        iicdelay1();
      //  iicdelay2();
        IIC_SCL_1;    
        temp <<= 1;
        if(IIC_SDA_in) temp |= 1;
        ii++;
      iicdelay2();// 高电平
    }
    IIC_SCL_0;
    //iicdelay2();
    return temp;
}


/******华丽的分割线************************/
//RFM76 操作
void i2c_write_data(u8 adds, u16 dat)
{
    i2c_start();
    i2c_send_byte(0x50);
    ACK();
    i2c_send_byte(adds);
     ACK();
    i2c_send_byte(dat / 256);
   ACK();
    i2c_send_byte(dat % 256);
    ACK();
    i2c_stop();
}


void RFM76_write_reg(u8 adds, u8 dat, u8 dat1)
{
    i2c_start();
    i2c_send_byte(0x50);
    ACK();
    i2c_send_byte(adds);
    ACK();
    i2c_send_byte(dat);
     ACK();
    i2c_send_byte(dat1);
     ACK();
    i2c_stop();
}

void RFM76_ReadReg(unsigned char reg)
{
  i2c_start();
  i2c_send_byte(0x50);
  ACK();
  i2c_send_byte(reg|0X80);
  ACK();
  i2c_start();
  i2c_send_byte(0x51);
  ACK();
  _RegH = i2c_read_byte();
  ACK1();
  _RegL = i2c_read_byte();
  NO_ACK();
  i2c_stop();
} 


u16 i2c_read_data(u8 adds)
{
    u16 dat;
    i2c_start();
    i2c_send_byte(0x50);
    ACK();
    i2c_send_byte(adds);
   ACK();
    i2c_start();
    i2c_send_byte(0x51);
     ACK();
   dat = i2c_read_byte();
    ACK1();
    dat *= 256;
    dat += i2c_read_byte();
    NO_ACK();
    i2c_stop();
    return dat;
}
char RFM76_read_fifo(void)
{
    u8 il1 = 1;
    u8 len = 0;
    i2c_start();
    i2c_send_byte(0x50);
    ACK();
    i2c_send_byte(0xb2);
     ACK();
    i2c_start();
    i2c_send_byte(0x51);
    ACK();
    len = i2c_read_byte();
    if(len>RXMAX){
   NO_ACK();
    i2c_stop();
    return 0;
  }
  
    ACK1();
    rxBUF[0] = len;
    for(il1 = 0; il1 < len; il1++)
    {
        rxBUF[il1 + 1] = i2c_read_byte();
if(il1<len-1){   
      ACK1();
}
    }
    NO_ACK();
    i2c_stop();
    return 1;
}
//##################################################################################################
//#######################################RFM76.c###########################################################
#define  delay_MS delay
void RFM76_INIT()
{
reTry:  
  RFM76_write_reg(0x38,0x00,0x00);      //reset?
  delay_MS(10);
  RFM76_write_reg(0x38,0x9B,0xFF);
  delay_MS(10);    
  
  RFM76_ReadReg(0);
  if(_RegH!=0x6F||_RegL!=0XE0)  
  {
    goto reTry; 
  }
  
  RFM76_write_reg(8, 0x6c, 0x90);
  RFM76_write_reg(9, 0x78, 0x30);//set TX power level 8.9dbm
  RFM76_write_reg(28, 0x18, 0x00);
  RFM76_write_reg(32, 0x48, 0x00);//48 06//26;set preamble_len :3 byes.  set syncword_len:32 bits ----Reg36[15:0]------>06->20
  RFM76_write_reg(35, 0x03, 0x00);//自动重传
  
  RFM76_write_reg(36, 0x22, 0x11);  //同步字
  RFM76_write_reg(39, 0x44, 0x33);  //同步字

  RFM76_write_reg(40, 0x44, 0x02);//4402
  RFM76_write_reg(41, 0xB0, 0x00);//crc on scramble off ,1st byte packet length ,auto ack  off  //TX-E800     RX-E000  ACK:BIT 11(接收不能打开,Tx打开后重发自动重传次数)
  RFM76_write_reg(42, 0xFD, 0xB0);
  //LT8960_WriteReg(43, 0x00, 0x0F);//configure scan_rssi

  RFM76_write_reg(44, 0x10, 0x00);//62.5K
  RFM76_write_reg(45, 0x05, 0x52);//62.5K 
  
  RFM76_write_reg(52, 0x80, 0x80);//清空TRX FIFO指针                  
}

static uint8_t RegSetCPL;

void RFM76_Change_0x38(void)
{
  if(RegSetCPL)
  {
    RegSetCPL=0;
    RFM76_write_reg(0x38,0x9C,0x9F);
  }
  else
  {
    RegSetCPL=1;
    RFM76_write_reg(0x38,0x9B,0xFF);
  }
}

//配置MCU与RF进入休眠模式
void Sleep_Mode_Test(void)
{

}

//##################################################################################################
#define Test_Channel  76
#define Tx_Interval_mS  20
#define Set_Led()      digitalWrite(LED_BUILTIN, HIGH)   // turn the LED on (HIGH is the voltage level)
#define Clr_Led()      digitalWrite(LED_BUILTIN, LOW)   // turn the LED on (HIGH is the voltage level)

#define DataLength  20
uint8_t RF_DATA[DataLength];

uint8_t Work,Rate,WorkCnt,GetPacketFlag;
uint8_t RF_init_flag,RxBusy=0,renew;
uint8_t wave_var;
word SysTime;                                      
byte RxTimer;
unsigned char RF_Time;
void TimeQurry(void)
{
  static unsigned long myTime;
  
  if(myTime != millis())   //判断有无标志产生
  {
      myTime = millis();
      SysTime++;
      RxTimer++;
      RF_Time++;
      
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  RFM76_INIT(); 
  //Work定义程序要运行在发射还是接收
  Work=0;     //1:T  0:R
  Serial.begin(115200);
  Serial.setTimeout(20);
  
}
int incomingByte = 0; // for incoming serial data
int incomingByte_cnt = 0; // for incoming serial data

// the loop function runs over and over again forever
void loop() {
  uint8_t i,len,cnt,checksum;

//  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);                       // wait for a second
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);                       // wait for a second
  // reply only when you receive data:
  incomingByte_cnt = Serial.available();
  if (incomingByte_cnt > 0)
  {
    do{
        incomingByte_cnt = Serial.available();//已接收数据
        delay(5);//等待数据接收完毕
      }
    while(incomingByte_cnt != Serial.available()) ;//如果没有新数据进来则退出
    
    // say what you got:
    Serial.print("I received ");
    Serial.println(incomingByte_cnt);
    Serial.print(": ");

    // read the incoming byte:
    for(i=0;i<incomingByte_cnt;i++)//接收串口数据串，并回复
    {
      RF_DATA[i] = Serial.read();
      Serial.write(RF_DATA[i]);      
     }
      Serial.flush();// 清空串口缓存
      for(cnt=0;cnt<1;cnt++)//重复发送次
      {
        RFM76_write_reg(7,0,Test_Channel);  //进入IDLE
        RFM76_write_reg(52, 0x80, 0x80);    //清除指针和FIFO
        i2c_start();
        i2c_send_byte(0x50);  ACK();
        i2c_send_byte(50);  ACK();  //操作FIFO
        i2c_send_byte(incomingByte_cnt);  ACK();  //长度
        
        for(i=0;i<incomingByte_cnt;i++)
        {
          i2c_send_byte(RF_DATA[i]);  ACK();
        }
        i2c_stop();
        RFM76_write_reg(8,0x6C,0x90);
        RFM76_write_reg(7,1,Test_Channel);    //进入TX
        
        for(SysTime=0;SysTime<7;) //7MS
          TimeQurry();
        RxBusy=0;  //准备开启接收
      }

  }

//  while(1)
  {
    TimeQurry();
    
    RxTimer = 0;
    
    
    //1mS
    
    if(RxBusy==0)   //准备开启接收
    {
      RxBusy=1;
      GetPacketFlag=0;
      
      RFM76_write_reg(7,0,Test_Channel);  //IDLE
      RFM76_write_reg(52, 0x80, 0x80);    //清除FIFO
      RFM76_write_reg(8,0x6c,0x90);
      RFM76_Change_0x38();
      RFM76_write_reg(7,0,Test_Channel|0X80);     //启动接收
      goto program_end;
    }
    
    RFM76_ReadReg(3);   //读取状态 PKT 状态指示
    if(((_RegH&0X20)) && RxBusy)  
    {
      RxBusy=0; 
      i2c_start();
      i2c_send_byte(0x50);  ACK();
      i2c_send_byte(50|0x80); ACK();
      i2c_start();
      i2c_send_byte(0x51);  ACK();
      
      len = i2c_read_byte(); ACK1();
      if(len>32)
      len=32;
      
      for(i=0; i<len; i++)
      {
        RF_DATA[i]=i2c_read_byte(); 
        Serial.write(RF_DATA[i]);
        if(i<len-1)      ACK1();
        else          NO_ACK();
      }
      i2c_stop();   

      checksum=0;
      for(i=0;i<(len-1);i++)
      {
        checksum^=RF_DATA[i];
      }         
    
//      if(checksum==RF_DATA[len-1])
      {
        Set_Led();
        for(SysTime=0;SysTime<128;) 
          TimeQurry();
        Clr_Led();
      }
    }
    
    if(RF_Time>100)
    {
      RF_Time=0;
      RxBusy=0;
    }       
    goto program_end;
    
program_end: delay(0);
    
  }
    
}
