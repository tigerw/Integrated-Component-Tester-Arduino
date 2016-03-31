#pragma once
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

class Display
{
public:
  static Adafruit_SSD1306 & GetInstance()
  {
    static Adafruit_SSD1306 Instance(/* D/C */ 6, /* Reset */ 10, /* Slave-Select */ 7);
    return Instance;
  }

  static void begin()
  {
    GetInstance().begin(SSD1306_SWITCHCAPVCC);
  }

  static void ClearNonHeaderArea()
  {   
    Display::GetInstance().fillRect(0, 16, SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT - 16, 0);
  }
  
  static void ClearHeaderArea()
  {   
    Display::GetInstance().fillRect(0, 0, SSD1306_LCDWIDTH, 8, 0);
  }

  Display(Display const &) = delete;
  void operator=(Display const &) = delete;
};

