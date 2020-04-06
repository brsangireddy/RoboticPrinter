#include "p3rp_globals.h"
#include "p3rp_master_control.h"
#include "p3rp_carriage_control.h"
#include "p3rp_printhead_control.h"

uint64_t chipid;
#define CMDBUF_SZ 20  
uint8_t cmdBuf[CMDBUF_SZ+1];//+1 for inserting null character
// timeout time
#define RX_TO 100UL

int readCmdBuf()
{
  int index = 0;                // index in receive bufffer
  unsigned long lastRxTime = 0; // time when last character was received

  for(int i=0; i<CMDBUF_SZ; i++)//Clear command buffer for fresh command to be received
  {
    cmdBuf[i] = 0;
  }
  cmdBuf[CMDBUF_SZ] = 0;
 
  while(1)
  {   
    if (Serial2.available())
    {
      lastRxTime = millis();      // set the last time that a character was received      
      if (index >= sizeof(cmdBuf))// if the buffer is full
      {
        Serial.println("Command received.");
        break;
      }
      else
      {
        cmdBuf[index++] = Serial2.read();
      }
    }
    
    if (lastRxTime != 0 && (millis() - lastRxTime) >= 1000)
    {
      Serial.print("Received "); Serial.print(index); Serial.println(" characters");
      break;
    }
  }
  //Serial2.flush();
  return(index);
}
void processCmd()
{
  
}
void setup() {
  Serial.begin(DBG_SER_BAUD); //Console port over USB
  Serial2.begin(CMD_SER_BAUD); //Sending commands to controllers and receiving replies

  ConfigurePortPins();
  ConfigureWiFi();
  ConfigureHttpServer();
  SPIFFS.begin(); 
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID: %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  Serial.print("Build Date & Time: ");Serial.print(__DATE__);Serial.print(",");Serial.println(__TIME__);
}
void loop()
{
  httpserver.handleClient();
#if 0  
  int rcvd_byte_cnt = readCmdBuf();
  if(rcvd_byte_cnt > 0)
  {
    Serial.println((char*)cmdBuf);
    cmdBuf[0] = '#';
    Serial2.write(cmdBuf,CMDBUF_SZ);//Send reply to sender on ESP32's Serial port2
    processCmd();    
  }
  else
  {
    Serial.println("No cmd received.");    
  }
#endif  
}
