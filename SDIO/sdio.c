
#include"data.c"

#include"sdCard.h"
#include <string.h>


/***************************************************
  SD card / T-Flash card / MMC card
SDIO  : 4bits
PC8     : Data0
PC9     : Data1
PC10    : Data2
PC11    : CD/Data3
PC12    : SDIO_CK
PD2     : SDIO_CMD
 ****************************************************/

//用户配置区
//SDIO时钟计算公式:SDIO_CK时钟=SDIOCLK/[clkdiv+2];其中,SDIOCLK一般为72Mhz
//使用DMA模式的时候,传输速率可以到24Mhz,不过如果你的卡不是高速卡,可能也会出错
//出错就请降低时钟,使用查询模式的话,推荐SDIO_TRANSFER_CLK_DIV设置为3或者更大
#define SDIO_INIT_CLK_DIV        0xB2 		//SDIO初始化频率，最大400Kh
#define SDIO_TRANSFER_CLK_DIV    0x02		//SDIO传输频率，最大24Mhz(4bit)。该值太小可能会导致读写文件出错

//SDIO工作模式定义,通过SD_SetDeviceMode函数设置.
#define SD_POLLING_MODE    	0 //查询模式,该模式下,建议SDIO_TRANSFER_CLK_DIV设置为0X04.
#define SD_DMA_MODE    		  1	//DMA模式,该模式下,SDIO_TRANSFER_CLK_DIV可以设置为1,单如果写入出错的话,可以考虑降低增大SDIO_TRANSFER_CLK_DIV的值
#define SD_INTERRUPT_MODE   2 //

dword CIDReg[4];    // cid register data
dword CSDReg[4];    // csd register data
word RCA;           // 卡相对地址
byte CardType;      // SD卡类型
SD_CardInfo SDInfo; // SD卡所有信息

byte SDIO_DATA_BUFFER[512];
byte DeviceMode;
SD_ERROR TransferError=SD_OK;
byte StopCondition=0;			//单块读,不需要发送停止传输指令
byte TransferEnd=0;
byte TotalNumberOfBytes = 0;


/**********************************************
  SDIO_CK=SDIOCLK / [CLKDIV + 2]
 ***********************************************/
inline void SDIO_CK_Set(byte div)
{
	SDIO_CLKCR &= 0xffffff00;
	SDIO_CLKCR |= div;
}

/**********************************************
  SDIO power on/ SDIO power off
state:  SDIO_POWER_PWRCTRL_ON
SDIO_POWER_PWRCTRL_OFF
 ***********************************************/

inline void SDIO_PWRCTRL(byte State)
{
	SDIO_POWER=State;
}

/**********************************************
  SDIO 寄存器初始化，为SD卡上电初始化做准备
  SDIO初始化频率，最大400Kh
 ***********************************************/
void Init_SDIO()
{
	RCC_AHBENR |= RCC_AHBENR_SDIOEN;
	SDIO_DATA_OUT;    // alternate function output push-pull
	SDIO_CMD_OUT;
	SDIO_CK_OUT;

	SDIO_CLKCR=0;
	SDIO_CLKCR &= ~SDIO_CLKCR_HWFC_EN;
	SDIO_CLKCR &= ~SDIO_CLKCR_NEGEDGE;
	SDIO_CLKCR |= SDIO_CLKCR_WIDBUS_ONEBITS;
	SDIO_CLKCR &= ~SDIO_CLKCR_BYPASS;
	SDIO_CLKCR &= ~SDIO_CLKCR_PWRSAV;

	SDIO_CK_Set(SDIO_INIT_CLK_DIV);
	SDIO_PWRCTRL(SDIO_POWER_PWRCTRL_ON);

	SDIO_CLKCR |= SDIO_CLKCR_CLKEN;

	NVIC_Interrupt(IRQ_SDIO,Enable);
}
/**********************************************
  SDIO发送命令函数,无等待中断请求
cmdindex:命令索引,低六位有效
arg:参数
waitResp:期待的相应.00/10,无响应;01,短相应;11,长响应
 ***********************************************/
void SDIO_SendCMD(byte cmdindex,dword arg,byte waitResp)
{
	SDIO_ARG=arg;
	SDIO_CMD &= 0xfffff800;
	SDIO_CMD |= cmdindex;
	SDIO_CMD |= waitResp<<6;
	SDIO_CMD |= SDIO_CMD_CPSMEN; //命令通道状态机使能
}
/**********************************************
  SDIO发送数据配置函数
datatimeout  :数据超时时间
datalen      :数据长度 字节长度
blocksize    :数据块大小 2^size(bit)
datadir      :数据方向 0:to card, 1:to sdio
 ***********************************************/
void SDIO_DataConfig(dword datatimeout,dword datalen,byte blocksize,byte datadir)
{
	SDIO_DTIMER=datatimeout;
	SDIO_DLEN=datalen;
	SDIO_DCTRL &= 0xff08;
	SDIO_DCTRL |= blocksize<<4;
	//SDIO_DCTRL &= ~SDIO_DCTRL_DTMODE; //块数据传输
	SDIO_DCTRL |= (datadir&1)<<1;
	SDIO_DCTRL |= SDIO_DCTRL_DTEN;  //数据传输使能
}

/**********************************************
//配置SDIO DMA  (DMA2->Channel 4)
//mbuf:存储器地址
//bufsize:传输数据量,字节数
//dir:方向;1,存储器->SDIO(写数据);0,SDIO->存储器(读数据);
 **********************************************/
void SD_DMA2_Config(dword*mbuf,dword bufsize,byte dir)
{
	DMA2_IFCR &= 0xffff0fff;  //清除DMA2通道4的各种标记
	DMA2_CCR4 = 0;

	DMA2_CCR4 |= dir<<4;//从存储器读
	DMA2_CCR4 |= BIT7;//存储器增量模式
	DMA2_CCR4 |= BIT9;//外设数据宽度32bit
	DMA2_CCR4 |= BIT11;//存储器数据宽度32bit
	DMA2_CCR4 |= BIT13;//通道优先级为高

	DMA2_CNDTR4=bufsize>>2;//DMA2,传输数据量 byte->dword
	DMA2_CPAR4=(dword)&SDIO_FIFO;//DMA2 外设地址
	DMA2_CMAR4=(dword)mbuf; 		//DMA2,存储器地址
	DMA2_CCR4 |= BIT0;// enable DMA Channel

}


/**********************************************
  命令CMD0的响应
  等待命令发送完成，否则响应超时
 ***********************************************/
SD_ERROR CMD0Response()
{
	dword timeout=SDIO_CMD0TIMEOUT;
	while(timeout--)
	{
		if(SDIO_STA&SDIO_STA_CMDSENT)break;
	}
	if(0 == timeout)return SD_CMD_RSP_TIMEOUT;
	SDIO_ICR=SDIO_STATIC_FLAGS;  //清除标记
	return SD_OK;
}

/**********************************************
  R1响应
 ***********************************************/
SD_ERROR CMDResponse1(byte CMD)
{
	dword value;
	while(1)    // 等待R1响应
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRC校验错误
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   //cmd响应超时
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	if(SDIO_RESPCMD != CMD)return SD_ILLEGAL_CMD;
	SDIO_ICR=SDIO_STATIC_FLAGS;  //清除标记
	if(SDIO_RESP1&SD_STAT_ERRORBITS)return SD_Error;
	else{
		return SD_OK;
	}
}


