
#ifndef _SD_CARD_H_
#define _SD_CARD_H_


//----------------CID register---------------------//
typedef struct{
	byte  MID;        //ManufacturerID
	word  OID;        //OEM/Application ID
	dword PNM1;       //Product Name part1
	byte  PNM2;       //Product Name part2
	byte  PRV;        //Product Revision
	dword PSN;        //Product Serial Number
	byte  Reserved1;  //Reserved1
	word  MDT;        //Manufacturing Date
	byte  CIDCRC;     //CID CRC
	byte  Reserved2;  //always 1

}SD_CID_Register;


//----------------CSD register---------------------//
typedef struct{
	byte  CSDStruct;            /*!< CSD structure */
	byte  SysSpecVersion;       /*!< System specification version */
	byte  Reserved1;            /*!< Reserved */
	byte  TAAC;                 /*!< Data read access-time 1 */
	byte  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
	byte  MaxBusClkFrec;        /*!< Max. bus clock frequency */
	word  CardComdClasses;      /*!< Card command classes */
	byte  RdBlockLen;           /*!< Max. read data block length */
	byte  PartBlockRead;        /*!< Partial blocks for read allowed */
	byte  WrBlockMisalign;      /*!< Write block misalignment */
	byte  RdBlockMisalign;      /*!< Read block misalignment */
	byte  DSRImpl;              /*!< DSR implemented */
	byte  Reserved2;            /*!< Reserved */
	dword DeviceSize;           /*!< Device Size */
	byte  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
	byte  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
	byte  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
	byte  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
	byte  DeviceSizeMul;        /*!< Device size multiplier */

	byte  EraseGrSize;          /*!< Erase group size */
	byte  EraseGrMul;           /*!< Erase group size multiplier */
	byte  WrProtectGrSize;      /*!< Write protect group size */
	byte  WrProtectGrEnable;    /*!< Write protect group enable */
	byte  ManDeflECC;           /*!< Manufacturer default ECC */
	byte  WrSpeedFact;          /*!< Write speed factor */
	byte  MaxWrBlockLen;        /*!< Max. write data block length */
	byte  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
	byte  Reserved3;            /*!< Reserded */
	byte  ContentProtectAppli;  /*!< Content protection application */
	byte  FileFormatGrouop;     /*!< File format group */
	byte  CopyFlag;             /*!< Copy flag (OTP) */
	byte  PermWrProtect;        /*!< Permanent write protection */
	byte  TempWrProtect;        /*!< Temporary write protection */
	byte  FileFormat;           /*!< File Format */
	byte  ECC;                  /*!< ECC code */
	byte  CSD_CRC;              /*!< CSD CRC */
	byte  Reserved4;            /*!< always 1*/
}SD_CSD_Register;

//-------------------SD Card infomation----------------//
typedef struct
{
	SD_CSD_Register SD_csd;
	SD_CID_Register SD_cid;
	long long CardCapacity; //SD卡容量,单位:字节,最大支持2^64字节大小的卡.
	dword CardBlockSize; 		//SD卡块大小
	word RCA;					      //卡相对地址
	byte CardType;				  //卡类型
} SD_CardInfo;




