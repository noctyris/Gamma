#include "kandinsky/color.h"
#include "kandinsky/font.h"
#include "kandinsky/ion_context.h"
#include "kandinsky/point.h"
#include <ion.h>

// This function isn't technically an SVC call, but is used inside
// UserlandHeader as a trampoline to the userland draw_string. This is used by
// NWA apps only, so it makes sense to declare it here.
extern "C" void eadk_display_draw_string(const char * text, KDPoint point, bool large_font, KDColor text_color, KDColor background_color) {
  auto ctx = KDIonContext::sharedContext();
  ctx->setClippingRect(KDRect(0, 0, 320, 240));
  ctx->setOrigin(KDPoint(0, 0));
  point = ctx->drawString(text, point, large_font ? KDFont::LargeFont : KDFont::SmallFont, text_color, background_color);
}


// We don't need a corresponding header file because the function is directly
// referenced from ASM
extern "C" void __attribute__((noinline, used)) svcall_handler_c(uint32_t * svc_args) {
  unsigned int svc_number;
  /*
  * Stack contains:
  * R0, R1, R2, R3, R12, R14, the return address and xPSR
  * First argument (R0) is svc_args[0]
  */
  svc_number = ((char *)svc_args[6])[-2];
  // Mapping from https://github.com/numworks/epsilon-sample-app-cpp/blob/e654d85d82f12ba9462bb7af41c2a7694572f0e1/eadk/eadk.s
  // (We can't use data from Epsilon's repo due to license incompatibility)
  switch(svc_number) {
    case 01:
      // eadk_backlight_brightness
      // Value is returned in r0
      svc_args[0] = Ion::Backlight::brightness();
      break;
    case 02:
      // eadk_backlight_set_brightness
      Ion::Backlight::setBrightness(svc_args[0]);
      break;
    case 03:
      // eadk_battery_is_charging
      svc_args[0] = Ion::Battery::isCharging();
      break;
    case 04:
      // eadk_battery_level
      // FIXME: This will cause compatibility issues with Epsilon as we have
      // less battery states than newer versions
      svc_args[0] = (uint32_t)Ion::Battery::level();
      break;
    case 05:
      // eadk_battery_voltage
      svc_args[0] = Ion::Battery::voltage();
      break;
    case 18:
      {
        // eadk_display_pull_rect
        KDRect rect = *(KDRect *)&svc_args[0];
        KDColor * pixels = (KDColor *)svc_args[2];
        Ion::Display::pullRect(rect, pixels);
        break;
      }
    case 19:
      {
        // eadk_display_push_rect
        KDRect rect = *(KDRect *)&svc_args[0];
        const KDColor * pixels = reinterpret_cast<const KDColor *>(svc_args[2]);
        Ion::Display::pushRect(rect, pixels);
        break;
      }
    case 20:
      {
        // eadk_display_push_rect_uniform
        KDRect rect = *(KDRect *)&svc_args[0];
        uint16_t color = svc_args[2];

        Ion::Display::pushRectUniform(rect, KDColor::RGB16(color));
        break;
      }
    case 21:
      // eadk_display_wait_for_vblank
      Ion::Display::waitForVBlank();
      break;
    case 23:
      // eadk_event_get
      svc_args[0] = Ion::Events::getEvent((int32_t *)svc_args[0]).id();
      break;
    case 34:
      {
        // eadk_keyboard_scan
        // We have 2 registers here: r0 and r1
        uint64_t state = Ion::Keyboard::scan();
        svc_args[0] = state;
        svc_args[1] = state >> 32;
        break;
      }
    case 45:
      svc_args[0] = Ion::random();
      break;
    case 48:
      {
        // eadk_timing_millis
        // We have 2 registers here: r0 and r1
        uint64_t millis = Ion::Timing::millis();
        svc_args[0] = millis;
        svc_args[1] = millis >> 32;
        break;
      }
    case 49:
      Ion::Timing::msleep(svc_args[0]);
      break;
    case 50:
      Ion::Timing::usleep(svc_args[0]);
      break;
    default:
      /* Unknown SVC */
      // TODO: Show an error message
      eadk_display_draw_string("Invalid SVC call, ignoring for compatibility.", KDPoint(0, 0), false, KDColorBlack, KDColorWhite);
      eadk_display_draw_string("Please add the missing syscall to Upsilon", KDPoint(0, 14), false, KDColorBlack, KDColorWhite);
      Ion::Timing::msleep(250);
      break;
  }
}