/**********************************************
  R2响应(CID/CSD)
 ***********************************************/
SD_ERROR CMDResponse2()
{
	dword value;
	while(1)    // 等待R1响应
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRC校验错误
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   // cmd响应超时
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;  //清除标记

	return SD_OK;
}



/**********************************************
  R3响应(OCR)
Commandindex :default 111,111
CRC7         :default 111,111
 ***********************************************/
SD_ERROR CMDResponse3()
{
	dword value;
	while(1)    // 等待R1响应
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   // cmd响应超时
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}

	SDIO_ICR=SDIO_STATIC_FLAGS;  //清除标记

	return SD_OK;
}

/**********************************************
  R6响应(RCA)
 ***********************************************/
SD_ERROR CMDResponse6(byte CMD,word *prca)
{
	dword value;
	dword resp;
	while(1)    // 等待R1响应
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   // cmd响应超时
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRC校验错误
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(SDIO_RESPCMD != CMD)return SD_ILLEGAL_CMD;//判断是否响应cmd命令
	SDIO_ICR=SDIO_STATIC_FLAGS;  //清除标记

	resp=SDIO_RESP1;
	if(SD_ALLZERO == (resp&(SD_R6_GENERAL_UNKNOWN_ERROR|SD_R6_ILLEGAL_CMD|SD_R6_COM_CRC_FAILED)))
	{
		*prca=(word)(resp>>16);			//右移16位得到,rca
		return SD_OK;
	}
	if(resp&SD_R6_GENERAL_UNKNOWN_ERROR)return SD_GENERAL_UNKNOWN_ERROR;
	if(resp&SD_R6_ILLEGAL_CMD)return SD_ILLEGAL_CMD;
	if(resp&SD_R6_COM_CRC_FAILED)return SD_COM_CRC_FAILED;
	return SD_OK;
}

/**********************************************
  R7响应
 ***********************************************/
SD_ERROR CMDResponse7()   // 与R2一样?
{
	dword value;
	dword timeout=SDIO_CMD0TIMEOUT;
	while(timeout--){
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRC校验错误
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(0==timeout || SDIO_STA & SDIO_STA_CTIMEOUT){// cmd响应超时
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;  //清除标记
	return SD_OK;
}

/**********************************************
  SD card power on
  SD卡上电操作
  查询sd总线上的卡设备，并查询其电压配置
 ***********************************************/

SD_ERROR SD_PowerON()
{
	SD_ERROR status=SD_OK;
	dword SDType=SD_STD_CAPACITY;
	dword response=0,count=0;
	word BusyFlag=0;

	//---------------CMD0----------------//
	// SD_CMD_GO_IDLE_STATE
	// no response

	SDIO_SendCMD(SD_CMD_GO_IDLE_STATE,0,0);
	status=CMD0Response();
	if(status != SD_OK)return status;

	//---------------CMD8----------------//
	// Send CMD8 to verify SD card interface operating condition
	// SDIO_SEND_IF_COND
	// arg 11:8 _ supply voltage(VHS)0x01
	// resp: R7

	SDIO_SendCMD(SDIO_SEND_IF_COND,SD_CHECK_PATTERN,1); //2.7~3.6V
	status = CMDResponse7(); // R7 response
	if(status != SD_OK)return status;
	else{
		SDType=SD_HIGH_CAPACITY;//高容量卡
		CardType=SDIO_STD_CAPACITY_SD_CARD_V2_0;// 2.0卡
	}
	// CMD55  用以区分SD2.0和MMC卡，有响应的为SD
	SDIO_SendCMD(SD_CMD_APP_CMD,0,1);
	status=CMDResponse1(SD_CMD_APP_CMD);
	if(status==SD_OK){//SD2.0/SD 1.1
		//SD卡,发送ACMD41 SD_APP_OP_COND,参数为:0x80100000
		while((!BusyFlag)&&(count<SD_MAX_VOLT_TRIAL))
		{
			SDIO_SendCMD(SD_CMD_APP_CMD,0,1);			 //发送CMD55,短响应
			status=CMDResponse1(SD_CMD_APP_CMD);  //等待R1响应
			if(status!=SD_OK)return status;   	   //响应错误
			// -----Send ACMD41-----//
			//发送ACMD41,短响应
			SDIO_SendCMD(SD_CMD_SD_APP_OP_COND,SD_VOLTAGE_WINDOW_SD|SDType,1);
			status=CMDResponse3(); 					     //等待R3响应
			if(status!=SD_OK)return status;        //响应错误
			response=SDIO_RESP1;                   //得到响应
			BusyFlag=(((response>>31)==1)?1:0);//判断SD卡上电是否完成
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL){
			return SD_INVALID_VOLTRANGE;
		}
		if(response&=SD_HIGH_CAPACITY){
			CardType=SDIO_HIGH_CAPACITY_SD_CARD;
		}
	}
	else{    //MMC卡,发送CMD1 SDIO_SEND_OP_COND,参数为:0x80FF8000
		while((!BusyFlag)&&(count<SD_MAX_VOLT_TRIAL))
		{
			//发送CMD1,短响应
			SDIO_SendCMD(SD_CMD_SEND_OP_COND,SD_VOLTAGE_WINDOW_MMC,1);
			status=CMDResponse3(); 					//等待R3响应
			if(status!=SD_OK)return status;   //响应错误
			response=SDIO_RESP1;              //得到响应
			BusyFlag=(((response>>31)==1)?1:0);
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL){
			return SD_INVALID_VOLTRANGE;
		}
		CardType=SDIO_MULTIMEDIA_CARD;
	}
	return(status);

}

/**********************************************
  SD卡初始化操作
  卡进入就绪状态
  先发送CMD2,读取CID
  再发送CMD3,获取RCA
  最后发送CMD9,读取CSD
 ***********************************************/

SD_ERROR SD_InitializeCard()
{
	SD_ERROR status=SD_OK;
	word rca=1;
	if(0 == SDIO_POWER & SDIO_POWER_PWRCTRL_ON)return SD_REQUEST_NOT_APPLICABLE;
	if(SDIO_SECURE_DIGITAL_IO_CARD != CardType){//非SECURE_DIGITAL_IO_CARD
		//Send CMD2 ALL_SEND_CID
		//resp: R2
		SDIO_SendCMD(SD_CMD_ALL_SEND_CID,0,3);// 长响应,read CID
		status=CMDResponse2();
		if(status != SD_OK)return status;
		CIDReg[0]=SDIO_RESP1;
		CIDReg[1]=SDIO_RESP2;
		CIDReg[2]=SDIO_RESP3;
		CIDReg[3]=SDIO_RESP4;
	}
	if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType)||
			(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType)||
			(SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType)||
			(SDIO_HIGH_CAPACITY_SD_CARD == CardType))//判断卡类型
	{
		// Send CMD3 SD_CMD_SET_REL_ADDR with argument 0
		// resp :R6
		SDIO_SendCMD(SD_CMD_SET_REL_ADDR,0,1);
		status=CMDResponse6(SD_CMD_SET_REL_ADDR,&rca);//返回RCA
		if(status != SD_OK)return status;
	}
	else if (SDIO_MULTIMEDIA_CARD == CardType){  // MMC
		SDIO_SendCMD(SD_CMD_SET_REL_ADDR,(dword)(rca<<16),1);//发送CMD3,短响应
		status=CMDResponse2();           //等待R2响应
		if(status!=SD_OK)return status;   //响应错误
	}
	//非SECURE_DIGITAL_IO_CARD
	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType){
		RCA = rca;
		//发送CMD9+卡RCA,取得CSD,长响应
		SDIO_SendCMD(SD_CMD_SEND_CSD,(dword)(rca<<16),3);
		status=CMDResponse2(); 					//等待R2响应
		if(status!=SD_OK)return status;   	//响应错误
		CSDReg[0]=SDIO_RESP1;
		CSDReg[1]=SDIO_RESP2;
		CSDReg[2]=SDIO_RESP3;
		CSDReg[3]=SDIO_RESP4;
	}
	return SD_OK;//卡初始化成功

}

