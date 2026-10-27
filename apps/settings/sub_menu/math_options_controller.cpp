#include "math_options_controller.h"
#include <assert.h>
#include "../../global_preferences.h"

using namespace Poincare;

namespace Settings {

MathOptionsController::MathOptionsController(Responder * parentResponder, InputEventHandlerDelegate * inputEventHandlerDelegate) :
  GenericSubController(parentResponder),
  m_preferencesController(this),
  m_displayModeController(this, inputEventHandlerDelegate)
{
  for (int i = 0; i < k_numberOfSimpleChevronCells; i++) {
    m_cells[i].setMessageFont(KDFont::LargeFont);
  }
  m_unitsWithoutUnderscoreCell.setMessageFont(KDFont::LargeFont);
}

bool MathOptionsController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
    if (m_messageTreeModel->childAtIndex(selectedRow())->label() == I18n::Message::UnitsWithoutUnderscore) {
      Preferences * preferences = Preferences::sharedPreferences();
      preferences->setUnitsWithoutUnderscore(!preferences->unitsWithoutUnderscore());
      m_selectableTableView.reloadCellAtLocation(m_selectableTableView.selectedColumn(), m_selectableTableView.selectedRow());
      return true;
    }
    GenericSubController * subController = nullptr;
    if (m_messageTreeModel->childAtIndex(selectedRow())->label() == I18n::Message::DisplayMode)
      subController = &m_displayModeController;
    else
      subController = &m_preferencesController;
    subController->setMessageTreeModel(m_messageTreeModel->childAtIndex(selectedRow()));
    StackViewController * stack = stackController();
    m_lastSelect = selectedRow();
    stack->push(subController);
    return true;
  }
  return GenericSubController::handleEvent(event);
}

int MathOptionsController::typeAtLocation(int i, int j) {
  if (m_messageTreeModel->childAtIndex(j)->label() == I18n::Message::UnitsWithoutUnderscore) {
    return k_unitsWithoutUnderscoreCellType;
  }
  return 0;
}

HighlightCell * MathOptionsController::reusableCell(int index, int type) {
  if (type == k_unitsWithoutUnderscoreCellType) {
    assert(index == 0);
    return &m_unitsWithoutUnderscoreCell;
  }
  assert(type == 0);
  assert(index >= 0 && index < k_numberOfSimpleChevronCells);
  return &m_cells[index];
}

int MathOptionsController::reusableCellCount(int type) {
  if (type == k_unitsWithoutUnderscoreCellType) {
    return 1;
  }
  assert(type == 0);
  return k_numberOfSimpleChevronCells;
}

void MathOptionsController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (m_messageTreeModel->childAtIndex(index)->label() == I18n::Message::UnitsWithoutUnderscore) {
    GenericSubController::willDisplayCellForIndex(cell, index);
    MessageTableCellWithSwitch * mySwitchCell = (MessageTableCellWithSwitch *)cell;
    SwitchView * mySwitch = (SwitchView *)mySwitchCell->accessoryView();
    mySwitch->setState(Preferences::sharedPreferences()->unitsWithoutUnderscore());
    return;
  }
  MessageTableCellWithChevronAndMessage * myTextCell = (MessageTableCellWithChevronAndMessage *)cell;
  Preferences * preferences = Preferences::sharedPreferences();
  I18n::Message thisLabel = m_messageTreeModel->childAtIndex(index)->label();
  myTextCell->setMessage(thisLabel);

  //add text for preferences
  int childIndex = -1;
  switch (thisLabel) {
    case I18n::Message::AngleUnit:
      childIndex = (int)preferences->angleUnit();
      break;
    case I18n::Message::DisplayMode:
      childIndex = (int)preferences->displayMode();
      break;
    case I18n::Message::EditionMode:
      childIndex = (int)preferences->editionMode();
      break;
    case I18n::Message::ComplexFormat:
      childIndex = (int)preferences->complexFormat();
      break;
    case I18n::Message::SymbolMultiplication:
      childIndex = (int)preferences->symbolOfMultiplication();
      break;
    case I18n::Message::SymbolFunction:
      childIndex = (int)preferences->symbolOfFunction();
      break;      
    default:
      break;
  }
  I18n::Message message = childIndex >= 0 ? m_messageTreeModel->childAtIndex(index)->childAtIndex(childIndex)->label() : I18n::Message::Default;
  myTextCell->setSubtitle(message);
}

}
