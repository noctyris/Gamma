#include "list_controller.h"
#include "../app.h"
#include <apps/i18n.h>
#include <assert.h>
#include <escher/metric.h>
#include <apps/apps_container.h>

using namespace Shared;

namespace Graph {

namespace {

const char * InequalityTitle(ContinuousFunction::Inequality inequality) {
  switch (inequality) {
  case ContinuousFunction::Inequality::Inferior:
    return "y<";
  case ContinuousFunction::Inequality::InferiorEqual:
    return "y≤";
  case ContinuousFunction::Inequality::Superior:
    return "y>";
  case ContinuousFunction::Inequality::SuperiorEqual:
    return "y≥";
  default:
    assert(inequality == ContinuousFunction::Inequality::None);
    return "y";
  }
}

const char * ImplicitInequalityTitle(ContinuousFunction::Inequality inequality) {
  switch (inequality) {
  case ContinuousFunction::Inequality::Inferior:
    return "<0";
  case ContinuousFunction::Inequality::InferiorEqual:
    return "≤0";
  case ContinuousFunction::Inequality::Superior:
    return ">0";
  case ContinuousFunction::Inequality::SuperiorEqual:
    return "≥0";
  default:
    assert(inequality == ContinuousFunction::Inequality::None);
    return "";
  }
}

}

ListController::ListController(Responder * parentResponder, ButtonRowController * header, ButtonRowController * footer, InputEventHandlerDelegate * inputEventHandlerDelegate) :
  Shared::FunctionListController(parentResponder, header, footer, I18n::Message::AddElement),
  m_functionTitleCells{ //TODO find better initialization
    TextFieldFunctionTitleCell(this),
    TextFieldFunctionTitleCell(this),
    TextFieldFunctionTitleCell(this),
    TextFieldFunctionTitleCell(this),
    TextFieldFunctionTitleCell(this),
  },
  m_expressionCells{},
  m_modelsParameterController(this, this),
  m_modelsStackController(nullptr, &m_modelsParameterController, Palette::BannerFirstVariantText, Palette::BannerFirstVariantBackground, Palette::BannerFirstVariantBorder),
  m_parameterController(this, this, I18n::Message::FunctionColor, I18n::Message::DeleteFunction, inputEventHandlerDelegate)
{
  for (int i = 0; i < k_maxNumberOfDisplayableRows; i++) {
    m_expressionCells[i].setLeftMargin(k_expressionMargin);
  }
}

const char * ListController::title() {
  return I18n::translate(I18n::Message::FunctionTab);
}

void ListController::renameSelectedFunction() {
  assert(selectedColumn() == 0);
  assert(selectedRow() >= 0 && selectedRow() < numberOfRows()-1); // TODO change if sometimes the addFunction row is not displayed

  // Inequalities, implicit models and point clouds have no function name to
  // edit.
  ExpiringPointer<ContinuousFunction> function = modelStore()->modelForRecord(modelStore()->recordAtIndex(modelIndexForRow(selectedRow())));
  if (function->isInequality() || function->isImplicit() ||
      function->isScatterPlot()) {
    return;
  }

  // Increase the size of the name column
  computeTitlesColumnWidth(true);
  selectableTableView()->reloadData();

  AppsContainer::sharedAppsContainer()->setShiftAlphaStatus(Ion::Events::ShiftAlphaStatus::AlphaLock);
  TextFieldFunctionTitleCell * selectedTitleCell = (TextFieldFunctionTitleCell *)(selectableTableView()->selectedCell());
  selectedTitleCell->setHorizontalAlignment(1.0f);
  Container::activeApp()->setFirstResponder(selectedTitleCell);
  selectedTitleCell->setEditing(true);
}

bool ListController::textFieldDidFinishEditing(TextField * textField, const char * text, Ion::Events::Event event) {
  assert(textField != nullptr);
  // Compute the new name
  size_t textLength = strlen(text);
  size_t argumentLength = UTF8Helper::HasCodePoint(text, UCodePointGreekSmallLetterTheta) ? Function::k_parenthesedThetaArgumentByteLength : Function::k_parenthesedXNTArgumentByteLength;
  constexpr int maxBaseNameSize = Function::k_maxNameWithArgumentSize;
  char baseName[maxBaseNameSize];
  if (textLength <= argumentLength) {
    // The user entered an empty name. Use a default function name.
    ContinuousFunction::DefaultName(baseName, maxBaseNameSize);
    /* We don't need to update the textfield edited text here because we are
     * sure that the default name is compliant. It will thus lead to the end of
     * edition and its content will be reloaded by willDisplayTitleCellAtIndex. */
  } else {
    assert(argumentLength <= textLength + 1);
    strlcpy(baseName, text, textLength - argumentLength + 1);
  }

  // Delete any variable with the same name
  GlobalContext::DestroyRecordsBaseNamedWithoutExtension(baseName, Ion::Storage::funcExtension);

  // Set the name
  Function::NameNotCompliantError nameError = Function::BaseNameCompliant(baseName);
  Ion::Storage::Record::ErrorStatus error = nameError == Function::NameNotCompliantError::None ? modelStore()->recordAtIndex(m_selectableTableView.selectedRow()).setBaseNameWithExtension(baseName, Ion::Storage::funcExtension) : Ion::Storage::Record::ErrorStatus::NonCompliantName;

  // Handle any error
  if (error == Ion::Storage::Record::ErrorStatus::None) {
    bool selectTab = false;
    textField->setEditing(false);
    computeTitlesColumnWidth();
    int currentRow = m_selectableTableView.selectedRow();
    if (event == Ion::Events::Down && currentRow < numberOfRows() - 1) {
      m_selectableTableView.selectCellAtLocation(m_selectableTableView.selectedColumn(), currentRow + 1);
    } else if (event == Ion::Events::Up) {
      if (currentRow > 0) {
        m_selectableTableView.selectCellAtLocation(m_selectableTableView.selectedColumn(), currentRow - 1);
      } else {
        selectTab = true;
      }
    }
    m_selectableTableView.selectedCell()->setHighlighted(true);
    m_selectableTableView.reloadData();
    Container::activeApp()->setFirstResponder(&m_selectableTableView);
    if (selectTab) {
      m_selectableTableView.parentResponder()->handleEvent(event);
    }
    AppsContainer::sharedAppsContainer()->setShiftAlphaStatus(Ion::Events::ShiftAlphaStatus::Default);
    return true;
  } else if (error == Ion::Storage::Record::ErrorStatus::NameTaken) {
    Container::activeApp()->displayWarning(I18n::Message::NameTaken);
  } else if (error == Ion::Storage::Record::ErrorStatus::NonCompliantName) {
    assert(nameError != Function::NameNotCompliantError::None);
    if (nameError == Function::NameNotCompliantError::CharacterNotAllowed) {
      Container::activeApp()->displayWarning(I18n::Message::AllowedCharactersAZaz09);
    } else if (nameError == Function::NameNotCompliantError::NameCannotStartWithNumber) {
      Container::activeApp()->displayWarning(I18n::Message::NameCannotStartWithNumber);
    } else {
      assert(nameError == Function::NameNotCompliantError::ReservedName);
      Container::activeApp()->displayWarning(I18n::Message::ReservedName);
    }
  } else {
    assert(error == Ion::Storage::Record::ErrorStatus::NotEnoughSpaceAvailable);
    Container::activeApp()->displayWarning(I18n::Message::NameTooLong);
  }
  textField->setEditing(true);
  return false;
}

bool ListController::textFieldDidAbortEditing(TextField * textField) {
  assert(textField != nullptr);
  // Put the name column back to normal size
  computeTitlesColumnWidth();
  selectableTableView()->reloadData();
  ExpiringPointer<ContinuousFunction> function = modelStore()->modelForRecord(modelStore()->recordAtIndex(selectedRow()));
  setFunctionNameInTextField(function, textField);
  m_selectableTableView.selectedCell()->setHighlighted(true);
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
  AppsContainer::sharedAppsContainer()->setShiftAlphaStatus(Ion::Events::ShiftAlphaStatus::Default);
  return true;
}

bool ListController::textFieldShouldFinishEditing(TextField * textField, Ion::Events::Event event) {
  return event == Ion::Events::Up || event == Ion::Events::Down || Shared::TextFieldDelegate::textFieldShouldFinishEditing(textField, event);
}

bool ListController::textFieldDidReceiveEvent(TextField * textField, Ion::Events::Event event) {
  assert(textField != nullptr);
  if (textField->isEditing() && textField->shouldFinishEditing(event)) {
    return false;
  }
  return Shared::TextFieldDelegate::textFieldDidReceiveEvent(textField, event);
}

Shared::ListParameterController * ListController::parameterController() {
  return &m_parameterController;
}

int ListController::maxNumberOfDisplayableRows() {
  return k_maxNumberOfDisplayableRows;
}

FunctionTitleCell * ListController::titleCells(int index) {
  assert(index >= 0 && index < k_maxNumberOfDisplayableRows);
  return &m_functionTitleCells[index];
}

HighlightCell * ListController::expressionCells(int index) {
  assert(index >= 0 && index < k_maxNumberOfDisplayableRows);
  return &m_expressionCells[index];
}

void ListController::willDisplayTitleCellAtIndex(HighlightCell * cell, int j) {
  assert(cell != nullptr);
  assert(j >= 0 && j < modelStore()->numberOfModels());
  TextFieldFunctionTitleCell * titleCell = static_cast<TextFieldFunctionTitleCell *>(cell);
  // Update the corresponding expression cell in order to get the baseline
  ExpressionModelListController::willDisplayExpressionCellAtIndex(m_selectableTableView.cellAtLocation(1, j), j);
  titleCell->setBaseline(baseline(j));
  if (!titleCell->isEditing()) {
    // Set name and color if the name is not being edited
    ExpiringPointer<ContinuousFunction> function = modelStore()->modelForRecord(modelStore()->recordAtIndex(j));
    setFunctionNameInTextField(function, titleCell->textField());
    KDColor functionNameColor = function->isActive() ? function->color() : Palette::SecondaryText;
    titleCell->setColor(functionNameColor);
  }
}

void ListController::willDisplayExpressionCellAtIndex(HighlightCell * cell, int j) {
  assert(cell != nullptr);
  assert(j >= 0 && j < modelStore()->numberOfModels());
  Shared::FunctionListController::willDisplayExpressionCellAtIndex(cell, j);
  FunctionExpressionCell * myCell = (FunctionExpressionCell *)cell;
  ExpiringPointer<ContinuousFunction> f = modelStore()->modelForRecord(modelStore()->recordAtIndex(j));
  KDColor textColor = f->isActive() ? Palette::PrimaryText : Palette::SecondaryText;
  myCell->setTextColor(textColor);
}

void ListController::setFunctionNameInTextField(ExpiringPointer<ContinuousFunction> function, TextField * textField) {
  assert(textField != nullptr);
  if (function->isImplicit()) {
    textField->setText(function->inequality() == ContinuousFunction::Inequality::None ? "" : ImplicitInequalityTitle(function->inequality()));
    return;
  }
  if (function->isInequality()) {
    textField->setText(InequalityTitle(function->inequality()));
    return;
  }
  if (function->isScatterPlot()) {
    textField->setText("");
    return;
  }
  char bufferName[BufferTextView::k_maxNumberOfChar];
  function->nameWithArgument(bufferName, BufferTextView::k_maxNumberOfChar);
  textField->setText(bufferName);
}

void ListController::addEmptyModel() {
  Container::activeApp()->displayModalViewController(&m_modelsStackController, 0.f, 0.f, Metric::CommonTopMargin, Metric::CommonRightMargin, 0, Metric::CommonLeftMargin);
}

void ListController::addModel(int modelIndex) {
  ContinuousFunction::PlotType plotType = ContinuousFunction::PlotType::Cartesian;
  ContinuousFunction::Inequality inequality = ContinuousFunction::Inequality::None;
  bool setInequalityAfterContent = false;
  bool usePolarName = false;
  const char * content = "";
  switch (modelIndex) {
  case 1:
    content = "x";
    break;
  case 2:
    plotType = ContinuousFunction::PlotType::Implicit;
    content = "x+y+1=0";
    break;
  case 3:
    plotType = ContinuousFunction::PlotType::Implicit;
    content = "x+y≤0";
    break;
  case 4:
    plotType = ContinuousFunction::PlotType::Implicit;
    content = "x=√(y)";
    break;
  case 5:
    content = "{-x,x<0\\\\x,x≥0}";
    break;
  case 6:
    content = "(2,3)";
    break;
  case 7:
    content = "{(2,3),(3,5)}";
    break;
  case 8:
    plotType = ContinuousFunction::PlotType::Implicit;
    content = "x^2+y^2+x*y+x+y=0";
    break;
  case 9:
    plotType = ContinuousFunction::PlotType::Parametric;
    content = "(cos(t),sin(t))";
    break;
  case 10:
    plotType = ContinuousFunction::PlotType::Polar;
    content = "cos(θ)";
    usePolarName = true;
    break;
  case 11:
    inequality = ContinuousFunction::Inequality::Inferior;
    setInequalityAfterContent = true;
    content = "x";
    break;
  case 12:
    inequality = ContinuousFunction::Inequality::InferiorEqual;
    setInequalityAfterContent = true;
    content = "x";
    break;
  case 13:
    inequality = ContinuousFunction::Inequality::Superior;
    setInequalityAfterContent = true;
    content = "x";
    break;
  case 14:
    inequality = ContinuousFunction::Inequality::SuperiorEqual;
    setInequalityAfterContent = true;
    content = "x";
    break;
  default:
    assert(modelIndex == 0);
  }

  // The new model takes the place of the "add" row.
  int newRow = selectedRow();
  Ion::Storage::Record::ErrorStatus error = modelStore()->addEmptyModel();
  if (error == Ion::Storage::Record::ErrorStatus::NotEnoughSpaceAvailable) {
    return;
  }
  assert(error == Ion::Storage::Record::ErrorStatus::None);
  selectableTableView()->selectCellAtLocation(1, newRow);
  Ion::Storage::Record record = modelStore()->recordAtIndex(modelIndexForRow(newRow));
  if (usePolarName) {
    char polarName[Poincare::SymbolAbstract::k_maxNameSize];
    ContinuousFunction::DefaultPolarName(polarName, sizeof(polarName));
    record.setBaseNameWithExtension(polarName, Ion::Storage::funcExtension);
    record = modelStore()->recordAtIndex(modelIndexForRow(newRow));
  }
  ExpiringPointer<ContinuousFunction> function = modelStore()->modelForRecord(record);
  function->setPlotType(plotType, Poincare::Preferences::sharedPreferences()->angleUnit(), App::app()->localContext());
  function->setContent(content, App::app()->localContext());
  if (setInequalityAfterContent) {
    function->setInequality(inequality);
  }
  didChangeModelsList();
  selectableTableView()->reloadData();
  editExpression(Ion::Events::OK);
}

ContinuousFunctionStore * ListController::modelStore() {
  return App::app()->functionStore();
}

}
