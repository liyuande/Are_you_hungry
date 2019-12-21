#include "camera.h"

uint8_t camera_mode = 0;

extern DMA_HandleTypeDef   DMADMCI_Handler;

#ifdef JPEG_OUTPUT_MODE

volatile uint32_t jpeg_data_len=0; 			//buf中的JPEG有效数据长度
volatile uint8_t jpeg_data_ok=0;				//JPEG数据采集完成标志

//处理JPEG数据
//当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void DCMI_Frame_Event()
{
	//if(camera_mode == 1)//只有在JPEG格式下,才需要做处理.
	{
		if(jpeg_data_ok==0)	//jpeg数据还未采集完?
		{
			__HAL_DMA_DISABLE(&DMADMCI_Handler);//关闭DMA
			while(DMA2_Stream1->CR&0X01);	//等待DMA2_Stream1可配置
			jpeg_data_len=jpeg_buf_size-__HAL_DMA_GET_COUNTER(&DMADMCI_Handler);//得到剩余数据长度
			jpeg_data_ok=1; 				//标记JPEG数据采集完按成,等待其他函数处理
		}
		if(jpeg_data_ok==2)	//上一次的jpeg数据已经被处理了
		{
			__HAL_DMA_SET_COUNTER(&DMADMCI_Handler,jpeg_buf_size);//传输长度为jpeg_buf_size*4字节
			__HAL_DMA_ENABLE(&DMADMCI_Handler); //打开DMA
			jpeg_data_ok=0;						//标记数据未采集
		}
	}
}

#endif

void CAMERA_DCMI_INTERFACE::begin(uint8_t device_id)
{

  camera_init(device_id);

    setOutMode();

}

void CAMERA_DCMI_INTERFACE::setOutSize(uint16_t w, uint16_t h)
{
  OV2640_OutSize_Set(w,h);
}

void CAMERA_DCMI_INTERFACE::setOutMode()
{

uint16_t i=0;

#ifdef JPEG_OUTPUT_MODE
//yuv422
  for(i=0;i<(sizeof(ov2640_yuv422_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_yuv422_reg_tbl[i][0],ov2640_yuv422_reg_tbl[i][1]);
	}
	//jpg
	for(i=0;i<(sizeof(ov2640_jpeg_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_jpeg_reg_tbl[i][0],ov2640_jpeg_reg_tbl[i][1]);
	}
#else
for(i=0;i<(sizeof(ov2640_rgb565_reg_tbl)/2);i++)
{
  SCCB_WR_Reg(ov2640_rgb565_reg_tbl[i][0],ov2640_rgb565_reg_tbl[i][1]);
}
#endif

}

void CAMERA_DCMI_INTERFACE::setColorBarMode(uint8_t colorbar)
{
  uint8_t reg;
	SCCB_WR_Reg(0XFF,0X01);
	reg=SCCB_RD_Reg(0X12);
	reg&=~(1<<1);
	if(colorbar)reg|=1<<1;
	SCCB_WR_Reg(0X12,reg);
}

void CAMERA_DCMI_INTERFACE::startCapture(uint32_t outmem)
{

  DCMI_Init();


#ifdef JPEG_OUTPUT_MODE
  DCMI_DMA_Init(outmem,jpeg_buf_size,DMA_MDATAALIGN_WORD,DMA_MINC_ENABLE);
#else
  DCMI_DMA_Init(outmem,1,DMA_MDATAALIGN_HALFWORD,DMA_MINC_DISABLE);
#endif
  DCMI_Start();
}

void CAMERA_DCMI_INTERFACE::camera_init(uint8_t device_id)
{
  uint16_t i=0;
	uint16_t reg;
	//ÉèÖÃIO

	//delay_us_init
	    //HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTickÆµÂÊÎªHCLK

  pinMode(CAMERA_PWDN_PIN,OUTPUT);
  pinMode(CAMERA_RST_PIN,OUTPUT);

	OV2640_PWDN_0;	//POWER ON
	HAL_Delay(10);
	OV2640_RST_0;	//¸´Î»OV2640
	HAL_Delay(10);
	OV2640_RST_1;	//½áÊø¸´Î»

  SCCB_Init(device_id);        		//³õÊ¼»¯SCCB µÄIO¿Ú

	SCCB_WR_Reg(OV2640_DSP_RA_DLMT, 0x01);	//²Ù×÷sensor¼Ä´æÆ÷
 	SCCB_WR_Reg(OV2640_SENSOR_COM7, 0x80);	//Èí¸´Î»OV2640
	delay(50);
	reg=SCCB_RD_Reg(OV2640_SENSOR_MIDH);	//¶ÁÈ¡³§¼ÒID ¸ß°ËÎ»
	reg<<=8;
	reg|=SCCB_RD_Reg(OV2640_SENSOR_MIDL);	//¶ÁÈ¡³§¼ÒID µÍ°ËÎ»
	if(reg!=OV2640_MID)
	{
    // Serial.println("ov2640 mid error");

		// printf("MID:%d\r\n",reg);
		// return 1;
	}
	reg=SCCB_RD_Reg(OV2640_SENSOR_PIDH);	//¶ÁÈ¡³§¼ÒID ¸ß°ËÎ»
	reg<<=8;
	reg|=SCCB_RD_Reg(OV2640_SENSOR_PIDL);	//¶ÁÈ¡³§¼ÒID µÍ°ËÎ»
	if(reg!=OV2640_PID)
	{
      // Serial.println("ov2640 pid error");
		// printf("HID:%d\r\n",reg);
		//return 2;
	}

 	//OV2640,uxga(1600*1200) 15fps
//	for(i=0;i<sizeof(ov2640_uxga_init_reg_tbl)/2;i++)
//	{
//	   	SCCB_WR_Reg(ov2640_uxga_init_reg_tbl[i][0],ov2640_uxga_init_reg_tbl[i][1]);
// 	}

	 	// OV2640,svga mode (800*600) 30fps
	for(i=0;i<sizeof(ov2640_svga_init_reg_tbl)/2;i++)
	{
	   	SCCB_WR_Reg(ov2640_svga_init_reg_tbl[i][0],ov2640_svga_init_reg_tbl[i][1]);
 	}

}
