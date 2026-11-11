#include "function_models_parameter_controller.h"
#include "list_controller.h"
#include <poincare/expression.h>
#include <poincare/layout_helper.h>
#include <poincare/preferences.h>
#include <apps/i18n.h>
#include <assert.h>
#include <string.h>

using namespace Poincare;

namespace Graph {

namespace {

bool ContainsInequalityOperator(const char * text) {
  /* Do not use strstr here: the device libc is freestanding and does not
   * declare it. The UTF-8 sequences are the two comparison symbols. */
  const unsigned char * pointer = reinterpret_cast<const unsigned char *>(text);
  for (; *pointer != 0; pointer++) {
    if (*pointer == '<' || *pointer == '>') {
      return true;
    }
    if (pointer[0] == 0xE2 && pointer[1] == 0x89 &&
        (pointer[2] == 0xA4 || pointer[2] == 0xA5)) {
      return true;
    }
  }
  return false;
}

Poincare::Layout ModelLayout(const char * text) {
  /* The inequality templates cannot be parsed as plain expressions. The other
   * templates are parsed to get proper exponents and square roots. */
  if (!ContainsInequalityOperator(text)) {
    Poincare::Expression expression = Poincare::Expression::Parse(text, nullptr);
    if (!expression.isUninitialized()) {
      return expression.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 1);
    }
  }
  return Poincare::LayoutHelper::String(text, strlen(text));
}

}

constexpr const char * FunctionModelsParameterController::k_modelDescriptions[k_numberOfExpressionCells];

FunctionModelsParameterController::FunctionModelsParameterController(Responder * parentResponder, ListController * listController) :
  ViewController(parentResponder),
  m_emptyModelCell(I18n::Message::Empty, KDFont::LargeFont),
  m_modelCells{},
  m_selectableTableView(this),
  m_listController(listController)
{
  m_selectableTableView.setMargins(0);
  m_selectableTableView.setDecoratorType(ScrollView::Decorator::Type::None);
  for (int i = 0; i < k_numberOfReusableCells; i++) {
    m_modelCells[i].setParentResponder(&m_selectableTableView);
  }
}

const char * FunctionModelsParameterController::title() {
  return I18n::translate(I18n::Message::AddElement);
}

void FunctionModelsParameterController::viewWillAppear() {
  ViewController::viewWillAppear();
  selectCellAtLocation(0, 0);
}

void FunctionModelsParameterController::didBecomeFirstResponder() {
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

bool FunctionModelsParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    int modelIndex = selectedRow();
    Container::activeApp()->dismissModalViewController();
    m_listController->addModel(modelIndex);
    return true;
  }
  return false;
}

int FunctionModelsParameterController::numberOfRows() const {
  return k_numberOfModels;
}

KDCoordinate FunctionModelsParameterController::rowHeight(int j) {
  return Metric::ToolboxRowHeight;
}

KDCoordinate FunctionModelsParameterController::cumulatedHeightFromIndex(int j) {
  return rowHeight(0) * j;
}

int FunctionModelsParameterController::indexFromCumulatedHeight(KDCoordinate offsetY) {
  KDCoordinate height = rowHeight(0);
  if (height == 0) {
    return 0;
  }
  return (offsetY - 1) / height;
}

HighlightCell * FunctionModelsParameterController::reusableCell(int index, int type) {
  if (type == 0) {
    return &m_emptyModelCell;
  }
  assert(index >= 0 && index < k_numberOfReusableCells);
  return &m_modelCells[index];
}

int FunctionModelsParameterController::reusableCellCount(int type) {
  if (type == 0) {
    return 1;
  }
  return k_numberOfReusableCells;
}

void FunctionModelsParameterController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index == 0) {
    return;
  }
  assert(index > 0 && index <= k_numberOfExpressionCells);
  /* The cells are reused while scrolling: build the layout of the row that is
   * being displayed instead of keeping one layout per template in RAM. */
  static_cast<ExpressionTableCell *>(cell)->setLayout(ModelLayout(k_modelDescriptions[index - 1]));
}

int FunctionModelsParameterController::typeAtLocation(int i, int j) {
  if (j == 0) {
    return 0;
  }
  return 1;
}

}
