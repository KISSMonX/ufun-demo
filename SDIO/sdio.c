
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

//�û�������
//SDIOʱ�Ӽ��㹫ʽ:SDIO_CKʱ��=SDIOCLK/[clkdiv+2];����,SDIOCLKһ��Ϊ72Mhz
//ʹ��DMAģʽ��ʱ��,�������ʿ��Ե�24Mhz,���������Ŀ����Ǹ��ٿ�,����Ҳ�����
//������뽵��ʱ��,ʹ�ò�ѯģʽ�Ļ�,�Ƽ�SDIO_TRANSFER_CLK_DIV����Ϊ3���߸���
#define SDIO_INIT_CLK_DIV        0xB2 		//SDIO��ʼ��Ƶ�ʣ����400Kh
#define SDIO_TRANSFER_CLK_DIV    0x02		//SDIO����Ƶ�ʣ����24Mhz(4bit)����ֵ̫С���ܻᵼ�¶�д�ļ�����

//SDIO����ģʽ����,ͨ��SD_SetDeviceMode��������.
#define SD_POLLING_MODE    	0 //��ѯģʽ,��ģʽ��,����SDIO_TRANSFER_CLK_DIV����Ϊ0X04.
#define SD_DMA_MODE    		  1	//DMAģʽ,��ģʽ��,SDIO_TRANSFER_CLK_DIV��������Ϊ1,�����д�����Ļ�,���Կ��ǽ�������SDIO_TRANSFER_CLK_DIV��ֵ
#define SD_INTERRUPT_MODE   2 //

dword CIDReg[4];    // cid register data
dword CSDReg[4];    // csd register data
word RCA;           // ����Ե�ַ
byte CardType;      // SD������
SD_CardInfo SDInfo; // SD��������Ϣ

byte SDIO_DATA_BUFFER[512];
byte DeviceMode;
SD_ERROR TransferError=SD_OK;
byte StopCondition=0;			//�����,����Ҫ����ֹͣ����ָ��
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
  SDIO �Ĵ�����ʼ����ΪSD���ϵ��ʼ����׼��
  SDIO��ʼ��Ƶ�ʣ����400Kh
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
  SDIO���������,�޵ȴ��ж�����
cmdindex:��������,����λ��Ч
arg:����
waitResp:�ڴ�����Ӧ.00/10,����Ӧ;01,����Ӧ;11,����Ӧ
 ***********************************************/
void SDIO_SendCMD(byte cmdindex,dword arg,byte waitResp)
{
	SDIO_ARG=arg;
	SDIO_CMD &= 0xfffff800;
	SDIO_CMD |= cmdindex;
	SDIO_CMD |= waitResp<<6;
	SDIO_CMD |= SDIO_CMD_CPSMEN; //����ͨ��״̬��ʹ��
}
/**********************************************
  SDIO�����������ú���
datatimeout  :���ݳ�ʱʱ��
datalen      :���ݳ��� �ֽڳ���
blocksize    :���ݿ��С 2^size(bit)
datadir      :���ݷ��� 0:to card, 1:to sdio
 ***********************************************/
void SDIO_DataConfig(dword datatimeout,dword datalen,byte blocksize,byte datadir)
{
	SDIO_DTIMER=datatimeout;
	SDIO_DLEN=datalen;
	SDIO_DCTRL &= 0xff08;
	SDIO_DCTRL |= blocksize<<4;
	//SDIO_DCTRL &= ~SDIO_DCTRL_DTMODE; //�����ݴ���
	SDIO_DCTRL |= (datadir&1)<<1;
	SDIO_DCTRL |= SDIO_DCTRL_DTEN;  //���ݴ���ʹ��
}

/**********************************************
//����SDIO DMA  (DMA2->Channel 4)
//mbuf:�洢����ַ
//bufsize:����������,�ֽ���
//dir:����;1,�洢��->SDIO(д����);0,SDIO->�洢��(������);
 **********************************************/
void SD_DMA2_Config(dword*mbuf,dword bufsize,byte dir)
{
	DMA2_IFCR &= 0xffff0fff;  //���DMA2ͨ��4�ĸ��ֱ��
	DMA2_CCR4 = 0;

	DMA2_CCR4 |= dir<<4;//�Ӵ洢����
	DMA2_CCR4 |= BIT7;//�洢������ģʽ
	DMA2_CCR4 |= BIT9;//�������ݿ��32bit
	DMA2_CCR4 |= BIT11;//�洢�����ݿ��32bit
	DMA2_CCR4 |= BIT13;//ͨ�����ȼ�Ϊ��

	DMA2_CNDTR4=bufsize>>2;//DMA2,���������� byte->dword
	DMA2_CPAR4=(dword)&SDIO_FIFO;//DMA2 �����ַ
	DMA2_CMAR4=(dword)mbuf; 		//DMA2,�洢����ַ
	DMA2_CCR4 |= BIT0;// enable DMA Channel

}


/**********************************************
  ����CMD0����Ӧ
  �ȴ��������ɣ�������Ӧ��ʱ
 ***********************************************/
SD_ERROR CMD0Response()
{
	dword timeout=SDIO_CMD0TIMEOUT;
	while(timeout--)
	{
		if(SDIO_STA&SDIO_STA_CMDSENT)break;
	}
	if(0 == timeout)return SD_CMD_RSP_TIMEOUT;
	SDIO_ICR=SDIO_STATIC_FLAGS;  //������
	return SD_OK;
}

/**********************************************
  R1��Ӧ
 ***********************************************/
SD_ERROR CMDResponse1(byte CMD)
{
	dword value;
	while(1)    // �ȴ�R1��Ӧ
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRCУ�����
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   //cmd��Ӧ��ʱ
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	if(SDIO_RESPCMD != CMD)return SD_ILLEGAL_CMD;
	SDIO_ICR=SDIO_STATIC_FLAGS;  //������
	if(SDIO_RESP1&SD_STAT_ERRORBITS)return SD_Error;
	else{
		return SD_OK;
	}
}


/**********************************************
  R2��Ӧ(CID/CSD)
 ***********************************************/
SD_ERROR CMDResponse2()
{
	dword value;
	while(1)    // �ȴ�R1��Ӧ
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRCУ�����
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   // cmd��Ӧ��ʱ
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;  //������

	return SD_OK;
}



/**********************************************
  R3��Ӧ(OCR)
Commandindex :default 111,111
CRC7         :default 111,111
 ***********************************************/
SD_ERROR CMDResponse3()
{
	dword value;
	while(1)    // �ȴ�R1��Ӧ
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   // cmd��Ӧ��ʱ
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}

	SDIO_ICR=SDIO_STATIC_FLAGS;  //������

	return SD_OK;
}

/**********************************************
  R6��Ӧ(RCA)
 ***********************************************/
