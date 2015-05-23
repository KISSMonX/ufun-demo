
#ifndef _SDIO_H_
#define _SDIO_H_




//---------------SDIO_POWER------------//

#define SDIO_POWER_PWRCTRL_OFF      0
#define SDIO_POWER_PWRCTRL_ON       3


//---------------SDIO_CLKCR------------//

#define SDIO_CLKCR_HWFC_EN          BIT14   //使能硬件流控制
#define SDIO_CLKCR_NEGEDGE          BIT13   //SDIO_CK is Falling edge

#define SDIO_CLKCR_WIDBUS_ONEBITS   0
#define SDIO_CLKCR_WIDBUS_4WIDEBITS BIT11
#define SDIO_CLKCR_WIDBUS_8WIDEBITS BIT12

#define SDIO_CLKCR_BYPASS           BIT10 //SDIOCLK=SDIO_CK
#define SDIO_CLKCR_PWRSAV           BIT9  //省电配置:仅在有总线活动时才输出CK

#define SDIO_CLKCR_CLKEN            BIT8

#define SDIO_CLKCR_CLKDIV(n)        n     //SDIO_CK=SDIOCLK/(DIV+2)

//---------------SDIO_CMD------------//
#define SDIO_CMD_ATACMD           BIT14 //CPSM transfers CMD61
#define SDIO_CMD_nIEN             BIT13 //0->interrupt in CE-ATA device enable
#define SDIO_CMD_ENCMDcompl       BIT12 //the command completion signal is enabled
#define SDIO_CMD_SDIOSuspend      BIT11 //the command to be sent is a suspend command
#define SDIO_CMD_CPSMEN           BIT10 //the CPSM is enabled
#define SDIO_CMD_WAITPEND         BIT9  //CPSM Waits for ends of data transfer
#define SDIO_CMD_WAITINT          BIT8  //CPSM waits for interrupt request
#define SDIO_CMD_WAITRESP(n)      (n<<6)// Wait for response bits
                                        //00: No response, expect CMDSENT flag
                                        //01: Short response, expect CMDREND or CCRCFAIL flag
                                        //10: No response, expect CMDSENT flag
                                        //11: Long response, expect CMDREND or CCRCFAIL flag
#define SDIO_CMD_CMDINDEX(index)  index //The command index is sent to the card as part of a command message

//--------------------SDIO_DCTRL--------------------//
#define SDIO_DCTRL_SDIOEN         BIT11 //the DPSM performs an SD I/O-card-specific operation
#define SDIO_DCTRL_RWMOD          BIT10 //Read Wait control using SDIO_CK
#define SDIO_DCTRL_RWSTOP         BIT9  //Enable for read wait stop if RWSTART bit is set
#define SDIO_DCTRL_RWSTART        BIT8  //Read wait start
#define SDIO_DCTRL_DBLOCKSIZE(size) (size<<4) //Data block size 2^size
#define SDIO_DCTRL_DMAEN          BIT3  // DMA enable
#define SDIO_DCTRL_DTMODE         BIT2  // Stream data transfer
#define SDIO_DCTRL_DTDIR          BIT1  //Data transfer direction : From card to controller
#define SDIO_DCTRL_DTEN           BIT0  //Data transfer enabled bit

//------------------SDIO_STA------------------------//
#define SDIO_STA_CEATAEND         BIT23 //CE-ATA command completion signal received for CMD61
#define SDIO_STA_SDIOIT           BIT22 //SDIO interrupt received
#define SDIO_STA_RXDAVL           BIT21 //Data available in receive FIFO
#define SDIO_STA_TXDAVL           BIT20 //Data available in transmit FIFO
#define SDIO_STA_RXFIFOE          BIT19 //Receive FIFO empty
#define SDIO_STA_TXFIFOE          BIT18 //Transmit FIFO empty
#define SDIO_STA_RXFIFOF          BIT17 //Receive FIFO full
#define SDIO_STA_TXFIFOF          BIT16 //Transmit FIFO full
#define SDIO_STA_RXFIFOHF         BIT15 //Receive FIFO half full: there are at least 8 words in the FIFO
#define SDIO_STA_TXFIFOHE         BIT14 //Transmit FIFO half empty: at least 8 words can be written into the FIFO
#define SDIO_STA_RXACT            BIT13 //Data receive in progress
#define SDIO_STA_TXACT            BIT12 //Data transmit in progress
#define SDIO_STA_CMDACT           BIT11 //Command transfer in progress
#define SDIO_STA_DBCKEND          BIT10 //Data block sent/received (CRC check passed)
#define SDIO_STA_STBITERR         BIT9  //Start bit not detected on all data signals in wide bus mode
#define SDIO_STA_DATAEND          BIT8  //Data end (data counter, SDIDCOUNT, is zero)
#define SDIO_STA_CMDSENT          BIT7  //Command sent (no response required)
#define SDIO_STA_CMDREND          BIT6  //Command response received (CRC check passed)
#define SDIO_STA_RXOVERR          BIT5  //Received FIFO overrun error
#define SDIO_STA_TXUNDERR         BIT4  //Transmit FIFO underrun error
#define SDIO_STA_DTIMEOUT         BIT3  //Data timeout
#define SDIO_STA_CTIMEOUT         BIT2  //Command response timeout
#define SDIO_STA_DCRCFAIL         BIT1  //Data block sent/received (CRC check failed)
#define SDIO_STA_CCRCFAIL         BIT0  //Command response received (CRC check failed)



