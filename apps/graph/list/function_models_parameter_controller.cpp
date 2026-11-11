#include "function_models_parameter_controller.h"
#include "list_controller.h"
#include <poincare/layout_helper.h>
#include <apps/i18n.h>
#include <assert.h>

using namespace Poincare;

namespace Graph {

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
  for (int i = 0; i < k_numberOfExpressionCells; i++) {
    Layout layout = LayoutHelper::String(k_modelDescriptions[i], strlen(k_modelDescriptions[i]));
    m_modelCells[i].setLayout(layout);
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
  assert(index >= 0 && index < k_numberOfExpressionCells);
  return &m_modelCells[index];
}

int FunctionModelsParameterController::reusableCellCount(int type) {
  if (type == 0) {
    return 1;
  }
  return k_numberOfExpressionCells;
}

int FunctionModelsParameterController::typeAtLocation(int i, int j) {
  if (j == 0) {
    return 0;
  }
  return 1;
}

}
