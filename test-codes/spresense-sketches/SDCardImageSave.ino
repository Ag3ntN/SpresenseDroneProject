/*
 *  camera.ino - Simple camera example sketch
 *  Copyright 2018, 2022 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */

#include <SDHCI.h>
#include <stdio.h>
#include <Camera.h>
#include <SPI.h>
//#include <SPISD.h>


//#define BAUDRATE                (1000000)
#define BAUDRATE                (115200)

/* Bitmap definition of RGB565 */
#define IMG_WIDTH         (320)
#define IMG_HEIGHT        (240)
#define BITS_PER_PIXEL    (16)
#define BI_BITFIELD       (3)
#define IMG_SIZE (IMG_WIDTH*IMG_HEIGHT*(BITS_PER_PIXEL/8))
#define HEADER_SIZE       (54)
#define INFO_HEADER_SIZE  (40)
#define MASK_SIZE         (12)

const int lineSize = 3*32;
uint8_t encoded[lineSize * 4/3 + 3];

//SpiSDClass SD(SPI5);
SDClass SD;
static int gCounter = 0;
File myFile;

/* Bitmap Header parameters */
uint16_t bfType = 0x4D42; /* "BM" */
uint32_t bfSize = IMG_SIZE + HEADER_SIZE + MASK_SIZE; /* image size + 54 + 12*/
uint16_t bfReserved1 = 0;
uint16_t bfReserved2 = 0;
uint32_t bfOffBits = HEADER_SIZE + MASK_SIZE;
uint32_t biSize = INFO_HEADER_SIZE;
uint32_t biWidth = IMG_WIDTH;
uint32_t biHeight = IMG_HEIGHT;
uint16_t biPlanes = 1;
uint16_t biBitCount = BITS_PER_PIXEL;
uint32_t biCompression = BI_BITFIELD;
uint32_t biSizeImage = IMG_SIZE;
uint32_t biXPelsPerMeter = 4724; /* dummy */
uint32_t biYPelsPerMeter = 4724; /* dummy */
uint32_t biClrUsed = 0;
uint32_t biClrImportant = 0;
uint32_t biRmask = 0x0000f800; /* 16bit mask */
uint32_t biGmask = 0x000007e0; /* 16bit mask */
uint32_t biBmask = 0x0000001f; /* 16bit mask */

uint8_t WORD[2];
uint8_t* swap16(uint16_t word16) {
  WORD[0] = (uint8_t)(word16 & 0x00ff);
  WORD[1] = (uint8_t)((word16 & 0xff00) >> 8);
  return WORD;
}

uint8_t DWORD[4];
uint8_t* swap32(uint32_t dword32) {
  DWORD[0] = (uint8_t)(dword32 & 0x000000ff);
  DWORD[1] = (uint8_t)((dword32 & 0x0000ff00) >> 8);
  DWORD[2] = (uint8_t)((dword32 & 0x00ff0000) >> 16);
  DWORD[3] = (uint8_t)((dword32 & 0xff000000) >> 24);
  return DWORD;
}