//--------------------SDIO_ICR--------------------//
#define SDIO_ICR_CEATAENDC        BIT23 //CEATAEND flag clear bit
#define SDIO_ICR_SDIOITC          BIT22 //SDIOIT flag clear bit
#define SDIO_ICR_DBCKENDC         BIT10 //DBCKEND cleared
#define SDIO_ICR_STBITERRC        BIT9  //STBITERR cleared
#define SDIO_ICR_DATAENDC         BIT8  //DATAEND cleared
#define SDIO_ICR_CMDSENTC         BIT7  //CMDSENT cleared
#define SDIO_ICR_CMDRENDC         BIT6  //CMDREND cleared
#define SDIO_ICR_RXOVERRC         BIT5  //RXOVERR cleared
#define SDIO_ICR_TXUNDERRC        BIT4  //TXUNDERR cleared
#define SDIO_ICR_DTIMEOUTC        BIT3  //DTIMEOUT cleared
#define SDIO_ICR_CTIMEOUTC        BIT2  //CTIMEOUT cleared
#define SDIO_ICR_DCRCFAILC        BIT1  //DCRCFAIL cleared
#define SDIO_ICR_CCRCFAILC        BIT0  //CCRCFAIL cleared

//---------------------SDIO_MASK--------------------//
#define SDIO_MASK_CEATAENDIE        BIT23 //CEATAEND
#define SDIO_MASK_SDIOITIE          BIT22 //SDIOIT 
#define SDIO_MASK_RXDVALIE          BIT21 //接收FIFO中的数据有效产生中断
#define SDIO_MASK_TXDVALIE          BIT20 //发送FIFO中的数据有效产生中断
#define SDIO_MASK_RXFIFOEIE         BIT19 //接收FIFO空产生中断
#define SDIO_MASK_TXFIFOEIE         BIT18 //发送FIFO空产生中断
#define SDIO_MASK_RXFIFOFIE         BIT17 //接收FIFO满产生中断
#define SDIO_MASK_TXFIFOFIE         BIT16 //发送FIFO满产生中断
#define SDIO_MASK_RXFIFOHFIE        BIT15 //接收FIFO半满产生中断
#define SDIO_MASK_TXFIFOHE          BIT14 //发送FIFO半空产生中断
#define SDIO_MASK_RXACTIE           BIT13 //正在接收数据产生中断
#define SDIO_MASK_TXACTIE           BIT12 //正在发送数据产生中断
#define SDIO_MASK_CMDACTIE          BIT11 //正在传输命令产生中断
#define SDIO_MASK_DBCKENDIE         BIT10 //数据块传输结束产生中断
#define SDIO_MASK_STBITERRIE        BIT9  //起始位错误产生中断
#define SDIO_MASK_DATAENDIE         BIT8  //数据传输结束产生中断
#define SDIO_MASK_CMDSENTIE         BIT7  //命令已发送产生中断
#define SDIO_MASK_CMDRENDIE         BIT6  //接收到响应产生中断 
#define SDIO_MASK_RXOVERRIE         BIT5  //接收FIFO上溢错误产生中断 
#define SDIO_MASK_TXUNDERRIE        BIT4  //发送FIFO下溢错误产生中断
#define SDIO_MASK_DTIMEOUTIE        BIT3  //DTIMEOUT 
#define SDIO_MASK_CTIMEOUTIE        BIT2  //CTIMEOUT 
#define SDIO_MASK_DCRCFAILIE        BIT1  //数据块CRC检测失败产生中断 
#define SDIO_MASK_CCRCFAILIE        BIT0  //命令CRC检测失败产生中断 


//------------------------SDIO --------------------------//
#define SDIO_DATA_OUT   (GPIOC_CRH &= 0xffff0000,GPIOC_CRH |= 0x0000bbbb)
#define SDIO_DATA_IN    (GPIOC_CRH &= 0xffff0000,GPIOC_CRH |= 0x00004444)
#define SDIO_CK_OUT     (GPIOC_CRH &= 0xfff0ffff,GPIOC_CRH |= 0x000b0000)

#define SDIO_CMD_OUT    (GPIOD_CRL &= 0xfffff0ff,GPIOD_CRL |= 0xfffffbff)
#define SDIO_CMD_IN     (GPIOD_CRL &= 0xfffff0ff,GPIOD_CRL |= 0x00000400)






#endif


