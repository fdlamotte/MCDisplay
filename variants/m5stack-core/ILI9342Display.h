#pragma once

#include <helpers/ui/LGFXDisplay.h>

#define LGFX_AUTODETECT 

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <LGFX_AUTODETECT.hpp> 

class ILI9342Display : public LGFXDisplay {
  LGFX disp;

public:
  ILI9342Display() : LGFXDisplay(320, 240) { display=&disp; }
};
