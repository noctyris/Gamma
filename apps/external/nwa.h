#ifndef EXTERNAL_NWA_H
#define EXTERNAL_NWA_H

#include <stddef.h>
#include <stdint.h>

namespace External {
namespace NWA {

constexpr uint32_t k_appHeaderMagik = 0xDEC0BEBA;

struct NWAHeader {
  uint32_t m_header;
  uint32_t m_apiLevel;
  uint32_t m_appNameOffset;
  uint32_t m_appIconOffset;
  uint32_t m_appIconSize;
  uint32_t m_entrypointOffset;
  uint32_t m_size;
  uint32_t m_footer;
} __attribute__((packed));

const NWAHeader * appAtIndex(size_t index);
uint32_t executeApp(size_t index);
size_t numberOfApps();

}
}

#endif
