#ifndef REGS_SCB_H
#define REGS_SCB_H

#include "register.h"

namespace Ion {
namespace Device {
namespace Regs {

// http://www.st.com/content/ccc/resource/technical/document/programming_manual/6c/3a/cb/e7/e4/ea/44/9b/DM00046982.pdf/files/DM00046982.pdf/jcr:content/translations/en.DM00046982.pdf page 221 (or 192 on the F7/H7 programming manual)
// https://libopencm3.org/docs/latest/stm32f3/html/group__cm__scb__registers.html
class SCB {
public:
  class SHPR1 : public Register32 {
  public:
    using Register32::Register32;
    REGS_FIELD(PRI6, uint8_t, 23, 16);
    REGS_FIELD(PRI5, uint8_t, 15, 8);
    REGS_FIELD(PRI4, uint8_t, 7, 0);
  };

  class SHPR2 : public Register32 {
  public:
    using Register32::Register32;
    REGS_FIELD(PRI11, uint8_t, 31, 24);
  };

  class SHPR3 : public Register32 {
  public:
    using Register32::Register32;
    REGS_FIELD(PRI15, uint8_t, 31, 24);
    REGS_FIELD(PRI14, uint8_t, 23, 16);
  };

  REGS_REGISTER_AT(SHPR1, 0xD18);
  REGS_REGISTER_AT(SHPR2, 0xD1C);
  REGS_REGISTER_AT(SHPR3, 0xD20);

private:
  constexpr uint32_t Base() const {
    return 0xE000E000;
  }
};

constexpr SCB SCB;

}
}
}

#endif