SD_ERROR CMDResponse6(byte CMD,word *prca)
{
	dword value;
	dword resp;
	while(1)    // �ȴ�R1��Ӧ
	{
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CTIMEOUT){   // cmd��Ӧ��ʱ
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRCУ�����
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(SDIO_RESPCMD != CMD)return SD_ILLEGAL_CMD;//�ж��Ƿ���Ӧcmd����
	SDIO_ICR=SDIO_STATIC_FLAGS;  //������

	resp=SDIO_RESP1;
	if(SD_ALLZERO == (resp&(SD_R6_GENERAL_UNKNOWN_ERROR|SD_R6_ILLEGAL_CMD|SD_R6_COM_CRC_FAILED)))
	{
		*prca=(word)(resp>>16);			//����16λ�õ�,rca
		return SD_OK;
	}
	if(resp&SD_R6_GENERAL_UNKNOWN_ERROR)return SD_GENERAL_UNKNOWN_ERROR;
	if(resp&SD_R6_ILLEGAL_CMD)return SD_ILLEGAL_CMD;
	if(resp&SD_R6_COM_CRC_FAILED)return SD_COM_CRC_FAILED;
	return SD_OK;
}

/**********************************************
  R7��Ӧ
 ***********************************************/
SD_ERROR CMDResponse7()   // ��R2һ��?
{
	dword value;
	dword timeout=SDIO_CMD0TIMEOUT;
	while(timeout--){
		value=SDIO_STA_CCRCFAIL|SDIO_STA_CTIMEOUT|SDIO_STA_CMDREND;
		if(SDIO_STA & value)break;
	}
	if(SDIO_STA & SDIO_STA_CCRCFAIL){   //CRCУ�����
		SDIO_ICR = SDIO_ICR_CCRCFAILC;
		return SD_CMD_CRC_FAIL;
	}
	if(0==timeout || SDIO_STA & SDIO_STA_CTIMEOUT){// cmd��Ӧ��ʱ
		SDIO_ICR = SDIO_ICR_CTIMEOUTC;
		return SD_CMD_RSP_TIMEOUT;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;  //������
	return SD_OK;
}

/**********************************************
  SD card power on
  SD���ϵ����
  ��ѯsd�����ϵĿ��豸������ѯ���ѹ����
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
		SDType=SD_HIGH_CAPACITY;//��������
		CardType=SDIO_STD_CAPACITY_SD_CARD_V2_0;// 2.0��
	}
	// CMD55  ��������SD2.0��MMC��������Ӧ��ΪSD
	SDIO_SendCMD(SD_CMD_APP_CMD,0,1);
	status=CMDResponse1(SD_CMD_APP_CMD);
	if(status==SD_OK){//SD2.0/SD 1.1
		//SD��,����ACMD41 SD_APP_OP_COND,����Ϊ:0x80100000
		while((!BusyFlag)&&(count<SD_MAX_VOLT_TRIAL))
		{
			SDIO_SendCMD(SD_CMD_APP_CMD,0,1);			 //����CMD55,����Ӧ
			status=CMDResponse1(SD_CMD_APP_CMD);  //�ȴ�R1��Ӧ
			if(status!=SD_OK)return status;   	   //��Ӧ����
			// -----Send ACMD41-----//
			//����ACMD41,����Ӧ
			SDIO_SendCMD(SD_CMD_SD_APP_OP_COND,SD_VOLTAGE_WINDOW_SD|SDType,1);
			status=CMDResponse3(); 					     //�ȴ�R3��Ӧ
			if(status!=SD_OK)return status;        //��Ӧ����
			response=SDIO_RESP1;                   //�õ���Ӧ
			BusyFlag=(((response>>31)==1)?1:0);//�ж�SD���ϵ��Ƿ����
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL){
			return SD_INVALID_VOLTRANGE;
		}
		if(response&=SD_HIGH_CAPACITY){
			CardType=SDIO_HIGH_CAPACITY_SD_CARD;
		}
	}
	else{    //MMC��,����CMD1 SDIO_SEND_OP_COND,����Ϊ:0x80FF8000
		while((!BusyFlag)&&(count<SD_MAX_VOLT_TRIAL))
		{
			//����CMD1,����Ӧ
			SDIO_SendCMD(SD_CMD_SEND_OP_COND,SD_VOLTAGE_WINDOW_MMC,1);
			status=CMDResponse3(); 					//�ȴ�R3��Ӧ
			if(status!=SD_OK)return status;   //��Ӧ����
			response=SDIO_RESP1;              //�õ���Ӧ
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
  SD����ʼ������
  ���������״̬
  �ȷ���CMD2,��ȡCID
  �ٷ���CMD3,��ȡRCA
  �����CMD9,��ȡCSD
 ***********************************************/

SD_ERROR SD_InitializeCard()
{
	SD_ERROR status=SD_OK;
	word rca=1;
	if(0 == SDIO_POWER & SDIO_POWER_PWRCTRL_ON)return SD_REQUEST_NOT_APPLICABLE;
	if(SDIO_SECURE_DIGITAL_IO_CARD != CardType){//��SECURE_DIGITAL_IO_CARD
		//Send CMD2 ALL_SEND_CID
		//resp: R2
		SDIO_SendCMD(SD_CMD_ALL_SEND_CID,0,3);// ����Ӧ,read CID
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
			(SDIO_HIGH_CAPACITY_SD_CARD == CardType))//�жϿ�����
	{
		// Send CMD3 SD_CMD_SET_REL_ADDR with argument 0
		// resp :R6
		SDIO_SendCMD(SD_CMD_SET_REL_ADDR,0,1);
		status=CMDResponse6(SD_CMD_SET_REL_ADDR,&rca);//����RCA
		if(status != SD_OK)return status;
	}
	else if (SDIO_MULTIMEDIA_CARD == CardType){  // MMC
		SDIO_SendCMD(SD_CMD_SET_REL_ADDR,(dword)(rca<<16),1);//����CMD3,����Ӧ
		status=CMDResponse2();           //�ȴ�R2��Ӧ
		if(status!=SD_OK)return status;   //��Ӧ����
	}
	//��SECURE_DIGITAL_IO_CARD
	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType){
		RCA = rca;
		//����CMD9+��RCA,ȡ��CSD,����Ӧ
		SDIO_SendCMD(SD_CMD_SEND_CSD,(dword)(rca<<16),3);
		status=CMDResponse2(); 					//�ȴ�R2��Ӧ
		if(status!=SD_OK)return status;   	//��Ӧ����
		CSDReg[0]=SDIO_RESP1;
		CSDReg[1]=SDIO_RESP2;
		CSDReg[2]=SDIO_RESP3;
		CSDReg[3]=SDIO_RESP4;
	}
	return SD_OK;//����ʼ���ɹ�

}

/**********************************************
  ����֮ǰ��ȡ��SD Card �Ĵ���,�ó�SD Card �Ĳ���

 **********************************************/


void SD_GetCardInfo(SD_CardInfo *cardinfo)
{
	byte tmp=0;
	cardinfo->CardType=(byte)CardType;        //������
	cardinfo->RCA=(word)RCA;             //��RCAֵ
	tmp=(byte)((CSDReg[0]&0xFF000000)>>24);
	cardinfo->SD_csd.CSDStruct=(tmp&0xC0)>>6;   //CSD�ṹ
	cardinfo->SD_csd.SysSpecVersion=(tmp&0x3C)>>2;  //2.0Э�黹û�����ⲿ��(Ϊ����),Ӧ���Ǻ���Э�鶨���
	cardinfo->SD_csd.Reserved1=tmp&0x03;      //2������λ
	tmp=(byte)((CSDReg[0]&0x00FF0000)>>16);      //��1���ֽ�
	cardinfo->SD_csd.TAAC=tmp;              //���ݶ�ʱ��1
	tmp=(byte)((CSDReg[0]&0x0000FF00)>>8);       //��2���ֽ�
	cardinfo->SD_csd.NSAC=tmp;              //���ݶ�ʱ��2
	tmp=(byte)(CSDReg[0]&0x000000FF);        //��3���ֽ�
	cardinfo->SD_csd.MaxBusClkFrec=tmp;         //�����ٶ�
	tmp=(byte)((CSDReg[1]&0xFF000000)>>24);      //��4���ֽ�
	cardinfo->SD_csd.CardComdClasses=tmp<<4;      //��ָ�������λ
	tmp=(byte)((CSDReg[1]&0x00FF0000)>>16);      //��5���ֽ�
	cardinfo->SD_csd.CardComdClasses|=(tmp&0xF0)>>4;//��ָ�������λ
	cardinfo->SD_csd.RdBlockLen=tmp&0x0F;       //����ȡ���ݳ���
	tmp=(byte)((CSDReg[1]&0x0000FF00)>>8);     //��6���ֽ�
	cardinfo->SD_csd.PartBlockRead=(tmp&0x80)>>7; //����ֿ��
	cardinfo->SD_csd.WrBlockMisalign=(tmp&0x40)>>6; //д���λ
	cardinfo->SD_csd.RdBlockMisalign=(tmp&0x20)>>5; //�����λ
	cardinfo->SD_csd.DSRImpl=(tmp&0x10)>>4;
	cardinfo->SD_csd.Reserved2=0;           //����
	if((CardType==SDIO_STD_CAPACITY_SD_CARD_V1_1)||
			(CardType==SDIO_STD_CAPACITY_SD_CARD_V2_0)||
			(SDIO_MULTIMEDIA_CARD==CardType))       //��׼1.1/2.0��/MMC��
	{
		cardinfo->SD_csd.DeviceSize=(tmp&0x03)<<10; //C_SIZE(12λ)
		tmp=(byte)(CSDReg[1]&0x000000FF);      //��7���ֽ�
		cardinfo->SD_csd.DeviceSize|=(tmp)<<2;
		tmp=(byte)((CSDReg[2]&0xFF000000)>>24);    //��8���ֽ�
		cardinfo->SD_csd.DeviceSize|=(tmp&0xC0)>>6;
		cardinfo->SD_csd.MaxRdCurrentVDDMin=(tmp&0x38)>>3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax=(tmp&0x07);
		tmp=(byte)((CSDReg[2]&0x00FF0000)>>16);    //��9���ֽ�
		cardinfo->SD_csd.MaxWrCurrentVDDMin=(tmp&0xE0)>>5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax=(tmp&0x1C)>>2;
		cardinfo->SD_csd.DeviceSizeMul=(tmp&0x03)<<1;//C_SIZE_MULT
		tmp=(byte)((CSDReg[2]&0x0000FF00)>>8);     //��10���ֽ�
		cardinfo->SD_csd.DeviceSizeMul|=(tmp&0x80)>>7;
		cardinfo->CardCapacity=(cardinfo->SD_csd.DeviceSize+1);//���㿨����
		cardinfo->CardCapacity*=(1<<(cardinfo->SD_csd.DeviceSizeMul+2));
		cardinfo->CardBlockSize=1<<(cardinfo->SD_csd.RdBlockLen);//���С
		cardinfo->CardCapacity*=cardinfo->CardBlockSize;
	}else if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)  //��������
	{
		tmp=(byte)(CSDReg[1]&0x000000FF);    //��7���ֽ�
		cardinfo->SD_csd.DeviceSize=(tmp&0x3F)<<16;//C->SIZE
		tmp=(byte)((CSDReg[2]&0xFF000000)>>24);  //��8���ֽ�
		cardinfo->SD_csd.DeviceSize|=(tmp<<8);
		tmp=(byte)((CSDReg[2]&0x00FF0000)>>16);  //��9���ֽ�
		cardinfo->SD_csd.DeviceSize|=(tmp);
		tmp=(byte)((CSDReg[2]&0x0000FF00)>>8);   //��10���ֽ�
		cardinfo->CardCapacity=(long long)(cardinfo->SD_csd.DeviceSize+1)*512*1024;//���㿨����
		cardinfo->CardBlockSize=512;      //���С�̶�Ϊ512�ֽ�
	}
	cardinfo->SD_csd.EraseGrSize=(tmp&0x40)>>6;
	cardinfo->SD_csd.EraseGrMul=(tmp&0x3F)<<1;
	tmp=(byte)(CSDReg[2]&0x000000FF);      //��11���ֽ�
	cardinfo->SD_csd.EraseGrMul|=(tmp&0x80)>>7;
	cardinfo->SD_csd.WrProtectGrSize=(tmp&0x7F);
	tmp=(byte)((CSDReg[3]&0xFF000000)>>24);    //��12���ֽ�
	cardinfo->SD_csd.WrProtectGrEnable=(tmp&0x80)>>7;
	cardinfo->SD_csd.ManDeflECC=(tmp&0x60)>>5;
	cardinfo->SD_csd.WrSpeedFact=(tmp&0x1C)>>2;
	cardinfo->SD_csd.MaxWrBlockLen=(tmp&0x03)<<2;
	tmp=(byte)((CSDReg[3]&0x00FF0000)>>16);    //��13���ֽ�
	cardinfo->SD_csd.MaxWrBlockLen|=(tmp&0xC0)>>6;
	cardinfo->SD_csd.WriteBlockPaPartial=(tmp&0x20)>>5;
	cardinfo->SD_csd.Reserved3=0;
	cardinfo->SD_csd.ContentProtectAppli=(tmp&0x01);
	tmp=(byte)((CSDReg[3]&0x0000FF00)>>8);   //��14���ֽ�
	cardinfo->SD_csd.FileFormatGrouop=(tmp&0x80)>>7;
	cardinfo->SD_csd.CopyFlag=(tmp&0x40)>>6;
	cardinfo->SD_csd.PermWrProtect=(tmp&0x20)>>5;
	cardinfo->SD_csd.TempWrProtect=(tmp&0x10)>>4;
	cardinfo->SD_csd.FileFormat=(tmp&0x0C)>>2;
	cardinfo->SD_csd.ECC=(tmp&0x03);
	tmp=(byte)(CSDReg[3]&0x000000FF);      //��15���ֽ�
	cardinfo->SD_csd.CSD_CRC=(tmp&0xFE)>>1;
	cardinfo->SD_csd.Reserved4=1;

	tmp=(byte)((CIDReg[0]&0xFF000000)>>24);    //��0���ֽ�
	cardinfo->SD_cid.MID=tmp;
	tmp=(byte)((CIDReg[0]&0x00FF0000)>>16);    //��1���ֽ�
	cardinfo->SD_cid.OID=tmp<<8;
	tmp=(byte)((CIDReg[0]&0x000000FF00)>>8);   //��2���ֽ�
	cardinfo->SD_cid.OID|=tmp;
	tmp=(byte)(CIDReg[0]&0x000000FF);      //��3���ֽ�
	cardinfo->SD_cid.PNM1=tmp<<24;
	tmp=(byte)((CIDReg[1]&0xFF000000)>>24);    //��4���ֽ�
	cardinfo->SD_cid.PNM1|=tmp<<16;
	tmp=(byte)((CIDReg[1]&0x00FF0000)>>16);      //��5���ֽ�
	cardinfo->SD_cid.PNM1|=tmp<<8;
	tmp=(byte)((CIDReg[1]&0x0000FF00)>>8);   //��6���ֽ�
	cardinfo->SD_cid.PNM1|=tmp;
	tmp=(byte)(CIDReg[1]&0x000000FF);        //��7���ֽ�
	cardinfo->SD_cid.PNM2=tmp;
	tmp=(byte)((CIDReg[2]&0xFF000000)>>24);    //��8���ֽ�
	cardinfo->SD_cid.PRV=tmp;
	tmp=(byte)((CIDReg[2]&0x00FF0000)>>16);    //��9���ֽ�
	cardinfo->SD_cid.PSN=tmp<<24;
	tmp=(byte)((CIDReg[2]&0x0000FF00)>>8);     //��10���ֽ�
	cardinfo->SD_cid.PSN|=tmp<<16;
	tmp=(byte)(CIDReg[2]&0x000000FF);        //��11���ֽ�
	cardinfo->SD_cid.PSN|=tmp<<8;
	tmp=(byte)((CIDReg[3]&0xFF000000)>>24);    //��12���ֽ�
	cardinfo->SD_cid.PSN|=tmp;
	tmp=(byte)((CIDReg[3]&0x00FF0000)>>16);    //��13���ֽ�
	cardinfo->SD_cid.Reserved1|=(tmp&0xF0)>>4;
	cardinfo->SD_cid.MDT=(tmp&0x0F)<<8;
	tmp=(byte)((CIDReg[3]&0x0000FF00)>>8);   //��14���ֽ�
	cardinfo->SD_cid.MDT|=tmp;
	tmp=(byte)(CIDReg[3]&0x000000FF);      //��15���ֽ�
	cardinfo->SD_cid.CIDCRC=(tmp&0xFE)>>1;
	cardinfo->SD_cid.Reserved2=1;

}

