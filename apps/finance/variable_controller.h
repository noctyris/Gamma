#ifndef FINANCE_VARIABLE_CONTROLLER_H
#define FINANCE_VARIABLE_CONTROLLER_H

#include <escher.h>
#include "tvm_store.h"

namespace Finance {

class ParametersController;

class VariableController : public ViewController, public SimpleListViewDataSource, public SelectableTableViewDataSource {
public:
  VariableController(Responder * parentResponder, TvmStore * store, ParametersController * parametersController);
  View * view() override { return &m_selectableTableView; }
  const char * title() override;
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  void viewWillAppear() override;
  int numberOfRows() const override { return TvmStore::k_numberOfTvmVariables; }
  KDCoordinate cellHeight() override { return Metric::ParameterCellHeight; }
  HighlightCell * reusableCell(int index) override;
  int reusableCellCount() const override { return TvmStore::k_numberOfTvmVariables; }
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;
  TELEMETRY_ID("FinanceVariables");
private:
  SelectableTableView m_selectableTableView;
  MessageTableCell<> m_cells[TvmStore::k_numberOfTvmVariables];
  TvmStore * m_store;
  ParametersController * m_parametersController;
};

}

#endif
