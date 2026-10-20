#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
//C:\workspace\platformio_projects\HUB75_TEST2_ESP32> pio run -e freenove_esp32_wrover -t upload ; pio device monitor -p COM10 -b 115200
// HUB75E pinout
// 32 R1 | G1 23
// 33 B1 | GND
// 25 R2 | G2 22
// 26 B2 | E
// 27  A | B 21
// 14 C | D 19
// 13 CLK| LAT 18
// 4 OE | GND
#define R1 32
#define G1 23
#define BL1 33
#define R2 25
#define G2 22
#define BL2 26
#define CH_A 27
#define CH_B 21
#define CH_C 14
#define CH_D 19
#define CH_E -1
#define CLK 13
#define LAT 18
#define OE 4

MatrixPanel_I2S_DMA *dma_display = nullptr;

void setup() {
  Serial.begin(115200);
  Serial.println("Full-screen fill test start");

  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_width = 64;
  mxconfig.mx_height = 32;
  mxconfig.chain_length = 1;
  mxconfig.gpio.r1 = R1;
  mxconfig.gpio.g1 = G1;
  mxconfig.gpio.b1 = BL1;
  mxconfig.gpio.r2 = R2;
  mxconfig.gpio.g2 = G2;
  mxconfig.gpio.b2 = BL2;
  mxconfig.gpio.a = CH_A;
  mxconfig.gpio.b = CH_B;
  mxconfig.gpio.c = CH_C;
  mxconfig.gpio.d = CH_D;
  mxconfig.gpio.e = CH_E;
  mxconfig.gpio.clk = CLK;
  mxconfig.gpio.lat = LAT;
  mxconfig.gpio.oe = OE;
  mxconfig.driver = HUB75_I2S_CFG::FM6124;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  if (!dma_display->begin()) {
    Serial.println("I2S memory allocation failed");
    while (1) {}
  }

  dma_display->setBrightness8(3);

  
}

void loop() {
  Serial.println("Fill screen: RED");
  dma_display->fillScreenRGB888(255, 0, 0);
  delay(1000);

  Serial.println("Fill screen: GREEN");
  dma_display->fillScreenRGB888(0, 255, 0);
  delay(1000);

  Serial.println("Fill screen: BLUE");
  dma_display->fillScreenRGB888(0, 0, 255);
  delay(1000);

  Serial.println("Fill screen: WHITE");
  dma_display->fillScreenRGB888(255, 255, 255);
  delay(1000);

  Serial.println("Fill screen: BLACK");
  dma_display->fillScreenRGB888(0, 0, 0);
}