/**********************************************
  ��ȡ SD Card SCR register
rca:����Ե�ַ
pscr:���ݻ�����(�洢SCR����)
 **********************************************/
SD_ERROR ReadSCR(word rca,dword *pscr)
{
	dword index = 0;
	SD_ERROR status = SD_OK;
	dword tempscr[2]={0,0};

	//Send CMD16: Set Block Size To 8 Bytes
	SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,8,1);//����Ӧ,����Block SizeΪ8�ֽ�
	status=CMDResponse1(SD_CMD_SET_BLOCKLEN);
	if(status!=SD_OK)return status;

	//Send CMD55 APP_CMD with argument as card's RCA
	SDIO_SendCMD(SD_CMD_APP_CMD,(dword)rca<<16,1);
	status=CMDResponse1(SD_CMD_APP_CMD);
	if(status!=SD_OK)return status;

	//Send ACMD51 to read SCR register(64bit)
	// before the cmd, config return data format
	SDIO_DataConfig(SD_DATATIMEOUT,8,3,1); //8���ֽڳ���,blockΪ8�ֽ�,SD����SDIO.
	SDIO_SendCMD(SD_CMD_SD_APP_SEND_SCR,0,1);  //����ACMD51,����Ӧ,����Ϊ0
	status=CMDResponse1(SD_CMD_SD_APP_SEND_SCR);
	if(status!=SD_OK)return status;

	while(!(SDIO_STA&(SDIO_STA_DBCKEND|SDIO_STA_STBITERR|SDIO_STA_RXOVERR
					|SDIO_STA_DTIMEOUT|SDIO_STA_DCRCFAIL)))
	{
		if(SDIO_STA&SDIO_STA_RXDAVL){//����FIFO���ݿ���
			*(tempscr+index)=SDIO_FIFO;	//��ȡFIFO����
			index++;
			if(index>=2)break;
		}
	}
	if(SDIO_STA&SDIO_STA_DTIMEOUT){		//�������ݳ�ʱ
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
	SDIO_ICR=SDIO_STATIC_FLAGS;	 		//������б��
	//������˳��8λΪ��λ������.
	*(pscr+1)=((tempscr[0]&SD_0TO7BITS)<<24)|((tempscr[0]&SD_8TO15BITS)<<8)|((tempscr[0]&SD_16TO23BITS)>>8)|((tempscr[0]&SD_24TO31BITS)>>24);
	*(pscr)=((tempscr[1]&SD_0TO7BITS)<<24)|((tempscr[1]&SD_8TO15BITS)<<8)|((tempscr[1]&SD_16TO23BITS)>>8)|((tempscr[1]&SD_24TO31BITS)>>24);
	return status;
}