/**********************************************
  根据之前读取的SD Card 寄存器,得出SD Card 的参数

 **********************************************/


void SD_GetCardInfo(SD_CardInfo *cardinfo)
{
	byte tmp=0;
	cardinfo->CardType=(byte)CardType;        //卡类型
	cardinfo->RCA=(word)RCA;             //卡RCA值
	tmp=(byte)((CSDReg[0]&0xFF000000)>>24);
	cardinfo->SD_csd.CSDStruct=(tmp&0xC0)>>6;   //CSD结构
	cardinfo->SD_csd.SysSpecVersion=(tmp&0x3C)>>2;  //2.0协议还没定义这部分(为保留),应该是后续协议定义的
	cardinfo->SD_csd.Reserved1=tmp&0x03;      //2个保留位
	tmp=(byte)((CSDReg[0]&0x00FF0000)>>16);      //第1个字节
	cardinfo->SD_csd.TAAC=tmp;              //数据读时间1
	tmp=(byte)((CSDReg[0]&0x0000FF00)>>8);       //第2个字节
	cardinfo->SD_csd.NSAC=tmp;              //数据读时间2
	tmp=(byte)(CSDReg[0]&0x000000FF);        //第3个字节
	cardinfo->SD_csd.MaxBusClkFrec=tmp;         //传输速度
	tmp=(byte)((CSDReg[1]&0xFF000000)>>24);      //第4个字节
	cardinfo->SD_csd.CardComdClasses=tmp<<4;      //卡指令类高四位
	tmp=(byte)((CSDReg[1]&0x00FF0000)>>16);      //第5个字节
	cardinfo->SD_csd.CardComdClasses|=(tmp&0xF0)>>4;//卡指令类低四位
	cardinfo->SD_csd.RdBlockLen=tmp&0x0F;       //最大读取数据长度
	tmp=(byte)((CSDReg[1]&0x0000FF00)>>8);     //第6个字节
	cardinfo->SD_csd.PartBlockRead=(tmp&0x80)>>7; //允许分块读
	cardinfo->SD_csd.WrBlockMisalign=(tmp&0x40)>>6; //写块错位
	cardinfo->SD_csd.RdBlockMisalign=(tmp&0x20)>>5; //读块错位
	cardinfo->SD_csd.DSRImpl=(tmp&0x10)>>4;
	cardinfo->SD_csd.Reserved2=0;           //保留
	if((CardType==SDIO_STD_CAPACITY_SD_CARD_V1_1)||
			(CardType==SDIO_STD_CAPACITY_SD_CARD_V2_0)||
			(SDIO_MULTIMEDIA_CARD==CardType))       //标准1.1/2.0卡/MMC卡
	{
		cardinfo->SD_csd.DeviceSize=(tmp&0x03)<<10; //C_SIZE(12位)
		tmp=(byte)(CSDReg[1]&0x000000FF);      //第7个字节
		cardinfo->SD_csd.DeviceSize|=(tmp)<<2;
		tmp=(byte)((CSDReg[2]&0xFF000000)>>24);    //第8个字节
		cardinfo->SD_csd.DeviceSize|=(tmp&0xC0)>>6;
		cardinfo->SD_csd.MaxRdCurrentVDDMin=(tmp&0x38)>>3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax=(tmp&0x07);
		tmp=(byte)((CSDReg[2]&0x00FF0000)>>16);    //第9个字节
		cardinfo->SD_csd.MaxWrCurrentVDDMin=(tmp&0xE0)>>5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax=(tmp&0x1C)>>2;
		cardinfo->SD_csd.DeviceSizeMul=(tmp&0x03)<<1;//C_SIZE_MULT
		tmp=(byte)((CSDReg[2]&0x0000FF00)>>8);     //第10个字节
		cardinfo->SD_csd.DeviceSizeMul|=(tmp&0x80)>>7;
		cardinfo->CardCapacity=(cardinfo->SD_csd.DeviceSize+1);//计算卡容量
		cardinfo->CardCapacity*=(1<<(cardinfo->SD_csd.DeviceSizeMul+2));
		cardinfo->CardBlockSize=1<<(cardinfo->SD_csd.RdBlockLen);//块大小
		cardinfo->CardCapacity*=cardinfo->CardBlockSize;
	}else if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)  //高容量卡
	{
		tmp=(byte)(CSDReg[1]&0x000000FF);    //第7个字节
		cardinfo->SD_csd.DeviceSize=(tmp&0x3F)<<16;//C->SIZE
		tmp=(byte)((CSDReg[2]&0xFF000000)>>24);  //第8个字节
		cardinfo->SD_csd.DeviceSize|=(tmp<<8);
		tmp=(byte)((CSDReg[2]&0x00FF0000)>>16);  //第9个字节
		cardinfo->SD_csd.DeviceSize|=(tmp);
		tmp=(byte)((CSDReg[2]&0x0000FF00)>>8);   //第10个字节
		cardinfo->CardCapacity=(long long)(cardinfo->SD_csd.DeviceSize+1)*512*1024;//计算卡容量
		cardinfo->CardBlockSize=512;      //块大小固定为512字节
	}
	cardinfo->SD_csd.EraseGrSize=(tmp&0x40)>>6;
	cardinfo->SD_csd.EraseGrMul=(tmp&0x3F)<<1;
	tmp=(byte)(CSDReg[2]&0x000000FF);      //第11个字节
	cardinfo->SD_csd.EraseGrMul|=(tmp&0x80)>>7;
	cardinfo->SD_csd.WrProtectGrSize=(tmp&0x7F);
	tmp=(byte)((CSDReg[3]&0xFF000000)>>24);    //第12个字节
	cardinfo->SD_csd.WrProtectGrEnable=(tmp&0x80)>>7;
	cardinfo->SD_csd.ManDeflECC=(tmp&0x60)>>5;
	cardinfo->SD_csd.WrSpeedFact=(tmp&0x1C)>>2;
	cardinfo->SD_csd.MaxWrBlockLen=(tmp&0x03)<<2;
	tmp=(byte)((CSDReg[3]&0x00FF0000)>>16);    //第13个字节
	cardinfo->SD_csd.MaxWrBlockLen|=(tmp&0xC0)>>6;
	cardinfo->SD_csd.WriteBlockPaPartial=(tmp&0x20)>>5;
	cardinfo->SD_csd.Reserved3=0;
	cardinfo->SD_csd.ContentProtectAppli=(tmp&0x01);
	tmp=(byte)((CSDReg[3]&0x0000FF00)>>8);   //第14个字节
	cardinfo->SD_csd.FileFormatGrouop=(tmp&0x80)>>7;
	cardinfo->SD_csd.CopyFlag=(tmp&0x40)>>6;
	cardinfo->SD_csd.PermWrProtect=(tmp&0x20)>>5;
	cardinfo->SD_csd.TempWrProtect=(tmp&0x10)>>4;
	cardinfo->SD_csd.FileFormat=(tmp&0x0C)>>2;
	cardinfo->SD_csd.ECC=(tmp&0x03);
	tmp=(byte)(CSDReg[3]&0x000000FF);      //第15个字节
	cardinfo->SD_csd.CSD_CRC=(tmp&0xFE)>>1;
	cardinfo->SD_csd.Reserved4=1;

	tmp=(byte)((CIDReg[0]&0xFF000000)>>24);    //第0个字节
	cardinfo->SD_cid.MID=tmp;
	tmp=(byte)((CIDReg[0]&0x00FF0000)>>16);    //第1个字节
	cardinfo->SD_cid.OID=tmp<<8;
	tmp=(byte)((CIDReg[0]&0x000000FF00)>>8);   //第2个字节
	cardinfo->SD_cid.OID|=tmp;
	tmp=(byte)(CIDReg[0]&0x000000FF);      //第3个字节
	cardinfo->SD_cid.PNM1=tmp<<24;
	tmp=(byte)((CIDReg[1]&0xFF000000)>>24);    //第4个字节
	cardinfo->SD_cid.PNM1|=tmp<<16;
	tmp=(byte)((CIDReg[1]&0x00FF0000)>>16);      //第5个字节
	cardinfo->SD_cid.PNM1|=tmp<<8;
	tmp=(byte)((CIDReg[1]&0x0000FF00)>>8);   //第6个字节
	cardinfo->SD_cid.PNM1|=tmp;
	tmp=(byte)(CIDReg[1]&0x000000FF);        //第7个字节
	cardinfo->SD_cid.PNM2=tmp;
	tmp=(byte)((CIDReg[2]&0xFF000000)>>24);    //第8个字节
	cardinfo->SD_cid.PRV=tmp;
	tmp=(byte)((CIDReg[2]&0x00FF0000)>>16);    //第9个字节
	cardinfo->SD_cid.PSN=tmp<<24;
	tmp=(byte)((CIDReg[2]&0x0000FF00)>>8);     //第10个字节
	cardinfo->SD_cid.PSN|=tmp<<16;
	tmp=(byte)(CIDReg[2]&0x000000FF);        //第11个字节
	cardinfo->SD_cid.PSN|=tmp<<8;
	tmp=(byte)((CIDReg[3]&0xFF000000)>>24);    //第12个字节
	cardinfo->SD_cid.PSN|=tmp;
	tmp=(byte)((CIDReg[3]&0x00FF0000)>>16);    //第13个字节
	cardinfo->SD_cid.Reserved1|=(tmp&0xF0)>>4;
	cardinfo->SD_cid.MDT=(tmp&0x0F)<<8;
	tmp=(byte)((CIDReg[3]&0x0000FF00)>>8);   //第14个字节
	cardinfo->SD_cid.MDT|=tmp;
	tmp=(byte)(CIDReg[3]&0x000000FF);      //第15个字节
	cardinfo->SD_cid.CIDCRC=(tmp&0xFE)>>1;
	cardinfo->SD_cid.Reserved2=1;

}

