#include "result_controller.h"
#include <apps/shared/poincare_helpers.h>
#include <poincare/preferences.h>
#include <string.h>

namespace Finance {

ResultController::ResultController(Responder * parentResponder, TvmStore * store) :
  ViewController(parentResponder),
  m_messageView(KDFont::LargeFont, I18n::Message::Default, 0.5f, 0.5f, Palette::PrimaryText, Palette::BackgroundApps),
  m_store(store),
  m_buffer{}
{
}

const char * ResultController::title() {
  return I18n::translate(I18n::Message::FinanceResult);
}

void ResultController::viewWillAppear() {
  ViewController::viewWillAppear();
  const char * label = I18n::translate(TvmStore::labelMessage(m_store->unknown()));
  strlcpy(m_buffer, label, sizeof(m_buffer));
  strlcat(m_buffer, " = ", sizeof(m_buffer));
  const int length = strlen(m_buffer);
  if (length < static_cast<int>(sizeof(m_buffer))) {
    Shared::PoincareHelpers::ConvertFloatToText<double>(
        m_store->result(),
        m_buffer + length,
        sizeof(m_buffer) - length,
        Poincare::Preferences::LargeNumberOfSignificantDigits);
  }
  if (m_store->unknown() == TvmStore::Variable::IY) {
    strlcat(m_buffer, "%", sizeof(m_buffer));
  }
  m_messageView.setText(m_buffer);
}

bool ResultController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Back) {
    StackViewController * stack = static_cast<StackViewController *>(parentResponder());
    if (stack != nullptr) {
      stack->pop();
    }
    return true;
  }
  return false;
}

}