/**********************************************
  SD Card ʹ�ܿ�����ģʽ
enable:0,��ʹ��;1,ʹ��;
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
	if(SDIO_RESP1&SD_STAT_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//SD������LOCKED״̬

	// get SCR register
	status=ReadSCR(RCA,scr);						//�õ�SCR�Ĵ�������
	if(status!=SD_OK)return status;

	if((scr[1]&SD_WIDE_BUS_SUPPORT)!=SD_ALLZERO){		//֧�ֿ�����
		//����CMD55+RCA,����Ӧ
		SDIO_SendCMD(SD_CMD_APP_CMD,(dword)RCA<<16,1);
		status=CMDResponse1(SD_CMD_APP_CMD);
		if(status!=SD_OK)return status;
		//����ACMD6,����Ӧ,����:10,4λ;00,1λ
		SDIO_SendCMD(SD_CMD_APP_SD_SET_BUSWIDTH,arg,1);
		status=CMDResponse1(SD_CMD_APP_SD_SET_BUSWIDTH);
		return status;
	}else return SD_REQUEST_NOT_APPLICABLE;//��֧�ֿ���������
}


/**********************************************
  ����SD����SDIO���߿��(MMC����֧��4bitģʽ)
wmode  : WideMode: Specifies the SD card wide bus mode.
- 2: 8-bit data transfer (Only for MMC)
- 1: 4-bit data transfer
- 0: 1-bit data transfer
 **********************************************/
