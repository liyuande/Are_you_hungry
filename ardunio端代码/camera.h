#ifndef __CAMERA_H
#define __CAMERA_H

#include "ov2640.h"
#include "camera_dcmi_config.h"

#include <Arduino.h>

#include <stdint.h>
#include <stm32f4xx_hal.h>

#include <SoftWire.h>

#define CAMERA_RST_PIN  42
#define CAMERA_PWDN_PIN 43
#define CAMERA_SCCB_SCL 44
#define CAMERA_SCCB_SDA 45

#define OV2640_PWDN_1 digitalWrite(CAMERA_PWDN_PIN,HIGH)
#define OV2640_PWDN_0 digitalWrite(CAMERA_PWDN_PIN,LOW)

#define OV2640_RST_1	digitalWrite(CAMERA_RST_PIN,HIGH)
#define OV2640_RST_0 	digitalWrite(CAMERA_RST_PIN,LOW)

#define JPEG_OUTPUT_MODE

#ifdef JPEG_OUTPUT_MODE //jpeg output
#define jpeg_buf_size 16*1024  			//定义JPEG数据缓存jpeg_buf的大小(*4字节)

#endif

class CAMERA_DCMI_INTERFACE{
public:
  CAMERA_DCMI_INTERFACE(){}
  void begin(uint8_t device_id);
  void setOutSize(uint16_t w, uint16_t h);

  void startCapture(uint32_t memaddr);
  void setColorBarMode(uint8_t colorbar);

  void set_reg_value(uint8_t reg,uint8_t data){
    SCCB_WR_Reg(reg,data);
  }
  uint8_t get_reg_value(uint8_t reg){
    return SCCB_RD_Reg(reg);
  }


private:
  void camera_init(uint8_t device_id);
  void setOutMode();

private:

  uint8_t m_device_id;

private://sccb interface

  SoftWire camera_SCCB;

  void SCCB_Init(uint8_t device_id){
    m_device_id = device_id;
   camera_SCCB = SoftWire(CAMERA_SCCB_SCL,CAMERA_SCCB_SDA);
    camera_SCCB.begin();

    Serial.print("sccb init,device id=");
    Serial.println(m_device_id,HEX);
  }

  void SCCB_WR_Reg(uint8_t reg,uint8_t data){
    camera_SCCB.beginTransmission(m_device_id);

	camera_SCCB.write(reg);
	camera_SCCB.write(data);
	camera_SCCB.endTransmission();
  }

  uint8_t SCCB_RD_Reg(uint8_t reg){
    uint8_t result= 0;

	camera_SCCB.beginTransmission(m_device_id);
	camera_SCCB.write(reg);
	camera_SCCB.endTransmission();

	uint8_t ret = camera_SCCB.requestFrom(m_device_id,1);

	if(ret > 0)
		result = camera_SCCB.read();

	return result;
  }

  uint8_t OV2640_OutSize_Set(uint16_t width,uint16_t height)
  {
  	uint16_t outh;
  	uint16_t outw;
  	uint8_t temp;
  	if(width%4)return 1;
  	if(height%4)return 2;
  	outw=width/4;
  	outh=height/4;
  	SCCB_WR_Reg(0XFF,0X00);
  	SCCB_WR_Reg(0XE0,0X04);
  	SCCB_WR_Reg(0X5A,outw&0XFF);		//ÉèÖÃOUTWµÄµÍ°ËÎ»
  	SCCB_WR_Reg(0X5B,outh&0XFF);		//ÉèÖÃOUTHµÄµÍ°ËÎ»
  	temp=(outw>>8)&0X03;
  	temp|=(outh>>6)&0X04;
  	SCCB_WR_Reg(0X5C,temp);				//ÉèÖÃOUTH/OUTWµÄ¸ßÎ»
  	SCCB_WR_Reg(0XE0,0X00);

  	return 0;
  }
};

#endif
