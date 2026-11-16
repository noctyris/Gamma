#include "installer.h"
#include <bootloader/interface/static/messages.h>
#include <bootloader/usb_data.h>

#include <bootloader/interface/menus/dfu.h>

Bootloader::DfuMenu * Bootloader::InstallerMenu::SlotsDFU() {
  static DfuMenu * slotsDfu = new DfuMenu(Messages::dfuSlotsUpdate, USBData::DEFAULT());
  return slotsDfu;
}

Bootloader::DfuMenu * Bootloader::InstallerMenu::BootloaderDFU() {
  static DfuMenu * bootloaderDfu = new DfuMenu(Messages::dfuBootloaderUpdate, USBData::BOOTLOADER_UPDATE());
  return bootloaderDfu;
}

Bootloader::InstallerMenu::InstallerMenu() : Menu(KDColorBlack, KDColorWhite, Messages::installerTitle, Messages::mainTitle) {
  setup();
}

bool slotsSubmenu() {
  Bootloader::InstallerMenu::SlotsDFU()->open();
  return true;
}

bool bootloaderSubmenu() {
  Bootloader::InstallerMenu::BootloaderDFU()->open();
  return true;
}

void Bootloader::InstallerMenu::setup() {
  m_defaultColumns[0] = Column(Messages::installerText1, k_large_font, 0, true);
  m_defaultColumns[1] = Column(Messages::installerSlotsSubmenu, Ion::Keyboard::Key::One, k_small_font, 30, false, &slotsSubmenu);
  m_defaultColumns[2] = Column(Messages::installerBootloaderSubmenu, Ion::Keyboard::Key::Two, k_small_font, 30, false, &bootloaderSubmenu);

  m_columns[0] = ColumnBinder(&m_defaultColumns[0]);
  m_columns[1] = ColumnBinder(&m_defaultColumns[1]);
  m_columns[2] = ColumnBinder(&m_defaultColumns[2]);
}