SD_ERROR SD_EnableWideBusOperation(dword wmode)
{
	SD_ERROR status=SD_OK;
	if(SDIO_MULTIMEDIA_CARD==CardType)return SD_UNSUPPORTED_FEATURE;//MMC����֧��
	else if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
			(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
			(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
	{
		if(wmode>=2)return SD_UNSUPPORTED_FEATURE;//��֧��8λģʽ
		else{
			status=SDCardEnWideBus(wmode);//�趨SD�������߿��
			if(SD_OK==status){
				SDIO_CLKCR&=~(3<<11);		//���֮ǰ��λ������
				SDIO_CLKCR|=(word)wmode<<11;//1λ/4λ���߿��
				SDIO_CLKCR|=0<<14;			//������Ӳ��������
			}
		}
	}
	return status;
}
/**********************************************
  ����SD������ģʽ
Mode:
����ֵ:����״̬
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
  ѡ��/��ѡ�п� CMD7
  ���룺����Ե�ַ RCA
  ����: ��RCAΪ0ʱ�����п�����stand-by
  RCA��ȷʱ����Transfer mode
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
  ��ʼ�� SD card
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
		SDIO_CK_Set(SDIO_TRANSFER_CLK_DIV);//����ʱ��Ƶ��,SDIOʱ�Ӽ��㹫ʽ:SDIO_CKʱ��=SDIOCLK/[clkdiv+2];����,SDIOCLKһ��Ϊ72Mhz
		//status=SD_SetDeviceMode(SD_DMA_MODE);//����ΪDMAģʽ
		status=SD_SetDeviceMode(SD_POLLING_MODE);//����Ϊ��ѯģʽ
	}
	return status;
}

/**********************************************
  �õ�NumberOfBytes��2Ϊ�׵�ָ��.
NumberOfBytes:�ֽ���.
����ֵ:��2Ϊ�׵�ָ��ֵ
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
  ��ȡ���ĵ�ǰ״̬
curstatus:  card statu 12:9 bit
 **********************************************/
SD_ERROR GetSDCardMode(byte * curstatus)
{
	dword resp = 0;
	SD_ERROR status = SD_OK;
	SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);//����CMD13
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
  �������ӿ�ָ����ַ��ȡһ���������
  ������addr_Ҫ�������ڿ��ڵĵ�ַ
 *dataBuf_ָ��洢��ȡ���ݵĴ洢����ָ��
 blockSize_���ݿ��С,SD���̶�Ϊ512Byte
 **********************************************/
SD_ERROR SD_ReadSingleBlock(byte *dataBuf,dword addr,word blockSize)
{
	SD_ERROR status=SD_OK;
	dword count=0,*tempBuff=(dword *)dataBuf;
	byte power=0;
	dword timeout;

	if(0 == dataBuf)return SD_INVALID_PARAMETER;

	SDIO_DCTRL=0x0;	//���ݿ��ƼĴ�������(��DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);	//���DPSM״̬������
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){//��������
		blockSize=512;
		addr>>=9;  // addr /= 512;
	}
	if((blockSize>0)&&(blockSize<=2048)&&((blockSize&(blockSize-1))==0))
	{
		power=convert_from_bytes_to_power_of_two(blockSize);
		//����CMD16+�������ݳ���ΪblockSize,����Ӧ
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blockSize,1);
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN);	//�ȴ�R1��Ӧ
		if(status!=SD_OK)return status;
	}
	else{
		return SD_INVALID_PARAMETER;
	}
	SDIO_DataConfig(SD_DATATIMEOUT,blockSize,power,1);	//blockSize,����������

	TotalNumberOfBytes = blockSize;
	StopCondition = 0;
	//DestBuffer = dataBuf;
	//--------------����CMD17-------------------//
	//��addr��ַ����ȡ����,����Ӧ
	SDIO_SendCMD(SD_CMD_READ_SINGLE_BLOCK,addr,1);
	status=CMDResponse1(SD_CMD_READ_SINGLE_BLOCK);//�ȴ�R1��Ӧ
	if(status!=SD_OK)return status;   		//��Ӧ����

	if(DeviceMode==SD_POLLING_MODE){//��ѯģʽ,��ѯ����
		//������/CRC/��ʱ/���(��־)/��ʼλ����
		while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|
						SDIO_STA_RXOVERR|SDIO_STA_STBITERR|SDIO_STA_DBCKEND)))
		{
			if(SDIO_STA&SDIO_STA_RXFIFOHF){ //����������,��ʾ���ٴ���8����
				for(count=0;count<8;count++) //ѭ����ȡ����
				{
					*(tempBuff+count)=SDIO_FIFO;
				}
				tempBuff+=8;
			}
		}
		if(SDIO_STA&SDIO_STA_DTIMEOUT){		//���ݳ�ʱ����
			SDIO_ICR |= SDIO_ICR_DTIMEOUTC; 		//������־
			return SD_DATA_TIMEOUT;
		}
		else if(SDIO_STA&SDIO_STA_DCRCFAIL){	//���ݿ�CRC����
			SDIO_ICR |= SDIO_ICR_DCRCFAILC; 		//������־
			return SD_DATA_CRC_FAIL;
		}
		else if(SDIO_STA&SDIO_STA_RXOVERR){ 	//����fifo�������
			SDIO_ICR |= SDIO_ICR_RXOVERRC; 		//������־
			return SD_RX_OVERRUN;
		}
		else if(SDIO_STA&SDIO_STA_STBITERR){ //������ʼλ����
			SDIO_ICR |= SDIO_ICR_STBITERRC; 		//������־
			return SD_START_BIT_ERR;
		}
		// ��ȡFIFO������ʣ�������
		while(SDIO_STA&SDIO_STA_RXDAVL)
		{
			*tempBuff=SDIO_FIFO;	//ѭ����ȡ����
			tempBuff++;
		}
		SDIO_ICR=SDIO_STATIC_FLAGS;
	}
	else if(DeviceMode==SD_DMA_MODE)
	{
		TransferError=SD_OK;
		StopCondition=0;			//�����,����Ҫ����ֹͣ����ָ��
		TransferEnd=0;				//�����������λ�����жϷ�����1
		SDIO_MASK = SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_RXOVERRIE|
			SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;	//������Ҫ���ж�
		SDIO_DCTRL |= SDIO_DCTRL_DMAEN;		 	//SDIO DMA en
		SD_DMA2_Config((dword *)dataBuf,blockSize,0);
		timeout=SDIO_DATATIMEOUT;
		while(((DMA2_ISR&BIT13)==0)&&(TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;//�ȴ��������
		if(timeout==0)return SD_DATA_TIMEOUT;//��ʱ
		if(TransferError!=SD_OK)status=TransferError;
	}
	return status;


}

/**********************************************
//SD����ȡ�����
//buf:�����ݻ�����
//addr:��ȡ��ַ
//blksize:���С
//nblks:Ҫ��ȡ�Ŀ���
//����ֵ:����״̬
 **********************************************/
SD_ERROR SD_ReadMultiBlocks(byte *buf,dword addr,word blksize,dword nblks)
{
	SD_ERROR status=SD_OK;
	byte power;
	dword count=0,*tempbuff=(dword*)buf;//ת��Ϊdwordָ��
	dword timeout=0;

	if(0 == buf)return SD_INVALID_PARAMETER;

	SDIO_DCTRL=0x0;	//���ݿ��ƼĴ�������(��DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){//��������
		blksize=512;
		addr>>=9;
	}
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0)){
		power=convert_from_bytes_to_power_of_two(blksize);
		//����CMD16+�������ݳ���Ϊblksize,����Ӧ
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blksize,1);
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN);	//�ȴ�R1��Ӧ
		if(status!=SD_OK)return status;   	//��Ӧ����
	}
	else{
		return SD_INVALID_PARAMETER;
	}
	if(nblks>1){  //����
		if(nblks*blksize>SD_MAX_DATA_LENGTH){
			return SD_INVALID_PARAMETER;//�ж��Ƿ񳬹������ճ���
		}
		SDIO_DataConfig(SD_DATATIMEOUT,nblks*blksize,power,1);//nblks*blksize,512���С,����������
		SDIO_SendCMD(SD_CMD_READ_MULT_BLOCK,addr,1);	//����CMD18+��addr��ַ����ȡ����,����Ӧ
		status=CMDResponse1(SD_CMD_READ_MULT_BLOCK);//�ȴ�R1��Ӧ
		if(status!=SD_OK)return status;   	//��Ӧ����

		if(DeviceMode==SD_POLLING_MODE){
			while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|
							SDIO_STA_RXOVERR|SDIO_STA_DATAEND|SDIO_STA_STBITERR)))//������/CRC/��ʱ/���(��־)/��ʼλ����
			{
				if(SDIO_STA&SDIO_STA_RXFIFOHF){//����������,��ʾ���ٴ���8����
					for(count=0;count<8;count++)//ѭ����ȡ����
					{
						*(tempbuff+count)=SDIO_FIFO;
					}
					tempbuff+=8;
				}
			}
			if(SDIO_STA&SDIO_STA_DTIMEOUT){   //���ݳ�ʱ����
				SDIO_ICR |= SDIO_ICR_DTIMEOUTC;     //������־
				return SD_DATA_TIMEOUT;
			}
			else if(SDIO_STA&SDIO_STA_DCRCFAIL){  //���ݿ�CRC����
				SDIO_ICR |= SDIO_ICR_DCRCFAILC;     //������־
				return SD_DATA_CRC_FAIL;
			}
			else if(SDIO_STA&SDIO_STA_RXOVERR){   //����fifo�������
				SDIO_ICR |= SDIO_ICR_RXOVERRC;    //������־
				return SD_RX_OVERRUN;
			}
			else if(SDIO_STA&SDIO_STA_STBITERR){ //������ʼλ����
				SDIO_ICR |= SDIO_ICR_STBITERRC;     //������־
				return SD_START_BIT_ERR;
			}
			//----����������ʣ�������
			while(SDIO_STA&SDIO_STA_RXDAVL){
				*tempbuff=SDIO_FIFO;
				tempbuff++;
			}
			if(SDIO_STA&SDIO_STA_DATAEND){		//���ս���
				if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
						(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
						(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
				{
					SDIO_SendCMD(SD_CMD_STOP_TRANSMISSION,0,1);//����CMD12+��������
					status=CMDResponse1(SD_CMD_STOP_TRANSMISSION);//�ȴ�R1��Ӧ
					if(status!=SD_OK)return status;
				}
			}
			SDIO_ICR=SDIO_STATIC_FLAGS;	 		//������б��
		}
		else if(DeviceMode==SD_DMA_MODE)
		{
			TransferError=SD_OK;
			StopCondition=1;			//����,��Ҫ����ֹͣ����ָ��
			TransferEnd=0;				//�����������λ�����жϷ�����1
			SDIO_MASK=SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_RXOVERRIE|
				SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;

			SDIO_DCTRL |= SDIO_DCTRL_DMAEN;//SDIO DMAʹ��
			SD_DMA2_Config((dword*)buf,nblks*blksize,0);
			timeout=SDIO_DATATIMEOUT;
			while(((DMA2_ISR&BIT13)== 0)&&timeout)timeout--;//�ȴ��������
			if(timeout==0)return SD_DATA_TIMEOUT;//��ʱ
			while((TransferEnd==0)&&(TransferError==SD_OK));
			if(TransferError!=SD_OK)status=TransferError;
		}
	}
	return status;
}