//---------------------Error status------------------------------//
typedef enum
{
	//特殊错误定义
	SD_CMD_CRC_FAIL           = (1), //ommand response received (but CRC check failed)
	SD_DATA_CRC_FAIL          = (2), //ata bock sent/received (CRC check Failed)
	SD_CMD_RSP_TIMEOUT        = (3), //ommand response timeout
	SD_DATA_TIMEOUT           = (4), //ata time out
	SD_TX_UNDERRUN            = (5), //ransmit FIFO under-run
	SD_RX_OVERRUN             = (6), //eceive FIFO over-run
	SD_START_BIT_ERR          = (7), //tart bit not detected on all data signals in widE bus mode
	SD_CMD_OUT_OF_RANGE       = (8), //MD's argument was out of range.
	SD_ADDR_MISALIGNED        = (9), //isaligned address
	SD_BLOCK_LEN_ERR          = (10),//Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length
	SD_ERASE_SEQ_ERR          = (11),//An error in the sequence of erase command occurs.
	SD_BAD_ERASE_PARAM        = (12),//An Invalid selection for erase groups
	SD_WRITE_PROT_VIOLATION   = (13),//Attempt to program a write protect block
	SD_LOCK_UNLOCK_FAILED     = (14),//Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card
	SD_COM_CRC_FAILED         = (15),//CRC check of the previous command failed
	SD_ILLEGAL_CMD            = (16),//Command is not legal for the card state
	SD_CARD_ECC_FAILED        = (17),//Card internal ECC was applied but failed to correct the data
	SD_CC_ERROR               = (18),//Internal card controller error
	SD_GENERAL_UNKNOWN_ERROR  = (19),//General or Unknown error
	SD_STREAM_READ_UNDERRUN   = (20),//The card could not sustain data transfer in stream read operation.
	SD_STREAM_WRITE_OVERRUN   = (21),//The card could not sustain data programming in stream mode
	SD_CID_CSD_OVERWRITE      = (22),//CID/CSD overwrite error
	SD_WP_ERASE_SKIP          = (23),//only partial address space was erased
	SD_CARD_ECC_DISABLED      = (24),//Command has been executed without using internal ECC
	SD_ERASE_RESET            = (25),//Erase sequence was cleared before executing because an out of erase sequence command was received
	SD_AKE_SEQ_ERROR          = (26),//Error in sequence of authentication.
	SD_INVALID_VOLTRANGE      = (27),
	SD_ADDR_OUT_OF_RANGE      = (28),
	SD_SWITCH_ERROR           = (29),
	SD_SDIO_DISABLED          = (30),
	SD_SDIO_FUNCTION_BUSY     = (31),
	SD_SDIO_FUNCTION_FAILED   = (32),
	SD_SDIO_UNKNOWN_FUNCTION  = (33),
	//标准错误定义
	SD_INTERNAL_ERROR,
	SD_NOT_CONFIGURED,
	SD_REQUEST_PENDING,
	SD_REQUEST_NOT_APPLICABLE,
	SD_INVALID_PARAMETER,
	SD_UNSUPPORTED_FEATURE,
	SD_UNSUPPORTED_HW,
	SD_Error,
	SD_OK = 0
}SD_ERROR;


//-------------------SD卡指令集-----------------------------------//
#define SD_CMD_GO_IDLE_STATE               (0)
#define SD_CMD_SEND_OP_COND                (1)
#define SD_CMD_ALL_SEND_CID                (2)
#define SD_CMD_SET_REL_ADDR                (3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define SD_CMD_SET_DSR                     (4)
#define SD_CMD_SDIO_SEN_OP_COND            (5)

#define SD_CMD_SEL_DESEL_CARD              (7)
#define SD_CMD_HS_SEND_EXT_CSD             (8)
#define SD_CMD_SEND_CSD                    (9)
#define SD_CMD_SEND_CID                    (10)
#define SD_CMD_READ_DAT_UNTIL_STOP         (11) /*!< SD Card doesn't support it */
#define SD_CMD_STOP_TRANSMISSION           (12)
#define SD_CMD_SEND_STATUS                 (13)
#define SD_CMD_HS_BUSTEST_READ             (14)
#define SD_CMD_GO_INACTIVE_STATE           (15)
#define SD_CMD_SET_BLOCKLEN                (16)
#define SD_CMD_READ_SINGLE_BLOCK           (17)
#define SD_CMD_READ_MULT_BLOCK             (18)
#define SD_CMD_HS_BUSTEST_WRITE            (19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP        (20)
#define SD_CMD_SET_BLOCK_COUNT             (23)
#define SD_CMD_WRITE_SINGLE_BLOCK          (24)
#define SD_CMD_WRITE_MULT_BLOCK            (25)
#define SD_CMD_PROG_CID                    (26)
#define SD_CMD_PROG_CSD                    (27)
#define SD_CMD_SET_WRITE_PROT              (28)
#define SD_CMD_CLR_WRITE_PROT              (29)
#define SD_CMD_SEND_WRITE_PROT             (30)
#define SD_CMD_SD_ERASE_GRP_START          (32) /*!< To set the address of the first write
						  block to be erased. (For SD card only) */
#define SD_CMD_SD_ERASE_GRP_END            (33) /*!< To set the address of the last write block of the
						  continuous range to be erased. (For SD card only) */
#define SD_CMD_ERASE_GRP_START             (35) /*!< To set the address of the first write block to be erased.
						  (For MMC card only spec 3.31) */
#define SD_CMD_ERASE_GRP_END               (36) /*!< To set the address of the last write block of the
						  continuous range to be erased. (For MMC card only spec 3.31) */
#define SD_CMD_ERASE                       (38)
#define SD_CMD_FAST_IO                     (39) /*!< SD Card doesn't support it */
#define SD_CMD_GO_IRQ_STATE                (40) /*!< SD Card doesn't support it */
#define SD_CMD_LOCK_UNLOCK                 (42)
#define SD_CMD_APP_CMD                     (55)
#define SD_CMD_GEN_CMD                     (56)
#define SD_CMD_NO_CMD                      (64)

/* @brief Following commands are SD Card Specific commands.
 *  SDIO_APP_CMD ：CMD55 should be sent before sending these commands.
 */
