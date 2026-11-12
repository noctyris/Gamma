#include "parameters_controller.h"
#include "result_controller.h"

namespace Finance {

ParametersController::ParametersController(Responder * parentResponder, InputEventHandlerDelegate * inputEventHandlerDelegate, TvmStore * store, ResultController * resultController) :
  Shared::FloatParameterController<double>(parentResponder),
  m_cells{},
  m_store(store),
  m_resultController(resultController)
{
  m_okButton.setMessage(I18n::Message::FinanceCompute);
  for (int i = 0; i < k_numberOfParameterCells; i++) {
    m_cells[i].setParentResponder(&m_selectableTableView);
    m_cells[i].textField()->setDelegates(inputEventHandlerDelegate, this);
  }
}

const char * ParametersController::title() {
  return I18n::translate(I18n::Message::FinanceTvm);
}

int ParametersController::numberOfRows() const {
  return k_numberOfParameterCells + 1;
}

TvmStore::Variable ParametersController::variableForRow(int row) const {
  int currentRow = 0;
  for (int i = 0; i < TvmStore::k_numberOfVariables; i++) {
    const TvmStore::Variable variable = static_cast<TvmStore::Variable>(i);
    if (variable == m_store->unknown()) {
      continue;
    }
    if (currentRow == row) {
      return variable;
    }
    currentRow++;
  }
  return TvmStore::Variable::N;
}

HighlightCell * ParametersController::reusableParameterCell(int index, int type) {
  assert(index >= 0 && index < k_numberOfParameterCells);
  return &m_cells[index];
}

int ParametersController::reusableParameterCellCount(int type) {
  return k_numberOfParameterCells;
}

double ParametersController::parameterAtIndex(int index) {
  return m_store->value(variableForRow(index));
}

bool ParametersController::setParameterAtIndex(int index, double value) {
  const TvmStore::Variable variable = variableForRow(index);
  if (variable == TvmStore::Variable::N && value < 0.0) {
    return false;
  }
  if (variable == TvmStore::Variable::IY && value <= -100.0) {
    return false;
  }
  if ((variable == TvmStore::Variable::PY || variable == TvmStore::Variable::CY) && value <= 0.0) {
    return false;
  }
  m_store->setValue(variable, value);
  return true;
}

void ParametersController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index == numberOfRows() - 1) {
    return;
  }
  Shared::FloatParameterController<double>::willDisplayCellForIndex(cell, index);
  MessageTableCellWithEditableText * myCell = static_cast<MessageTableCellWithEditableText *>(cell);
  myCell->setMessage(TvmStore::labelMessage(variableForRow(index)));
}

void ParametersController::buttonAction() {
  if (!m_store->compute()) {
    Container::activeApp()->displayWarning(I18n::Message::FinanceNoSolution);
    return;
  }
  StackViewController * stack = stackController();
  stack->push(m_resultController);
}

}