/**********************************************
  读取 SD Card SCR register
rca:卡相对地址
pscr:数据缓存区(存储SCR内容)
 **********************************************/
SD_ERROR ReadSCR(word rca,dword *pscr)
{
	dword index = 0;
	SD_ERROR status = SD_OK;
	dword tempscr[2]={0,0};

	//Send CMD16: Set Block Size To 8 Bytes
	SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,8,1);//短响应,设置Block Size为8字节
	status=CMDResponse1(SD_CMD_SET_BLOCKLEN);
	if(status!=SD_OK)return status;

	//Send CMD55 APP_CMD with argument as card's RCA
	SDIO_SendCMD(SD_CMD_APP_CMD,(dword)rca<<16,1);
	status=CMDResponse1(SD_CMD_APP_CMD);
	if(status!=SD_OK)return status;

	//Send ACMD51 to read SCR register(64bit)
	// before the cmd, config return data format
	SDIO_DataConfig(SD_DATATIMEOUT,8,3,1); //8个字节长度,block为8字节,SD卡到SDIO.
	SDIO_SendCMD(SD_CMD_SD_APP_SEND_SCR,0,1);  //发送ACMD51,短响应,参数为0
	status=CMDResponse1(SD_CMD_SD_APP_SEND_SCR);
	if(status!=SD_OK)return status;

	while(!(SDIO_STA&(SDIO_STA_DBCKEND|SDIO_STA_STBITERR|SDIO_STA_RXOVERR
					|SDIO_STA_DTIMEOUT|SDIO_STA_DCRCFAIL)))
	{
		if(SDIO_STA&SDIO_STA_RXDAVL){//接收FIFO数据可用
			*(tempscr+index)=SDIO_FIFO;	//读取FIFO内容
			index++;
			if(index>=2)break;
		}
	}
	if(SDIO_STA&SDIO_STA_DTIMEOUT){		//接收数据超时
		SDIO_ICR |= SDIO_ICR_DTIMEOUTC;
		return SD_DATA_TIMEOUT;
	}
	else if(SDIO_STA&SDIO_STA_DCRCFAIL){
		SDIO_ICR |= SDIO_ICR_DCRCFAILC;
		return SD_DATA_CRC_FAIL;
	}
	else if(SDIO_STA&SDIO_STA_RXOVERR){
		SDIO_ICR |= SDIO_ICR_RXOVERRC;
		return SD_RX_OVERRUN;
	}
	else if(SDIO_STA&SDIO_STA_STBITERR){
		SDIO_ICR |= SDIO_ICR_STBITERRC;
		return SD_START_BIT_ERR;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;	 		//清除所有标记
	//把数据顺序按8位为单位倒过来.
	*(pscr+1)=((tempscr[0]&SD_0TO7BITS)<<24)|((tempscr[0]&SD_8TO15BITS)<<8)|((tempscr[0]&SD_16TO23BITS)>>8)|((tempscr[0]&SD_24TO31BITS)>>24);
	*(pscr)=((tempscr[1]&SD_0TO7BITS)<<24)|((tempscr[1]&SD_8TO15BITS)<<8)|((tempscr[1]&SD_16TO23BITS)>>8)|((tempscr[1]&SD_24TO31BITS)>>24);
	return status;
}


/**********************************************
  SD Card 使能宽总线模式
enable:0,不使能;1,使能;
 **********************************************/
SD_ERROR SDCardEnWideBus(byte enable)
{
	SD_ERROR status = SD_OK;
	dword scr[2]={0,0};
	byte arg=0X00;      // 1bit
	if(enable)arg=0X02; // 4bit

	//-------CMD13:send card status with RCA-------//
	SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);
	status=CMDResponse1(SD_CMD_SEND_STATUS);
	if(status != SD_OK)return status;
	if(SDIO_RESP1&SD_STAT_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//SD卡处于LOCKED状态

	// get SCR register
	status=ReadSCR(RCA,scr);						//得到SCR寄存器数据
	if(status!=SD_OK)return status;

	if((scr[1]&SD_WIDE_BUS_SUPPORT)!=SD_ALLZERO){		//支持宽总线
		//发送CMD55+RCA,短响应
		SDIO_SendCMD(SD_CMD_APP_CMD,(dword)RCA<<16,1);
		status=CMDResponse1(SD_CMD_APP_CMD);
		if(status!=SD_OK)return status;
		//发送ACMD6,短响应,参数:10,4位;00,1位
		SDIO_SendCMD(SD_CMD_APP_SD_SET_BUSWIDTH,arg,1);
		status=CMDResponse1(SD_CMD_APP_SD_SET_BUSWIDTH);
		return status;
	}else return SD_REQUEST_NOT_APPLICABLE;//不支持宽总线设置
}


