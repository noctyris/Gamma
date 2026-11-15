#include "app.h"
#include <apps/i18n.h>
#include "finance_icon.h"

namespace Finance {

I18n::Message App::Descriptor::name() {
  return I18n::Message::FinanceApp;
}

I18n::Message App::Descriptor::upperName() {
  return I18n::Message::FinanceAppCapital;
}

App::Descriptor::ExaminationLevel App::Descriptor::examinationLevel() {
  return App::Descriptor::ExaminationLevel::Strict;
}

const Image * App::Descriptor::icon() {
  return ImageStore::FinanceIcon;
}

App::Snapshot::Snapshot() :
  m_store()
{
}

App * App::Snapshot::unpack(Container * container) {
  return new (container->currentAppBuffer()) App(this);
}

App::Descriptor * App::Snapshot::descriptor() {
  static Descriptor descriptor;
  return &descriptor;
}

void App::Snapshot::reset() {
  m_store.reset();
}

App::App(Snapshot * snapshot) :
  Shared::TextFieldDelegateApp(snapshot, &m_stackViewController),
  m_resultController(&m_stackViewController, snapshot->store()),
  m_parametersController(&m_stackViewController, this, snapshot->store(), &m_resultController),
  m_variableController(&m_stackViewController, snapshot->store(), &m_parametersController),
  m_stackViewController(&m_modalViewController, &m_variableController)
{
}

}