/**********************************************
  SD��д1����
buf:���ݻ�����
addr:д��ַ
blksize:���С(byte)
����ֵ:����״̬
 **********************************************/
SD_ERROR SD_WriteBlock(byte *buf,dword addr,word blksize)
{
	SD_ERROR status = SD_OK;
	byte  power=0,cardstate=0;
	dword timeout=0,bytestransferred=0;
	dword cardstatus=0,count=0,restwords=0;
	dword tlen=blksize;//�ܳ���(�ֽ�)
	dword*tempbuff=(dword*)buf;
	if(0 == buf)return SD_INVALID_PARAMETER;

	SDIO_DCTRL=0;//���ݿ��ƼĴ�������(��DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){  //��������
		blksize=512;
		addr>>=9;
	}
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0)){
		power=convert_from_bytes_to_power_of_two(blksize);
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blksize,1); //����CMD16,�������ݳ���Ϊblksize,����Ӧ
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN); //�ȴ�R1��Ӧ
		if(status!=SD_OK)return status;
	}
	else return SD_INVALID_PARAMETER;

	SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);//����CMD13,��ѯ����״̬,����Ӧ
	status=CMDResponse1(SD_CMD_SEND_STATUS);//�ȴ�R1��Ӧ
	if(status!=SD_OK)return status;
	cardstatus=SDIO_RESP1;
	timeout=SD_DATATIMEOUT;
	while(((cardstatus&SD_STAT_READY_FOR_DATA)==0)&&(timeout>0))//���READY_FOR_DATAλ�Ƿ���λ
	{
		timeout--;
		SDIO_SendCMD(SD_CMD_SEND_STATUS,(dword)RCA<<16,1);//����CMD13,��ѯ����״̬,����Ӧ
		status=CMDResponse1(SD_CMD_SEND_STATUS);//�ȴ�R1��Ӧ
		if(status!=SD_OK)return status;
		cardstatus=SDIO_RESP1;
	}
	if(timeout==0)return SD_DATA_TIMEOUT;

	SDIO_SendCMD(SD_CMD_WRITE_SINGLE_BLOCK,addr,1);//����CMD24,д����ָ��,����Ӧ
	status=CMDResponse1(SD_CMD_WRITE_SINGLE_BLOCK);//�ȴ�R1��Ӧ
	if(status!=SD_OK)return status;
	StopCondition=0;//����д,����Ҫ����ֹͣ����ָ��
	SDIO_DataConfig(SD_DATATIMEOUT,blksize,power,0);//blksize, ����������

	if(DeviceMode == SD_POLLING_MODE){
		while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|SDIO_STA_TXUNDERR|
						SDIO_STA_STBITERR|SDIO_STA_DBCKEND)))//���ݿ鷢�ͳɹ�/����/CRC/��ʱ/��ʼλ����
		{
			if(SDIO_STA&SDIO_STA_TXFIFOHE)//���������,��ʾ���ٴ���8��32bit��
			{
				if((tlen-bytestransferred)<SD_HALFFIFOBYTES)//����32�ֽ���
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
		if(SDIO_STA&SDIO_STA_DTIMEOUT){   //���ݳ�ʱ����
			SDIO_ICR |= SDIO_ICR_DTIMEOUTC;     //������־
			return SD_DATA_TIMEOUT;
		}
		else if(SDIO_STA&SDIO_STA_DCRCFAIL){  //���ݿ�CRC����
			SDIO_ICR |= SDIO_ICR_DCRCFAILC;     //������־
			return SD_DATA_CRC_FAIL;
		}
		else if(SDIO_STA&SDIO_STA_TXUNDERR){   //����fifo�������
			SDIO_ICR |= SDIO_ICR_TXUNDERRC;    //������־
			return SD_TX_UNDERRUN;
		}
		else if(SDIO_STA&SDIO_STA_STBITERR){ //������ʼλ����
			SDIO_ICR |= SDIO_ICR_STBITERRC;     //������־
			return SD_START_BIT_ERR;
		}
		SDIO_ICR=SDIO_STATIC_FLAGS;      //������б��
	}
	else if(DeviceMode==SD_DMA_MODE)
	{
		TransferError=SD_OK;
		StopCondition=0;      //����д,����Ҫ����ֹͣ����ָ��
		TransferEnd=0;        //�����������λ�����жϷ�����1
		SDIO_MASK=SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_TXUNDERRIE|
			SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;
		SD_DMA2_Config((dword*)buf,blksize,1);//SDIO DMA����
		SDIO_DCTRL |= SDIO_DCTRL_DMAEN;//SDIO DMAʹ��
		timeout=SDIO_DATATIMEOUT;
		while(((DMA2_ISR&BIT13)== 0 )&&timeout)timeout--;//�ȴ��������
		if(timeout==0){
			Init_SDCard();//���³�ʼ��SD��,���Խ��д������������
			return SD_DATA_TIMEOUT;//��ʱ
		}
		timeout=SDIO_DATATIMEOUT;
		while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
		if(timeout==0)return SD_DATA_TIMEOUT;//��ʱ
		if(TransferError!=SD_OK)return TransferError;
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;//������б��
	status=GetSDCardMode(&cardstate);
	while((status==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		status=GetSDCardMode(&cardstate);
	}
	return status;
}


/**********************************************
//SD��д�����
//buf:���ݻ�����
//addr:д��ַ
//blksize:���С
//nblks:Ҫд��Ŀ���
//����ֵ:����״̬

 **********************************************/
SD_ERROR SD_WriteMultiBlocks(byte *buf,dword addr,word blksize,dword nblks)
{
	SD_ERROR status = SD_OK;
	byte  power = 0, cardstate = 0;
	dword timeout=0,bytestransferred=0;
	dword count = 0, restwords = 0;
	dword tlen=nblks*blksize;       //�ܳ���(�ֽ�)
	dword *tempbuff = (dword*)buf;

	if(0 == buf)return SD_INVALID_PARAMETER; //��������
	SDIO_DCTRL=0x0;              //���ݿ��ƼĴ�������(��DMA)
	SDIO_DataConfig(SD_DATATIMEOUT,0,0,0);
	SDIO_DCTRL &= ~SDIO_DCTRL_DTEN;
	if(SDIO_RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;

	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD){
		blksize=512;
		addr>>=9;
	}
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0)){
		power=convert_from_bytes_to_power_of_two(blksize);
		//����CMD16+�������ݳ���Ϊblksize,����Ӧ
		SDIO_SendCMD(SD_CMD_SET_BLOCKLEN,blksize,1);
		status=CMDResponse1(SD_CMD_SET_BLOCKLEN);//�ȴ�R1��Ӧ
		if(status!=SD_OK)return status;//��Ӧ����
	}
	else return SD_INVALID_PARAMETER;
	if(nblks>1){
		if(nblks*blksize>SD_MAX_DATA_LENGTH)return SD_INVALID_PARAMETER;
		if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
				(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
				(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
		{//�������
			SDIO_SendCMD(SD_CMD_APP_CMD,(dword)RCA<<16,1);
			status=CMDResponse1(SD_CMD_APP_CMD);
			if(status!=SD_OK)return status;
			//����CMD23,���ÿ�����,����Ӧ
			SDIO_SendCMD(SD_CMD_SET_BLOCK_COUNT,nblks,1);
			status=CMDResponse1(SD_CMD_SET_BLOCK_COUNT);
			if(status!=SD_OK)return status;
		}
		SDIO_SendCMD(SD_CMD_WRITE_MULT_BLOCK,addr,1);//����CMD25,���дָ��,����Ӧ
		status=CMDResponse1(SD_CMD_WRITE_MULT_BLOCK);
		if(status!=SD_OK)return status;
		SDIO_DataConfig(SD_DATATIMEOUT,nblks*blksize,power,0);//blksize, ����������

		if(DeviceMode==SD_POLLING_MODE){
			while(!(SDIO_STA&(SDIO_STA_DCRCFAIL|SDIO_STA_DTIMEOUT|SDIO_STA_TXUNDERR|
							SDIO_STA_STBITERR|SDIO_STA_DATAEND)))//����/CRC/���ݽ���/��ʱ/��ʼλ����
			{
				if(SDIO_STA&SDIO_STA_TXFIFOHE){//���������,��ʾ���ٴ���8��(32�ֽ�)
					if((tlen-bytestransferred)<SD_HALFFIFOBYTES)//����32�ֽ���
					{
						restwords=((tlen-bytestransferred)%4==0)?((tlen-bytestransferred)/4):((tlen-bytestransferred)/4+1);
						for(count=0;count<restwords;count++,tempbuff++,bytestransferred+=4)
						{
							SDIO_FIFO=*tempbuff;
						}
					}else{//���������,���Է�������8��(32�ֽ�)����
						for(count=0;count<SD_HALFFIFO;count++)
						{
							SDIO_FIFO=*(tempbuff+count);
						}
						tempbuff+=SD_HALFFIFO;
						bytestransferred+=SD_HALFFIFOBYTES;
					}
				}
			}
			if(SDIO_STA&SDIO_STA_DTIMEOUT){   //���ݳ�ʱ����
				SDIO_ICR |= SDIO_ICR_DTIMEOUTC;
				return SD_DATA_TIMEOUT;
			}
			else if(SDIO_STA&SDIO_STA_DCRCFAIL){  //���ݿ�CRC����
				SDIO_ICR |= SDIO_ICR_DCRCFAILC;
				return SD_DATA_CRC_FAIL;
			}
			else if(SDIO_STA&SDIO_STA_TXUNDERR){   //����fifo�������
				SDIO_ICR |= SDIO_ICR_TXUNDERRC;
				return SD_TX_UNDERRUN;
			}
			else if(SDIO_STA&SDIO_STA_STBITERR){ //������ʼλ����
				SDIO_ICR |= SDIO_ICR_STBITERRC;
				return SD_START_BIT_ERR;
			}

			if(SDIO_STA&SDIO_STA_DATAEND){//���ͽ���
				if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||
						(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||
						(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
				{
					SDIO_SendCMD(SD_CMD_STOP_TRANSMISSION,0,1);//����CMD12+��������
					status=CMDResponse1(SD_CMD_STOP_TRANSMISSION);//�ȴ�R1��Ӧ
					if(status!=SD_OK)return status;
				}
			}
			SDIO_ICR=SDIO_STATIC_FLAGS;      //������б��
		}
		else if(DeviceMode==SD_DMA_MODE)
		{
			TransferError=SD_OK;
			StopCondition=1;//���д,��Ҫ����ֹͣ����ָ��
			TransferEnd=0;//�����������λ�����жϷ�����1
			SDIO_MASK=SDIO_MASK_DCRCFAILIE|SDIO_MASK_DTIMEOUTIE|SDIO_MASK_TXUNDERRIE|
				SDIO_MASK_DATAENDIE|SDIO_MASK_STBITERRIE;
			SD_DMA2_Config((dword*)buf,nblks*blksize,1);//SDIO DMA����
			SDIO_DCTRL |= SDIO_DCTRL_DMAEN;//SDIO DMAʹ��
			timeout=SDIO_DATATIMEOUT;
			while(((DMA2_ISR&BIT13)== 0 )&&timeout)timeout--;//�ȴ��������
			if(timeout==0){//��ʱ
				Init_SDCard();//���³�ʼ��SD��,���Խ��д������������
				return SD_DATA_TIMEOUT;//��ʱ
			}
			timeout=SDIO_DATATIMEOUT;
			while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
			if(timeout==0)return SD_DATA_TIMEOUT;//��ʱ
			if(TransferError!=SD_OK)return TransferError;
		}
	}
	SDIO_ICR=SDIO_STATIC_FLAGS;//������б��
	status=GetSDCardMode(&cardstate);
	while((status==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		status=GetSDCardMode(&cardstate);
	}
	return status;
}



/**********************************************
  SDIO�жϷ�����
 **********************************************/
//void SDIO_IRQHandler(void)
//{
// 	SD_ProcessIRQSrc();//��������SDIO����ж�
//}

/**********************************************
  SDIO�жϴ�����
  ����SDIO��������еĸ����ж�����
 ***********************************************/
SD_ERROR SD_ProcessIRQSrc(void)
{
	if(SDIO_STA&SDIO_STA_DATAEND){//��������ж�
		if(StopCondition==1){
			SDIO_SendCMD(SD_CMD_STOP_TRANSMISSION,0,1);//����CMD12,��������
			TransferError=CMDResponse1(SD_CMD_STOP_TRANSMISSION);
		}else TransferError = SD_OK;

		SDIO_ICR |= SDIO_ICR_DATAENDC;//�������жϱ��
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//�ر�����ж�
		TransferEnd = 1;
		return(TransferError);
	}
	if(SDIO_STA&SDIO_STA_DCRCFAIL){//����CRC����
		SDIO_ICR |= SDIO_ICR_DCRCFAILC;//����жϱ��
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//�ر�����ж�
		TransferError = SD_DATA_CRC_FAIL;
		return(SD_DATA_CRC_FAIL);
	}
	if(SDIO_STA&SDIO_STA_DTIMEOUT){//���ݳ�ʱ����
		SDIO_ICR|=SDIO_ICR_DTIMEOUTC;//����жϱ��
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//�ر�����ж�
		TransferError = SD_DATA_TIMEOUT;
		return(SD_DATA_TIMEOUT);
	}
	if(SDIO_STA&SDIO_STA_RXOVERR){//FIFO�������
		SDIO_ICR|=SDIO_ICR_RXOVERRC;//����жϱ��
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//�ر�����ж�
		TransferError = SD_RX_OVERRUN;
		return(SD_RX_OVERRUN);
	}
	if(SDIO_STA&SDIO_STA_TXUNDERR){//FIFO�������
		SDIO_ICR|=SDIO_ICR_TXUNDERRC;//����жϱ��
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//�ر�����ж�
		TransferError = SD_TX_UNDERRUN;
		return(SD_TX_UNDERRUN);
	}
	if(SDIO_STA&SDIO_STA_STBITERR){//��ʼλ����
		SDIO_ICR|=SDIO_ICR_STBITERRC;//����жϱ��
		SDIO_MASK&=~(BIT1|BIT3|BIT4|BIT5|BIT8|BIT9|BIT14|BIT15);//�ر�����ж�
		TransferError = SD_START_BIT_ERR;
		return(SD_START_BIT_ERR);
	}
	return(SD_OK);
}


/**********************************************
//��SD��
//buf:�����ݻ�����
//sector:������ַ
//cnt:��������
//����ֵ:����״̬;0,����;����,�������;
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
			status=SD_ReadSingleBlock(SDIO_DATA_BUFFER,sector+512*n,512);//����sector�Ķ�����
			memcpy(buf,SDIO_DATA_BUFFER,512);
			buf+=512;
		}
	}else
	{
		if(cnt==1)status=SD_ReadSingleBlock(buf,sector,512);    	//����sector�Ķ�����
		else status=SD_ReadMultiBlocks(buf,sector,512,cnt);//���sector
	}
	return status;
}

/**********************************************
//дSD��
//buf:д���ݻ�����
//sector:������ַ
//cnt:��������
//����ֵ:����״̬;0,����;����,�������;
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
			sta=SD_WriteBlock(SDIO_DATA_BUFFER,sector+512*n,512);//����sector��д����
			buf+=512;
		}
	}else
	{
		if(cnt==1)sta=SD_WriteBlock(buf,sector,512);    	//����sector��д����
		else sta=SD_WriteMultiBlocks(buf,sector,512,cnt);	//���sector
	}
	return sta;
}