/**********************************************
  设置SD卡和SDIO总线宽度(MMC卡不支持4bit模式)
wmode  : WideMode: Specifies the SD card wide bus mode.
- 2: 8-bit data transfer (Only for MMC)
- 1: 4-bit data transfer
- 0: 1-bit data transfer
 **********************************************/
SD_ERROR SD_EnableWideBusOperation(dword wmode)
{
	SD_ERROR status=SD_OK;
	if(SDIO_MULTIMEDIA_CARD==CardType)return SD_UNSUPPORTED_FEATURE;//MMC卡不支持
	else if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
			(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
			(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
	{
		if(wmode>=2)return SD_UNSUPPORTED_FEATURE;//不支持8位模式
		else{
			status=SDCardEnWideBus(wmode);//设定SD卡的总线宽度
			if(SD_OK==status){
				SDIO_CLKCR&=~(3<<11);		//清除之前的位宽设置
				SDIO_CLKCR|=(word)wmode<<11;//1位/4位总线宽度
				SDIO_CLKCR|=0<<14;			//不开启硬件流控制
			}
		}
	}
	return status;
}
/**********************************************
  设置SD卡工作模式
Mode:
返回值:错误状态
 **********************************************/
SD_ERROR SD_SetDeviceMode(dword Mode)
{
	SD_ERROR status = SD_OK;
	if((Mode==SD_DMA_MODE)||(Mode==SD_POLLING_MODE)||Mode==SD_INTERRUPT_MODE){
		DeviceMode=Mode;
	}
	else status=SD_INVALID_PARAMETER;
	return status;
}

/**********************************************
  选中/不选中卡 CMD7
  输入：卡相对地址 RCA
  描述: 当RCA为0时，所有卡进入stand-by
  RCA正确时进入Transfer mode
 **********************************************/
SD_ERROR SD_SelectCard(dword rca)
{
	SD_ERROR status=SD_OK;
	// send CMD7
	SDIO_SendCMD(SD_CMD_SEL_DESEL_CARD,rca<<16,1);
	status=CMDResponse1(SD_CMD_SEL_DESEL_CARD);
	return status;
}


/*********************************************
  初始化 SD card
 **********************************************/
SD_ERROR Init_SDCard()
{
	word i;
	SD_ERROR status=SD_OK;
	Init_SDIO();
	for(i=0;i<65535;i++){}
	status=SD_PowerON();
	if(status!=SD_OK)return status;
	status=SD_InitializeCard();
	SD_GetCardInfo(&SDInfo);
	status=SD_SelectCard(RCA);
	if(status!=SD_OK)return status;
	status=SD_EnableWideBusOperation(1);// 4bit
	if(status!=SD_OK)return status;

	if(SDIO_HIGH_CAPACITY_SD_CARD==CardType)
	{
		SDIO_CK_Set(SDIO_TRANSFER_CLK_DIV);//设置时钟频率,SDIO时钟计算公式:SDIO_CK时钟=SDIOCLK/[clkdiv+2];其中,SDIOCLK一般为72Mhz
		//status=SD_SetDeviceMode(SD_DMA_MODE);//设置为DMA模式
		status=SD_SetDeviceMode(SD_POLLING_MODE);//设置为查询模式
	}
	return status;
}

/**********************************************
  得到NumberOfBytes以2为底的指数.
NumberOfBytes:字节数.
返回值:以2为底的指数值
 **********************************************/
byte convert_from_bytes_to_power_of_two(word NumberOfBytes)
{
	byte count=0;
	while(NumberOfBytes!=1)
	{
		NumberOfBytes>>=1;
		count++;
	}
	return count;
}

/**********************************************
  获取卡的当前状态
curstatus:  card statu 12:9 bit
 **********************************************/
SD_ERROR GetSDCardMode(byte * curstatus)
{
	dword resp = 0;
	SD_ERROR status = SD_OK;
	SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);//发送CMD13
	status=CMDResponse1(SD_CMD_SEND_STATUS);
	if(status!=SD_OK)return status;
	resp=SDIO_RESP1;
	*curstatus=(byte)((resp>>9)&0x0000000F);
	if(0 == resp&SD_STAT_ERRORBITS)return SD_OK;
	else{
		return SD_Error;
	}
}

/**********************************************
  SD_ReadSingleBlock
  描述：从卡指定地址读取一个块的数据
  参数：addr_要读数据在卡内的地址
 *dataBuf_指向存储读取数据的存储区的指针
 blockSize_数据块大小,SD卡固定为512Byte
 **********************************************/
SD_ERROR SD_ReadSingleBlock(byte *dataBuf,dword addr,word blockSize)
{
	SD_ERROR status=SD_OK;
	dword count=0,*tempBuff=(dword *)dataBuf;
	byte power=0;
	dword timeout;

	if(0 == dataBuf)return SD_INVALID_PARAMETER;

	SDIO_DCTRL=0x0;	//数据控制寄存器清零(关DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);	//清除DPSM状态机配置
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){//大容量卡
		blockSize=512;
		addr>>=9;  // addr /= 512;
	}
	if((blockSize>0)&&(blockSize<=2048)&&((blockSize&(blockSize-1))==0))
	{
		power=convert_from_bytes_to_power_of_two(blockSize);
		//发送CMD16+设置数据长度为blockSize,短响应
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blockSize,1);
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN);	//等待R1响应
		if(status!=SD_OK)return status;
	}
	else{
		return SD_INVALID_PARAMETER;
	}
	SDIO_DataConfig(SD_DATATIMEOUT,blockSize,power,1);	//blockSize,卡到控制器

	TotalNumberOfBytes = blockSize;
	StopCondition = 0;
	//DestBuffer = dataBuf;
	//--------------发送CMD17-------------------//
	//从addr地址出读取数据,短响应
	SDIO_SendCMD(SD_CMD_READ_SINGLE_BLOCK,addr,1);
	status=CMDResponse1(SD_CMD_READ_SINGLE_BLOCK);//等待R1响应
	if(status!=SD_OK)return status;   		//响应错误

	if(DeviceMode==SD_POLLING_MODE){//查询模式,轮询数据
		//无上溢/CRC/超时/完成(标志)/起始位错误
		while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|
						SDIO_STA_RXOVERR|SDIO_STA_STBITERR|SDIO_STA_DBCKEND)))
		{
			if(SDIO_STA&SDIO_STA_RXFIFOHF){ //接收区半满,表示至少存了8个字
				for(count=0;count<8;count++) //循环读取数据
				{
					*(tempBuff+count)=SDIO_FIFO;
				}
				tempBuff+=8;
			}
		}
		if(SDIO_STA&SDIO_STA_DTIMEOUT){		//数据超时错误
			SDIO_ICR |= SDIO_ICR_DTIMEOUTC; 		//清错误标志
			return SD_DATA_TIMEOUT;
		}
		else if(SDIO_STA&SDIO_STA_DCRCFAIL){	//数据块CRC错误
			SDIO_ICR |= SDIO_ICR_DCRCFAILC; 		//清错误标志
			return SD_DATA_CRC_FAIL;
		}
		else if(SDIO_STA&SDIO_STA_RXOVERR){ 	//接收fifo上溢错误
			SDIO_ICR |= SDIO_ICR_RXOVERRC; 		//清错误标志
			return SD_RX_OVERRUN;
		}
		else if(SDIO_STA&SDIO_STA_STBITERR){ //接收起始位错误
			SDIO_ICR |= SDIO_ICR_STBITERRC; 		//清错误标志
			return SD_START_BIT_ERR;
		}
		// 读取FIFO缓冲区剩余的数据
		while(SDIO_STA&SDIO_STA_RXDAVL)
		{
			*tempBuff=SDIO_FIFO;	//循环读取数据
			tempBuff++;
		}
		SDIO_ICR=SDIO_STATIC_FLAGS;
	}
	else if(DeviceMode==SD_DMA_MODE)
	{
		TransferError=SD_OK;
		StopCondition=0;			//单块读,不需要发送停止传输指令
		TransferEnd=0;				//传输结束标置位，在中断服务置1
		SDIO_MASK = SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_RXOVERRIE|
			SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;	//配置需要的中断
		SDIO_DCTRL |= SDIO_DCTRL_DMAEN;		 	//SDIO DMA en
		SD_DMA2_Config((dword *)dataBuf,blockSize,0);
		timeout=SDIO_DATATIMEOUT;
		while(((DMA2_ISR&BIT13)==0)&&(TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;//等待传输完成
		if(timeout==0)return SD_DATA_TIMEOUT;//超时
		if(TransferError!=SD_OK)status=TransferError;
	}
	return status;


}

/**********************************************
//SD卡读取多个块
//buf:读数据缓存区
//addr:读取地址
//blksize:块大小
//nblks:要读取的块数
//返回值:错误状态
 **********************************************/
SD_ERROR SD_ReadMultiBlocks(byte *buf,dword addr,word blksize,dword nblks)
{
	SD_ERROR status=SD_OK;
	byte power;
	dword count=0,*tempbuff=(dword*)buf;//转换为dword指针
	dword timeout=0;

	if(0 == buf)return SD_INVALID_PARAMETER;

	SDIO_DCTRL=0x0;	//数据控制寄存器清零(关DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){//大容量卡
		blksize=512;
		addr>>=9;
	}
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0)){
		power=convert_from_bytes_to_power_of_two(blksize);
		//发送CMD16+设置数据长度为blksize,短响应
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blksize,1);
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN);	//等待R1响应
		if(status!=SD_OK)return status;   	//响应错误
	}
	else{
		return SD_INVALID_PARAMETER;
	}
	if(nblks>1){  //多块读
		if(nblks*blksize>SD_MAX_DATA_LENGTH){
			return SD_INVALID_PARAMETER;//判断是否超过最大接收长度
		}
		SDIO_DataConfig(SD_DATATIMEOUT,nblks*blksize,power,1);//nblks*blksize,512块大小,卡到控制器
		SDIO_SendCMD(SD_CMD_READ_MULT_BLOCK,addr,1);	//发送CMD18+从addr地址出读取数据,短响应
		status=CMDResponse1(SD_CMD_READ_MULT_BLOCK);//等待R1响应
		if(status!=SD_OK)return status;   	//响应错误

		if(DeviceMode==SD_POLLING_MODE){
			while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|
							SDIO_STA_RXOVERR|SDIO_STA_DATAEND|SDIO_STA_STBITERR)))//无上溢/CRC/超时/完成(标志)/起始位错误
			{
				if(SDIO_STA&SDIO_STA_RXFIFOHF){//接收区半满,表示至少存了8个字
					for(count=0;count<8;count++)//循环读取数据
					{
						*(tempbuff+count)=SDIO_FIFO;
					}
					tempbuff+=8;
				}
			}
			if(SDIO_STA&SDIO_STA_DTIMEOUT){   //数据超时错误
				SDIO_ICR |= SDIO_ICR_DTIMEOUTC;     //清错误标志
				return SD_DATA_TIMEOUT;
			}
			else if(SDIO_STA&SDIO_STA_DCRCFAIL){  //数据块CRC错误
				SDIO_ICR |= SDIO_ICR_DCRCFAILC;     //清错误标志
				return SD_DATA_CRC_FAIL;
			}
			else if(SDIO_STA&SDIO_STA_RXOVERR){   //接收fifo上溢错误
				SDIO_ICR |= SDIO_ICR_RXOVERRC;    //清错误标志
				return SD_RX_OVERRUN;
			}
			else if(SDIO_STA&SDIO_STA_STBITERR){ //接收起始位错误
				SDIO_ICR |= SDIO_ICR_STBITERRC;     //清错误标志
				return SD_START_BIT_ERR;
			}
			//----读出缓冲区剩余的数据
			while(SDIO_STA&SDIO_STA_RXDAVL){
				*tempbuff=SDIO_FIFO;
				tempbuff++;
			}
			if(SDIO_STA&SDIO_STA_DATAEND){		//接收结束
				if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
						(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
						(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
				{
					SDIO_SendCMD(SD_CMD_STOP_TRANSMISSION,0,1);//发送CMD12+结束传输
					status=CMDResponse1(SD_CMD_STOP_TRANSMISSION);//等待R1响应
					if(status!=SD_OK)return status;
				}
			}
			SDIO_ICR=SDIO_STATIC_FLAGS;	 		//清除所有标记
		}
		else if(DeviceMode==SD_DMA_MODE)
		{
			TransferError=SD_OK;
			StopCondition=1;			//多块读,需要发送停止传输指令
			TransferEnd=0;				//传输结束标置位，在中断服务置1
			SDIO_MASK=SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_RXOVERRIE|
				SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;

			SDIO_DCTRL |= SDIO_DCTRL_DMAEN;//SDIO DMA使能
			SD_DMA2_Config((dword*)buf,nblks*blksize,0);
			timeout=SDIO_DATATIMEOUT;
			while(((DMA2_ISR&BIT13)== 0)&&timeout)timeout--;//等待传输完成
			if(timeout==0)return SD_DATA_TIMEOUT;//超时
			while((TransferEnd==0)&&(TransferError==SD_OK));
			if(TransferError!=SD_OK)status=TransferError;
		}
	}
	return status;
}