#define SD_CMD_APP_SD_SET_BUSWIDTH          (6)  /*!< For SD Card only */
#define SD_CMD_SD_APP_STAUS                 (13) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS (22) /*!< For SD Card only */
#define SD_CMD_SD_APP_OP_COND               (41) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT   (42) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_SCR              (51) /*!< For SD Card only */
#define SD_CMD_SDIO_RW_DIRECT               (52) /*!< For SD I/O Card only */
#define SD_CMD_SDIO_RW_EXTENDED             (53) /*!< For SD I/O Card only */
//CMD8指令
#define SDIO_SEND_IF_COND               ((dword)0x00000008)



//支持的SD卡定义
#define SDIO_STD_CAPACITY_SD_CARD_V1_1      (0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0      (0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD          (0x00000002)
#define SDIO_MULTIMEDIA_CARD                (0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD         (0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD     (0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD   (0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD         (0x00000007)

//SDIO相关参数定义
#define NULL 0
#define SDIO_STATIC_FLAGS               (0x000005FF)
#define SDIO_CMD0TIMEOUT                (0x00002710)
#define SDIO_DATATIMEOUT                (0x0002FFFF)
#define SDIO_FIFO_Address               (0x40018080)


//--------------------Card State For R1------------------//
#define SD_STAT_ADDR_OUT_OF_RANGE        BIT31  //命令地址超出卡范围
#define SD_STAT_ADDR_MISALIGNED          BIT30  //命令地址与物理地址不对齐
#define SD_STAT_BLOCK_LEN_ERR            BIT29
#define SD_STAT_ERASE_SEQ_ERR            BIT28
#define SD_STAT_BAD_ERASE_PARAM          BIT27
#define SD_STAT_WRITE_PROT_VIOLATION     BIT26
#define SD_STAT_CARD_LOCKED              BIT25
#define SD_STAT_LOCK_UNLOCK_FAILED       BIT24
#define SD_STAT_COM_CRC_FAILED           BIT23
#define SD_STAT_ILLEGAL_CMD              BIT22
#define SD_STAT_CARD_ECC_FAILED          BIT21
#define SD_STAT_CC_ERROR                 BIT20
#define SD_STAT_GENERAL_UNKNOWN_ERROR    BIT19
#define SD_STAT_STREAM_READ_UNDERRUN     BIT18
#define SD_STAT_STREAM_WRITE_OVERRUN     BIT17
#define SD_STAT_CID_CSD_OVERWRIETE       BIT16
#define SD_STAT_WP_ERASE_SKIP            BIT15
#define SD_STAT_CARD_ECC_DISABLED        BIT14
#define SD_STAT_ERASE_RESET              BIT13
#define SD_STAT_READY_FOR_DATA           BIT8
#define SD_STAT_SWITCH_ERROR             BIT7
#define SD_STAT_APP_CMD                  BIT5
#define SD_STAT_AKE_SEQ_ERROR            BIT3
#define SD_STAT_ERRORBITS                0xFDFFE008


//--------------------Card State For R6------------------//
#define SD_R6_GENERAL_UNKNOWN_ERROR     BIT13
#define SD_R6_ILLEGAL_CMD               BIT14
#define SD_R6_COM_CRC_FAILED            BIT15







//---------------------------------------------------------

#define SD_VOLTAGE_WINDOW_SD            (0x80100000)
#define SD_HIGH_CAPACITY                (0x40000000)
#define SD_STD_CAPACITY                 (0x00000000)
#define SD_CHECK_PATTERN                (0x000001AA)
#define SD_VOLTAGE_WINDOW_MMC           (0x80FF8000)

#define SD_MAX_VOLT_TRIAL               (0x0000FFFF)
#define SD_ALLZERO                      (0x00000000)


#define SD_WIDE_BUS_SUPPORT             (0x00040000)
#define SD_SINGLE_BUS_SUPPORT           (0x00010000)
#define SD_CARD_LOCKED                  (0x02000000)
#define SD_CARD_PROGRAMMING             (0x00000007)
#define SD_CARD_RECEIVING               (0x00000006)
#define SD_DATATIMEOUT                  (0x000FFFFF)
#define SD_0TO7BITS                     (0x000000FF)
#define SD_8TO15BITS                    (0x0000FF00)
#define SD_16TO23BITS                   (0x00FF0000)
#define SD_24TO31BITS                   (0xFF000000)
#define SD_MAX_DATA_LENGTH              (0x01FFFFFF)

#define SD_HALFFIFO                     (0x00000008)
#define SD_HALFFIFOBYTES                (0x00000020)

//Command Class Supported
#define SD_CCCC_LOCK_UNLOCK             (0x00000080)
#define SD_CCCC_WRITE_PROT              (0x00000040)
#define SD_CCCC_ERASE                   (0x00000020)


#endif



