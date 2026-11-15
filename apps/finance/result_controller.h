#ifndef FINANCE_RESULT_CONTROLLER_H
#define FINANCE_RESULT_CONTROLLER_H

#include <escher.h>
#include "tvm_store.h"

namespace Finance {

class ResultController : public ViewController {
public:
  ResultController(Responder * parentResponder, TvmStore * store);
  View * view() override { return &m_messageView; }
  const char * title() override;
  bool handleEvent(Ion::Events::Event event) override;
  void viewWillAppear() override;
  TELEMETRY_ID("FinanceResult");
private:
  MessageTextView m_messageView;
  TvmStore * m_store;
  char m_buffer[64];
};

}

#endif