/**********************************************
  SD卡写1个块
buf:数据缓存区
addr:写地址
blksize:块大小(byte)
返回值:错误状态
 **********************************************/
SD_ERROR SD_WriteBlock(byte *buf,dword addr,word blksize)
{
	SD_ERROR status = SD_OK;
	byte  power=0,cardstate=0;
	dword timeout=0,bytestransferred=0;
	dword cardstatus=0,count=0,restwords=0;
	dword tlen=blksize;//总长度(字节)
	dword*tempbuff=(dword*)buf;
	if(0 == buf)return SD_INVALID_PARAMETER;

	SDIO_DCTRL=0;//数据控制寄存器清零(关DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){  //大容量卡
		blksize=512;
		addr>>=9;
	}
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0)){
		power=convert_from_bytes_to_power_of_two(blksize);
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blksize,1); //发送CMD16,设置数据长度为blksize,短响应
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN); //等待R1响应
		if(status!=SD_OK)return status;
	}
	else return SD_INVALID_PARAMETER;

	SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);//发送CMD13,查询卡的状态,短响应
	status=CMDResponse1(SD_CMD_SEND_STATUS);//等待R1响应
	if(status!=SD_OK)return status;
	cardstatus=SDIO_RESP1;
	timeout=SD_DATATIMEOUT;
	while(((cardstatus&SD_STAT_READY_FOR_DATA)==0)&&(timeout>0))//检查READY_FOR_DATA位是否置位
	{
		timeout--;
		SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);//发送CMD13,查询卡的状态,短响应
		status=CMDResponse1(SD_CMD_SEND_STATUS);//等待R1响应
		if(status!=SD_OK)return status;
		cardstatus=SDIO_RESP1;
	}
	if(timeout==0)return SD_DATA_TIMEOUT;

	SDIO_SendCMD(SD_CMD_WRITE_SINGLE_BLOCK,addr,1);//发送CMD24,写单块指令,短响应
	status=CMDResponse1(SD_CMD_WRITE_SINGLE_BLOCK);//等待R1响应
	if(status!=SD_OK)return status;
	StopCondition=0;//单块写,不需要发送停止传输指令
	SDIO_DataConfig(SD_DATATIMEOUT,blksize,power,0);//blksize, 控制器到卡

	if(DeviceMode == SD_POLLING_MODE){
		while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|SDIO_STA_TXUNDERR|
						SDIO_STA_STBITERR|SDIO_STA_DBCKEND)))//数据块发送成功/下溢/CRC/超时/起始位错误
		{
			if(SDIO_STA&SDIO_STA_TXFIFOHE)//发送区半空,表示至少存了8个32bit字
			{
				if((tlen-bytestransferred)<SD_HALFFIFOBYTES)//不够32字节了
				{
					restwords=((tlen-bytestransferred)%4==0)?((tlen-bytestransferred)/4):((tlen-bytestransferred)/4+1);
					for(count=0;count<restwords;count++,tempbuff++,bytestransferred+=4)
					{
						SDIO_FIFO=*tempbuff;
					}
				}else{
					for(count=0;count<8;count++)
					{
						SDIO_FIFO=*(tempbuff+count);
					}
					tempbuff+=8;
					bytestransferred+=32;
				}
			}
		}
		if(SDIO_STA&SDIO_STA_DTIMEOUT){   //数据超时错误
			SDIO_ICR |= SDIO_ICR_DTIMEOUTC;     //清错误标志
			return SD_DATA_TIMEOUT;
		}
		else if(SDIO_STA&SDIO_STA_DCRCFAIL){  //数据块CRC错误
			SDIO_ICR |= SDIO_ICR_DCRCFAILC;     //清错误标志
			return SD_DATA_CRC_FAIL;
		}
		else if(SDIO_STA&SDIO_STA_TXUNDERR){   //接收fifo上溢错误
			SDIO_ICR |= SDIO_ICR_TXUNDERRC;    //清错误标志
			return SD_TX_UNDERRUN;
		}
		else if(SDIO_STA&SDIO_STA_STBITERR){ //接收起始位错误
			SDIO_ICR |= SDIO_ICR_STBITERRC;     //清错误标志
			return SD_START_BIT_ERR;
		}
		SDIO_ICR=SDIO_STATIC_FLAGS;      //清除所有标记
	}
	else if(DeviceMode==SD_DMA_MODE)
	{
		TransferError=SD_OK;
		StopCondition=0;      //单块写,不需要发送停止传输指令
		TransferEnd=0;        //传输结束标置位，在中断服务置1
		SDIO_MASK=SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_TXUNDERRIE|
			SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;
		SD_DMA2_Config((dword*)buf,blksize,1);//SDIO DMA配置
		SDIO_DCTRL |= SDIO_DCTRL_DMAEN;//SDIO DMA使能
		timeout=SDIO_DATATIMEOUT;
		while(((DMA2_ISR&BIT13)== 0 )&&timeout)timeout--;//等待传输完成
		if(timeout==0){
			Init_SDCard();//重新初始化SD卡,可以解决写入死机的问题
			return SD_DATA_TIMEOUT;//超时
		}
		timeout=SDIO_DATATIMEOUT;
		while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
		if(timeout==0)return SD_DATA_TIMEOUT;//超时
		if(TransferError!=SD_OK)return TransferError;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;//清除所有标记
	status=GetSDCardMode(&cardstate);
	while((status==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		status=GetSDCardMode(&cardstate);
	}
	return status;
}


/**********************************************
//SD卡写多个块
//buf:数据缓存区
//addr:写地址
//blksize:块大小
//nblks:要写入的块数
//返回值:错误状态

 **********************************************/
SD_ERROR SD_WriteMultiBlocks(byte *buf,dword addr,word blksize,dword nblks)
{
	SD_ERROR status = SD_OK;
	byte  power = 0, cardstate = 0;
	dword timeout=0,bytestransferred=0;
	dword count = 0, restwords = 0;
	dword tlen=nblks*blksize;       //总长度(字节)
	dword *tempbuff = (dword*)buf;

	if(0 == buf)return SD_INVALID_PARAMETER; //参数错误
	SDIO_DCTRL=0x0;              //数据控制寄存器清零(关DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){
		blksize=512;
		addr>>=9;
	}
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0)){
		power=convert_from_bytes_to_power_of_two(blksize);
		//发送CMD16+设置数据长度为blksize,短响应
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blksize,1);
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN);//等待R1响应
		if(status!=SD_OK)return status;//响应错误
	}
	else return SD_INVALID_PARAMETER;
	if(nblks>1){
		if(nblks*blksize>SD_MAX_DATA_LENGTH)return SD_INVALID_PARAMETER;
		if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
				(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
				(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
		{//提高性能
			SDIO_SendCMD(SD_CMD_APP_CMD,(dword)RCA<<16,1);
			status=CMDResponse1(SD_CMD_APP_CMD);
			if(status!=SD_OK)return status;
			//发送CMD23,设置块数量,短响应
			SDIO_SendCMD(SD_CMD_SET_BLOCK_COUNT,nblks,1);
			status=CMDResponse1(SD_CMD_SET_BLOCK_COUNT);
			if(status!=SD_OK)return status;
		}
		SDIO_SendCMD(SD_CMD_WRITE_MULT_BLOCK,addr,1);//发送CMD25,多块写指令,短响应
		status=CMDResponse1(SD_CMD_WRITE_MULT_BLOCK);
		if(status!=SD_OK)return status;
		SDIO_DataConfig(SD_DATATIMEOUT,nblks*blksize,power,0);//blksize, 控制器到卡

		if(DeviceMode==SD_POLLING_MODE){
			while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|SDIO_STA_TXUNDERR|
							SDIO_STA_STBITERR|SDIO_STA_DATAEND)))//下溢/CRC/数据结束/超时/起始位错误
			{
				if(SDIO_STA&SDIO_STA_TXFIFOHE){//发送区半空,表示至少存了8字(32字节)
					if((tlen-bytestransferred)<SD_HALFFIFOBYTES)//不够32字节了
					{
						restwords=((tlen-bytestransferred)%4==0)?((tlen-bytestransferred)/4):((tlen-bytestransferred)/4+1);
						for(count=0;count<restwords;count++,tempbuff++,bytestransferred+=4)
						{
							SDIO_FIFO=*tempbuff;
						}
					}else{//发送区半空,可以发送至少8字(32字节)数据
						for(count=0;count<SD_HALFFIFO;count++)
						{
							SDIO_FIFO=*(tempbuff+count);
						}
						tempbuff+=SD_HALFFIFO;
						bytestransferred+=SD_HALFFIFOBYTES;
					}
				}
			}
			if(SDIO_STA&SDIO_STA_DTIMEOUT){   //数据超时错误
				SDIO_ICR |= SDIO_ICR_DTIMEOUTC;
				return SD_DATA_TIMEOUT;
			}
			else if(SDIO_STA&SDIO_STA_DCRCFAIL){  //数据块CRC错误
				SDIO_ICR |= SDIO_ICR_DCRCFAILC;
				return SD_DATA_CRC_FAIL;
			}
			else if(SDIO_STA&SDIO_STA_TXUNDERR){   //接收fifo下溢错误
				SDIO_ICR |= SDIO_ICR_TXUNDERRC;
				return SD_TX_UNDERRUN;
			}
			else if(SDIO_STA&SDIO_STA_STBITERR){ //接收起始位错误
				SDIO_ICR |= SDIO_ICR_STBITERRC;
				return SD_START_BIT_ERR;
			}

			if(SDIO_STA&SDIO_STA_DATAEND){//发送结束
				if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
						(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
						(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
				{
					SDIO_SendCMD(SD_CMD_STOP_TRANSMISSION,0,1);//发送CMD12+结束传输
					status=CMDResponse1(SD_CMD_STOP_TRANSMISSION);//等待R1响应
					if(status!=SD_OK)return status;
				}
			}
			SDIO_ICR=SDIO_STATIC_FLAGS;      //清除所有标记
		}
		else if(DeviceMode==SD_DMA_MODE)
		{
			TransferError=SD_OK;
			StopCondition=1;//多块写,需要发送停止传输指令
			TransferEnd=0;//传输结束标置位，在中断服务置1
			SDIO_MASK=SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_TXUNDERRIE|
				SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;
			SD_DMA2_Config((dword*)buf,nblks*blksize,1);//SDIO DMA配置
			SDIO_DCTRL |= SDIO_DCTRL_DMAEN;//SDIO DMA使能
			timeout=SDIO_DATATIMEOUT;
			while(((DMA2_ISR&BIT13)== 0 )&&timeout)timeout--;//等待传输完成
			if(timeout==0){//超时
				Init_SDCard();//重新初始化SD卡,可以解决写入死机的问题
				return SD_DATA_TIMEOUT;//超时
			}
			timeout=SDIO_DATATIMEOUT;
			while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
			if(timeout==0)return SD_DATA_TIMEOUT;//超时
			if(TransferError!=SD_OK)return TransferError;
		}
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;//清除所有标记
	status=GetSDCardMode(&cardstate);
	while((status==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		status=GetSDCardMode(&cardstate);
	}
	return status;
}



/**********************************************
  SDIO中断服务函数
 **********************************************/
//void SDIO_IRQHandler(void)
//{
// 	SD_ProcessIRQSrc();//处理所有SDIO相关中断
//}

/**********************************************
  SDIO中断处理函数
  处理SDIO传输过程中的各种中断事务
 ***********************************************/
SD_ERROR SD_ProcessIRQSrc(void)
{
	if(SDIO_STA&SDIO_STA_DATAEND){//接收完成中断
		if(StopCondition==1){
			SDIO_SendCMD(SD_CMD_STOP_TRANSMISSION,0,1);//发送CMD12,结束传输
			TransferError=CMDResponse1(SD_CMD_STOP_TRANSMISSION);
		}else TransferError = SD_OK;

		SDIO_ICR |= SDIO_ICR_DATAENDC;//清除完成中断标记
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//关闭相关中断
		TransferEnd = 1;
		return(TransferError);
	}
	if(SDIO_STA&SDIO_STA_DCRCFAIL){//数据CRC错误
		SDIO_ICR |= SDIO_ICR_DCRCFAILC;//清除中断标记
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//关闭相关中断
		TransferError = SD_DATA_CRC_FAIL;
		return(SD_DATA_CRC_FAIL);
	}
	if(SDIO_STA&SDIO_STA_DTIMEOUT){//数据超时错误
		SDIO_ICR|=SDIO_ICR_DTIMEOUTC;//清除中断标记
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//关闭相关中断
		TransferError = SD_DATA_TIMEOUT;
		return(SD_DATA_TIMEOUT);
	}
	if(SDIO_STA&SDIO_STA_RXOVERR){//FIFO上溢错误
		SDIO_ICR|=SDIO_ICR_RXOVERRC;//清除中断标记
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//关闭相关中断
		TransferError = SD_RX_OVERRUN;
		return(SD_RX_OVERRUN);
	}
	if(SDIO_STA&SDIO_STA_TXUNDERR){//FIFO下溢错误
		SDIO_ICR|=SDIO_ICR_TXUNDERRC;//清除中断标记
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//关闭相关中断
		TransferError = SD_TX_UNDERRUN;
		return(SD_TX_UNDERRUN);
	}
	if(SDIO_STA&SDIO_STA_STBITERR){//起始位错误
		SDIO_ICR|=SDIO_ICR_STBITERRC;//清除中断标记
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//关闭相关中断
		TransferError = SD_START_BIT_ERR;
		return(SD_START_BIT_ERR);
	}
	return(SD_OK);
}


/**********************************************
//读SD卡
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
 **********************************************/
SD_ERROR SD_ReadDisk(byte*buf,dword sector,byte cnt)
{
	SD_ERROR status=SD_OK;
	byte n;
	if(CardType!=SDIO_STD_CAPACITY_SD_CARD_V1_1)sector<<=9;
	if((dword)buf%4!=0)
	{
		for(n=0;n<cnt;n++)
		{
			status=SD_ReadSingleBlock(SDIO_DATA_BUFFER,sector+512*n,512);//单个sector的读操作
			memcpy(buf,SDIO_DATA_BUFFER,512);
			buf+=512;
		}
	}else
	{
		if(cnt==1)status=SD_ReadSingleBlock(buf,sector,512);    	//单个sector的读操作
		else status=SD_ReadMultiBlocks(buf,sector,512,cnt);//多个sector
	}
	return status;
}

/**********************************************
//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
 **********************************************/
byte SD_WriteDisk(byte*buf,dword sector,byte cnt)
{
	byte sta=SD_OK;
	byte n;
	if(CardType!=SDIO_STD_CAPACITY_SD_CARD_V1_1)sector<<=9;
	if((dword)buf%4!=0)
	{
		for(n=0;n<cnt;n++)
		{
			memcpy(SDIO_DATA_BUFFER,buf,512);
			sta=SD_WriteBlock(SDIO_DATA_BUFFER,sector+512*n,512);//单个sector的写操作
			buf+=512;
		}
	}else
	{
		if(cnt==1)sta=SD_WriteBlock(buf,sector,512);    	//单个sector的写操作
		else sta=SD_WriteMultiBlocks(buf,sector,512,cnt);	//多个sector
	}
	return sta;
}

