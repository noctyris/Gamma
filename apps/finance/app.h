#ifndef FINANCE_APP_H
#define FINANCE_APP_H

#include <escher.h>
#include "../shared/text_field_delegate_app.h"
#include "../shared/poincare_local_pool.h"
#include "../shared/shared_app.h"
#include "tvm_store.h"
#include "result_controller.h"
#include "parameters_controller.h"
#include "variable_controller.h"

namespace Finance {

class App : public Shared::TextFieldDelegateApp {
public:
  class Descriptor : public ::App::Descriptor {
  public:
    I18n::Message name() override;
    I18n::Message upperName() override;
    App::Descriptor::ExaminationLevel examinationLevel() override;
    const Image * icon() override;
  };
  class Snapshot : public ::SharedApp::Snapshot {
  public:
    Snapshot();
    App * unpack(Container * container) override;
    Descriptor * descriptor() override;
    void reset() override;
    TvmStore * store() { return &m_store; }
  private:
    TvmStore m_store;
  };
  static App * app() {
    return static_cast<App *>(Container::activeApp());
  }
  Snapshot * snapshot() const {
    return static_cast<Snapshot *>(::App::snapshot());
  }
  TELEMETRY_ID("Finance");
private:
  App(Snapshot * snapshot);
  Shared::PoincareLocalPool m_poincarePool;
  ResultController m_resultController;
  ParametersController m_parametersController;
  VariableController m_variableController;
  StackViewController m_stackViewController;
};

}

#endif