uint8_t BMP_HEADER[HEADER_SIZE+MASK_SIZE];
void make_bmp_header() {
  uint8_t* word16;
  uint8_t* word32;
  int n = 0;
  word16 = swap16(bfType); /* "BM" */
  for (int i = 0; i < 2; ++i) {
    BMP_HEADER[n++] = word16[i];  
  }
  word32 = swap32(bfSize); /* File Size */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word16 = swap16(bfReserved1); /* Reserved */
  for (int i = 0; i < 2; ++i) {
    BMP_HEADER[n++] = word16[i];  
  }
  word16 = swap16(bfReserved2); /* Reserved */
  for (int i = 0; i < 2; ++i) {
    BMP_HEADER[n++] = word16[i];  
  }
  word32 = swap32(bfOffBits); /* Offset to image data */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biSize); /* Bitmap info structure size (40) */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }  
  word32 = swap32(biWidth); /* Image width */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }  
  word32 = swap32(biHeight); /* Image height */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }    
  word16 = swap16(biPlanes); /* Image plane (almost 1) */
  for (int i = 0; i < 2; ++i) {
    BMP_HEADER[n++] = word16[i];  
  }   
  word16 = swap16(biBitCount); /* Pixel per bits */
  for (int i = 0; i < 2; ++i) {
    BMP_HEADER[n++] = word16[i];  
  }   
  word32 = swap32(biCompression); /* Complession type */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biSizeImage); /* Image size */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biXPelsPerMeter); /* Resolution (dummy) */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biYPelsPerMeter); /* Resolution (dummy) */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biClrUsed); /* Color used */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biClrImportant); /* Important Color */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biRmask); /* Bitmask for red in case of 16bits color */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biGmask); /* Bitmask for green in case of 16bits color */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }
  word32 = swap32(biBmask); /* Bitmask for blue in case of 16bits color */
  for (int i = 0; i < 4; ++i) {
    BMP_HEADER[n++] = word32[i];  
  }

  if (n != HEADER_SIZE + MASK_SIZE)  {
    Serial.println("HEADER_SIZE ERROR");
    exit(1);
  }   
}


// void setup() {
//   // Serial.begin(115200);
//   // if (!SD.begin(SPI_FULL_SPEED)) {
//   //   Serial.println("SD.begin() failed");
//     return;
//   }



// base64 encoder
const uint8_t base64Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void base64Encode(uint8_t* encoded, uint8_t* data, int dataLength){
  while(dataLength > 0){
    if(dataLength >= 3){
      encoded[0] = base64Table[data[0] >> 2];
      encoded[1] = base64Table[((data[0]&0x3) << 4) | (data[1] >> 4)];
      encoded[2] = base64Table[((data[1]&0xF) << 2) | (data[2] >> 6)];
      encoded[3] = base64Table[data[2] & 0x3F]; 
      data+=3;
      dataLength -= 3;
    }else if(dataLength == 2){
      encoded[0] = base64Table[data[0] >> 2];
      encoded[1] = base64Table[((data[0]&0x3) << 4) | (data[1] >> 4)];
      encoded[2] = base64Table[(data[1]&0xF) << 2];
      encoded[3] = '=';
      dataLength = 0;
    }else{
      encoded[0] = base64Table[data[0] >> 2];
      encoded[1] = base64Table[(data[0]&0x3) << 4];
      encoded[2] = '=';
      encoded[3] = '=';
      dataLength = 0;
    }
    encoded += 4;
  }
  *encoded = '\0';
}

/**
 * Print error message
 */

void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
  {
      case CAM_ERR_NO_DEVICE:
    Serial.println("No Device");
    break;
      case CAM_ERR_ILLEGAL_DEVERR:
    Serial.println("Illegal device error");
    break;
      case CAM_ERR_ALREADY_INITIALIZED:
    Serial.println("Already initialized");
    break;
      case CAM_ERR_NOT_INITIALIZED:
    Serial.println("Not initialized");
    break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
    Serial.println("Still picture not initialized");
    break;
      case CAM_ERR_CANT_CREATE_THREAD:
    Serial.println("Failed to create thread");
    break;
      case CAM_ERR_INVALID_PARAM:
    Serial.println("Invalid parameter");
    break;
      case CAM_ERR_NO_MEMORY:
    Serial.println("No memory");
    break;
      case CAM_ERR_USR_INUSED:
    Serial.println("Buffer already in use");
    break;
      case CAM_ERR_NOT_PERMITTED:
    Serial.println("Operation not permitted");
    break;
      default:
    break;
  }
}

/**
 * Callback from Camera library when video frame is captured.
 */

void CamCB(CamImage img)
{
  /* Check the img instance is available or not. */
  if (img.isAvailable())
  {
      /* If you want RGB565 data, convert image data format to RGB565 */
      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);

      /* You can use image data directly by using getImgSize() and getImgBuff().
       * for displaying image to a display, etc. */

      /*
      Serial.print("Image data size = ");
      Serial.print(img.getImgSize(), DEC);
      Serial.print(" , ");
      Serial.print("buff addr = ");
      Serial.print((unsigned long)img.getImgBuff(), HEX);
      Serial.println("");
      */
  }
    else
  {
      Serial.println("Failed to get video stream image");
  }
}

