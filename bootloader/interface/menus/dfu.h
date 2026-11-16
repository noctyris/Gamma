#ifndef _BOOTLOADER_INTERFACE_MENUS_DFU_H_
#define _BOOTLOADER_INTERFACE_MENUS_DFU_H_

#include <bootloader/interface/src/menu.h>
#include <bootloader/usb_data.h>

namespace Bootloader {
  class DfuMenu : public Menu {
    public:
      DfuMenu(const char * submenu, USBData usbData);

      void setup() override;
      void postOpen() override;

    private:
      const char * m_submenuText;
      USBData m_data;
  };
}

#endif
