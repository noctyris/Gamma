#include "function_app.h"

using namespace Poincare;

namespace Shared {

FunctionApp::Snapshot::Snapshot() :
  m_cursor(),
  m_indexFunctionSelectedByCursor(0),
  m_rangeVersion(0)
{
}

void FunctionApp::Snapshot::reset() {
  m_indexFunctionSelectedByCursor = 0;
  m_rangeVersion = 0;
  setActiveTab(0);
}

void FunctionApp::Snapshot::storageDidChangeForRecord(const Ion::Storage::Record record) {
  functionStore()->storageDidChangeForRecord(record);
}

void FunctionApp::willBecomeInactive() {
  if (m_modalViewController.isDisplayingModal()) {
    m_modalViewController.dismissModalViewController(true);
  }
  if (inputViewController()->isDisplayingModal()) {
    inputViewController()->abortEditionAndDismiss();
  }
  ::App::willBecomeInactive();
}

bool FunctionApp::isAcceptableExpression(const Poincare::Expression expression) {
  /* Accept equations so that an implicit equation such as x^2+y^2=1 can be
   * typed directly in the function list. Applications that do not support
   * equations (such as Sequences) override this method to reject them. */
  if (!TextFieldDelegateApp::ExpressionCanBeSerialized(expression, false, Expression(), localContext())) {
    return false;
  }
  return TextFieldDelegateApp::isAcceptableExpression(expression);
}

}