void initCamera(){
  CamErr err;

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */
  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }
  
  // カメラストリームを受信したら CamCBを実行する
  /*
  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }
  */

  // ホワイトバランスの設定
  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_AUTO);
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }

  // 静止画フォーマットの設定
  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
    CAM_IMGSIZE_QVGA_H,
    CAM_IMGSIZE_QVGA_V,
 //   CAM_IMAGE_PIX_FMT_JPG);
    CAM_IMAGE_PIX_FMT_YUV422);
  if (err != CAM_ERR_SUCCESS)
  {
    printError(err);
  }
  
  // ISOの設定
  Serial.println("Set ISO Sensitivity");
  err = theCamera.setAutoISOSensitivity(true);
  if (err != CAM_ERR_SUCCESS)
  {
    printError(err);
  }
  /*
  // ISO 値を固定する場合は setAutoISOSensitivity(false) に 
  err = theCamera.setISOSensitivity(CAM_ISO_SENSITIVITY_1600);
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }
  */

  // 露出の設定
  Serial.println("Set Auto exposure");
  err = theCamera.setAutoExposure(true);
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }
 /*
  // 露出を固定する場合は setAutoExposure(false) に 
  int exposure = 2740; // 最大値は 2740(NO HDR)  317(HDR) な模様
  err = theCamera.setAbsoluteExposure(exposure);
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }
*/
  // HDRの設定
  Serial.println("Set HDR");
  err = theCamera.setHDR(CAM_HDR_MODE_ON);
  if (err != CAM_ERR_SUCCESS)
  {
      printError(err);
  }
  make_bmp_header();
  

 
}

void sendImageToSerial(CamImage img){
    int inputLen = img.getImgSize();
    uint8_t* p = img.getImgBuff();
    Serial.println("#Image");
    while(inputLen > 0)
    {
      int len = inputLen > lineSize ? lineSize : inputLen;
      inputLen = inputLen - len;
      base64Encode(encoded, p, len); 
      p += len;
      Serial.println((char*)encoded);
    }
    Serial.println("#End");
}

void setup()
{
  Serial.begin(BAUDRATE);
  while (!Serial);
  initCamera();
}

void writeImageToSD(CamImage img)
{
  uint32_t start_time = millis();

 if (img.isAvailable()) {
    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    
    char filename[16] = {0};
    sprintf(filename, "PICT%03d.BMP", gCounter);
    if (SD.exists(filename)) {
//      Serial.println("remove " + String(filename));
      SD.remove(filename);
    }
    myFile = SD.open(filename,FILE_WRITE);
    
    /* write bitmap header */
    myFile.write(BMP_HEADER, (HEADER_SIZE+MASK_SIZE)*sizeof(uint8_t));
    myFile.write(img.getImgBuff(), img.getImgSize());
    myFile.close();
    ++gCounter;
  }
  uint32_t duration = millis() - start_time;
  Serial.println("time (ms) = " + String(duration));
}

void loop()
{
//  delay(10);
  CamImage img = theCamera.takePicture();
  if (img.isAvailable())
  {
    int iso = theCamera.getISOSensitivity();
    int exposure = theCamera.getAbsoluteExposure();
    int hdr = theCamera.getHDR();
    Serial.print("ISO ");
    Serial.print(iso);
    Serial.print(",Exposure ");
    Serial.print(exposure);
    Serial.print(",HDR ");
    Serial.print(hdr);
    Serial.println();


    // 画像の転送
    digitalWrite(LED0, HIGH);
 //   sendImageToSerial(img);
  
    // 画像の書き込み
    writeImageToSD(img);
    digitalWrite(LED0, LOW);
  }
  if (gCounter > 100)
  {
    exit(0);
  }
}
