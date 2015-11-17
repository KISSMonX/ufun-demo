测试准备: 
1. 两路 PWM 接到两路 ADC 上面. 即 con3的1/2脚接 j6 的 1/3脚.
2. SD 卡: 
        a) 上电前插入 SD 卡, 上电复位会输出 SD 卡详细信息. 见附注.
        b) 上电后插入 SD 卡, 无打印信息.
        c) 无 SD 卡时, 上电后 LED 是黄绿交替亮.
        d) 有 SD 卡时, 上电后 LED 是蓝粉交替亮. 

3. 串口打印信息每隔 1s 输出一次. 比如: 
        
        Current Time: 00:11:16
        The AD_SIG1 value is: 8
        The AD_SIG2 value is: 14
        X=183, Y=-41, Z=-1019

4. 如果发现计算机设备管理器不能识别串口设备. 把 j-link USB 口拔掉.
   按住复位键, 插入 USB 口. 松开. 即可识别虚拟串口设备. 


附注: 
    上电前 SD 卡打印信息: 

    LIS3DH Init is succeed! 
    External Reset occurred....
    No need to configure RTC....

    ManufacturerID:0x3 
    OEM_appliID:0x5344 
    ProductName:SS08G 
    ProductVersion: 80 
    ProductSerialNumber:47627c35 
    ManufactureDate:0f-4 
    CID_CRC:36 
    CSD Structure Version:2
    Asynchronous access time:0xe
    NSAC:0x0Max data transfer rate:0x32
    Card command classes:0x5b5
    Max. read block length:512
    Partial block read allowed?...0
    Write block misalignment allowed?...0
    Read block misalignment allowed?...0
    DSR implemented?...0
    C_Size:15165
    Max. read current@VDD min:0x0
    Max. read current@VDD max:0x0
    Max. write current@VDD min:0x0
    Max. write current@VDD max:0x0
    Device size multiplier:0
    Partial block write allowed?...0

    发现SD卡!