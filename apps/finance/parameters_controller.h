#ifndef FINANCE_PARAMETERS_CONTROLLER_H
#define FINANCE_PARAMETERS_CONTROLLER_H

#include <escher.h>
#include "tvm_store.h"
#include "../shared/float_parameter_controller.h"

namespace Finance {

class ResultController;

class ParametersController : public Shared::FloatParameterController<double> {
public:
  ParametersController(Responder * parentResponder, InputEventHandlerDelegate * inputEventHandlerDelegate, TvmStore * store, ResultController * resultController);
  const char * title() override;
  int numberOfRows() const override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;
  TELEMETRY_ID("FinanceParameters");
private:
  static constexpr int k_numberOfParameterCells = 6;
  TvmStore::Variable variableForRow(int row) const;
  HighlightCell * reusableParameterCell(int index, int type) override;
  int reusableParameterCellCount(int type) override;
  double parameterAtIndex(int index) override;
  bool setParameterAtIndex(int index, double value) override;
  void buttonAction() override;

  MessageTableCellWithEditableText m_cells[k_numberOfParameterCells];
  TvmStore * m_store;
  ResultController * m_resultController;
};

}

#endif
