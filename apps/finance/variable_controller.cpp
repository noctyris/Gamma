#include "variable_controller.h"
#include "parameters_controller.h"

namespace Finance {

VariableController::VariableController(Responder * parentResponder, TvmStore * store, ParametersController * parametersController) :
  ViewController(parentResponder),
  m_selectableTableView(this),
  m_cells{},
  m_store(store),
  m_parametersController(parametersController)
{
}

const char * VariableController::title() {
  return I18n::translate(I18n::Message::FinanceChooseVariable);
}

void VariableController::didBecomeFirstResponder() {
  if (selectedRow() < 0) {
    selectCellAtLocation(0, 0);
  }
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

void VariableController::viewWillAppear() {
  ViewController::viewWillAppear();
  selectCellAtLocation(0, static_cast<int>(m_store->unknown()));
  m_selectableTableView.reloadData();
}

bool VariableController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
    m_store->setUnknown(static_cast<TvmStore::Variable>(selectedRow()));
    StackViewController * stack = static_cast<StackViewController *>(parentResponder());
    stack->push(m_parametersController);
    return true;
  }
  return false;
}

HighlightCell * VariableController::reusableCell(int index) {
  assert(index >= 0 && index < TvmStore::k_numberOfTvmVariables);
  return &m_cells[index];
}

void VariableController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  MessageTableCell<> * myCell = static_cast<MessageTableCell<> *>(cell);
  myCell->setMessage(TvmStore::labelMessage(static_cast<TvmStore::Variable>(index)));
}

}
