#include "nwa.h"
#include "archive.h"
#include "extapp_api.h"
#include "../global_preferences.h"
#include <ion.h>

#include <string.h>
#include <stdlib.h>

namespace External {
namespace NWA {

#ifdef DEVICE

bool isSane(const NWAHeader * app) {
  if ((app->m_header != k_appHeaderMagik) || (app->m_footer != k_appHeaderMagik))  {
    return false;
  }

  // Check if the app footer collide with the extapp archive
  char * appEnd = (char *)app + app->m_size;
  if (((char *)app < (char *)0x90200000) && (appEnd >= (char *)0x90200000)) {
    // App collide with the extapp archive
    if (External::Archive::isArchiveHeaderValid()) {
      // Archive is valid, assume the app is invalid/corrupted (partially
      // overwritten by Archive data)
      return false;
    }
  }

  return true;
}

const NWAHeader * firstApp() {
  // Block NWA apps in exam mode (the format mixes user data and app code)
  if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode()) {
    return nullptr;
  }

  const NWAHeader * appHeader = reinterpret_cast<const NWAHeader*>(Ion::externalAppsFlashStart());
  return appHeader;
}

const NWAHeader * nextApp(const NWAHeader * appHeader) {
  uint32_t endofAppAddress = (uint32_t)((char *)appHeader + appHeader->m_size);

  // Apps are rounded to the 64K sector
  uint32_t roundedEndOfAppAddress = endofAppAddress & 0xFFFF0000;

  // Return immediately the address was already aligned (in case the app size
  // was exactly one sector, for example)
  if (endofAppAddress == roundedEndOfAppAddress) {
    const NWAHeader * nextAppHeader = (const NWAHeader *)roundedEndOfAppAddress;
    return nextAppHeader;
  }

  // Otherwise, skip the partially filled sector
  const NWAHeader * nextAppHeader = (const NWAHeader *)(roundedEndOfAppAddress + 0x10000);

  if (!isSane(nextAppHeader)) {
    // The app could be invalid due to a conflict with extapps. If the header is
    // still valid, try to skip to the next app. This allow structures like this:
    // NWA App 1 | NWA App 2/overwritten by extapps | NWA App 3
    // (of course this only work if NWA App 2 is longer than the full extapps
    // archive)
    if ((nextAppHeader->m_header == k_appHeaderMagik) && (nextAppHeader->m_footer == k_appHeaderMagik))  {
      return nextApp(nextAppHeader);
    }

    return nullptr;
  }

  return nextAppHeader;
}

const NWAHeader * appAtIndex(size_t index) {
  if (index == -1) {
    return nullptr;
  }

  const NWAHeader * appHeader = firstApp();

  // Sanity check.
  if (!isSane(appHeader)) {
    return nullptr;
  }

  while (true) {
    // Check if we found our file.
    if (index == 0) {
      // If yes, check for sanity
      if (!isSane(appHeader)) {
        return nullptr;
      }

      return appHeader;
    } else {
      appHeader = nextApp(appHeader);

      if (appHeader == nullptr) {
        return nullptr;
      }
    }
    index--;
  }

  // Achievement unlock: How did we get there ?
  return nullptr;
}

typedef uint32_t (*entrypoint)();

uint32_t executeApp(size_t index) {
  if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode()) {
    return 0;
  }

  const NWAHeader * app = appAtIndex(index);
  if (app != nullptr) {
    char * entrypointAddress = (char *)app + app->m_entrypointOffset + 1;
    return ((entrypoint)entrypointAddress)();
  }
  return -1;
}


#else

const NWAHeader * firstApp() {
  return nullptr;
}

const NWAHeader * nextApp(const NWAHeader * appHeader) {
  return nullptr;
}

const NWAHeader * appAtIndex(size_t index) {
  return nullptr;
}

uint32_t executeApp(size_t index) {
  return -1;
}

#endif

size_t numberOfApps() {
  const NWAHeader * appHeader = firstApp();
  size_t count = 0;

  while (appHeader != nullptr) {
    count++;
    appHeader = nextApp(appHeader);
  }

  return count;
}

}
}
