#include <Adafruit_MLX90640.h>
#include <SDHCI.h>
#include <Camera.h>
#include <SPI.h>
#include <stdio.h>

// #include "Adafruit_Arcada.h"

// Adafruit_Arcada arcada;


Adafruit_MLX90640 mlx;

#define BAUDRATE                (115200)

//low range of the sensor (this will be blue on the screen)
#define MINTEMP 20

//high range of the sensor (this will be red on the screen)
#define MAXTEMP 35

// /* Bitmap definition of RGB565 */
// #define IMG_WIDTH         (320)
// #define IMG_HEIGHT        (240)
#define IMG_WIDTH         (32)
#define IMG_HEIGHT        (24)
#define BITS_PER_PIXEL    (16)
#define BI_BITFIELD       (3)
#define IMG_SIZE (IMG_WIDTH*IMG_HEIGHT*(BITS_PER_PIXEL/8))
#define HEADER_SIZE       (54)
#define INFO_HEADER_SIZE  (40)
#define MASK_SIZE         (12)
float frame[IMG_WIDTH*IMG_HEIGHT]; // buffer for full frame of temperatures
//the colors we will be using
static int gCounter = 0;
const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

uint16_t displayPixelWidth, displayPixelHeight;
//SpiSDClass SD(SPI5);
SDClass SD;

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

void writeImageToSDRaw(uint16_t *img)
{
  uint32_t start_time = millis();

//  if (img.isAvailable()) {
//     img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    
    char filename[16] = {0};
    sprintf(filename, "PICT%03d.BMP", gCounter);
    if (SD.exists(filename)) {
//      Serial.println("remove " + String(filename));
      SD.remove(filename);
    }
    myFile = SD.open(filename,FILE_WRITE);
    
    /* write bitmap header */
    myFile.write(BMP_HEADER, (HEADER_SIZE+MASK_SIZE)*sizeof(uint8_t));
    myFile.write((uint8_t *)img, IMG_WIDTH*IMG_HEIGHT*2);
    myFile.close();
    ++gCounter;
//  }
  uint32_t duration = millis() - start_time;
  Serial.println("time (ms) = " + String(duration));
}
void setup() {
  // if (!arcada.arcadaBegin()) {
  //   Serial.print("Failed to begin");
  //   while (1);
  // }
  // arcada.displayBegin();
  // // Turn on backlight
  // arcada.setBacklight(255);
  
  Serial.begin(BAUDRATE);
  //while (!Serial);
  
  // arcada.display->fillScreen(ARCADA_BLACK);
  // displayPixelWidth = arcada.display->width() / 32;
  // displayPixelHeight = arcada.display->width() / 32; //Keep pixels square 

  delay(100);

  Serial.println("Adafruit MLX90640 Camera");
  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    // arcada.haltBox("MLX90640 not found!");
    Serial.println("MLX90640 not found!");
    exit(0);
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);
  
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_8_HZ);
  Wire.setClock(1000000); // max 1 MHz
  make_bmp_header();
}


void loop() {
  uint32_t timestamp = millis();

  uint16_t img [IMG_WIDTH*IMG_HEIGHT];

  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }

  int colorTemp;
  for (uint8_t h=0; h<IMG_HEIGHT; h++) {
    for (uint8_t w=0; w<IMG_WIDTH; w++) {
      float t = frame[h*IMG_WIDTH + w];
      // Serial.print(t, 1); Serial.print(", ");

      t = min(t, MAXTEMP);
      t = max(t, MINTEMP); 
           
      uint8_t colorIndex = map(t, MINTEMP, MAXTEMP, 0, 255);
      
      colorIndex = constrain(colorIndex, 0, 255);
      //draw the pixels!
      // arcada.display->fillRect(displayPixelWidth * w, displayPixelHeight * h,
      //                          displayPixelHeight, displayPixelWidth, 
      //                          camColors[colorIndex]);
      char pixel[16] = {0};
      sprintf(pixel, "%04x ", (uint16_t)camColors[colorIndex]);
      Serial.print(pixel);
      img [h*IMG_WIDTH+w] = (uint16_t)camColors[colorIndex];
    }
    Serial.println();
  }
  digitalWrite(LED0, HIGH);
  writeImageToSDRaw(img);
  digitalWrite(LED0, LOW);
  Serial.print((millis()-timestamp) / 2); Serial.println(" ms per frame (2 frames per display)");
  Serial.print(2000.0 / (millis()-timestamp)); Serial.println(" FPS (2 frames per display)");
  
  if (gCounter > 100)
  {
    exit(0);
  }
}
