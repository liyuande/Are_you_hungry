/*
WiFiSPI example: esp8266连接AP，UDP传输数据

电路连接：
   1. On ESP8266 must be running (flashed) WiFiSPIESP application.
    
   2. Connect the Arduino to the following pins on the esp8266:

            ESP8266         |
    GPIO    NodeMCU   Name  |   Lingzhi Enhanced Board
   ===================================
     15       D8       SS   |   53
     13       D7      MOSI  |   51
     12       D6      MISO  |   50
     14       D5      SCK   |   52
*/

#include "WiFiSpi.h"
#include "WiFiSpiUdp.h"  

#include "cJSON.h"
#include "Common.h"
#include "EdpKit.h"

#include "camera.h"

#include <fsmc_lcd.h>

CAMERA_DCMI_INTERFACE camera;

unsigned int localPort = 80;

//OneNet平台相关参数
char API_KEY[]	=	"Xsu4BlA79RsYIhc=BsLaZWhUdx8=";//你的oneNet上的api key
char DEVICE_ID[]=	"577406884";					//你的设备ID

#define JPG_DATA	"ov2640_jpg"

#define EDPserver	"jjfaedp.hedevice.com"

#define	ssid		"test"//换成自己的wifi热点
#define	password	"20010719"

//char udpServerIP[] = "183.230.40.34";
char serverIP[] = "183.230.40.39";//for testing

int edpServerPort = 876;

WiFiSpiClient client;

uint8_t connect_server_flag = 0;


#ifdef JPEG_OUTPUT_MODE
uint32_t jpeg_buf[jpeg_buf_size];	//JPEG数据缓存buf
extern volatile uint8_t jpeg_data_ok;
extern volatile uint32_t jpeg_data_len;
#endif

void connectEDP();
void sendPacket(uint8_t *jpg_data, uint32_t len);//发送一帧的图像

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize the ESP module
  WiFiSpi.init();
// 检查是否连接了运行SPIWifi程序的8266模块
  if (WiFiSpi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  
	 int status = WL_IDLE_STATUS;
  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to wifi with SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFiSpi.begin(ssid, password);
  }
 
  
}

void loop()
{
	

 

  // 已经连接到wifi
  Serial.println("You're connected to the network");
  
  if (client.connect(EDPserver, edpServerPort)) {
    Serial.println("connected to server");
	  //开启camera,这里传输jpg图像
	camera.begin(OV2640_DEVICE_ID);
#ifdef JPEG_OUTPUT_MODE
	camera.setOutSize(240,320);
	camera.startCapture((uint32_t)&jpeg_buf);
#endif
  
 //请求连接EDP服务器
	connectEDP();
	
	uint8_t *p;
	
	//发送摄像头数据
	if(jpeg_data_ok==1)	//
	{
		p=(uint8_t*)jpeg_buf;

		Serial.println("send one frame data...");
		
		//去掉开头的无效数据，有时候前面一堆0，？？
		bool start_flag = false;
		int start_index = 0;
		for(int i=0; i<jpeg_data_len*4; i++)
		{
			if(p[i] == 0xff)
			{
				if(p[i+1] == 0xd8)
				{
					start_index = i;
					start_flag = true;
					break;
				}
			}
		}

		//valid frame
		if(start_flag)
			sendPacket(p+start_index, jpeg_data_len*4-start_index);

		jpeg_data_ok=2;	//
	}
	  
  }else{
	Serial.println("connect server failed");
 }
	client.stop();

	delay(5000);
}

void connectEDP()
{
/*利用sdk 中EdpKit.h 中PacketConnect1 方法封包连接协议*/
	EdpPacket* send_pkg = PacketConnect1(DEVICE_ID,API_KEY);
	  
	/*发送连接协议包数据*/
	//int ret = DoSend (sockfd, send_pkg->_data, send_pkg->_write_pos);
	  
	  int size = send_pkg->_write_pos;
	  
	  Serial.print("send size:");
	  Serial.println(size);
	  
	  client.write(send_pkg->_data, send_pkg->_write_pos);
	  
	/*使用完后必须删除send_pkg ， 否则会造成内存泄漏*/
	DeleteBuffer(&send_pkg);	
}

// send an packet to server 
void sendPacket(uint8_t *jpg_data, uint32_t len)
{

	  Serial.println("send jpg");
	  char text2[]="{\"ds_id\": \"ov2640_jpg\"}";	
	  cJSON *desc_json;
	  
	  desc_json=cJSON_Parse(text2);
	  
	  EdpPacket* send_pkg = PacketSavedataBin(NULL, desc_json, jpg_data, len, 0);
	  
	  
	  Serial.println("buff size:");
	  Serial.print("jpg_buff:");Serial.println(len);
	  Serial.print("packet_size:");Serial.println(send_pkg->_write_pos);
	  
	  uint8_t *send_buff = send_pkg->_data;
	  uint32_t send_len = send_pkg->_write_pos;
	  
	  int packetLen = 1024;
		
	  int t = 0;
	  for(t=0; t< send_len/packetLen; t++)
	  {
		  Serial.print("ofset:");Serial.println(t*packetLen);
		  Serial.print("lenth:");Serial.println(packetLen);
		 client.write(send_buff+t*packetLen, packetLen);
		 delay(100);  
	  }
	  
	  Serial.print("ofset:");Serial.println(t*packetLen);
	  Serial.print("lenth:");Serial.println(send_len-packetLen*t);
	  
	  client.write(send_buff+t*packetLen, send_len-packetLen*t);
	  
	  DeleteBuffer(&send_pkg);
	
	  Serial.println("send done");

}

