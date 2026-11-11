#ifndef GRAPH_FUNCTION_MODELS_PARAMETER_CONTROLLER_H
#define GRAPH_FUNCTION_MODELS_PARAMETER_CONTROLLER_H

#include <escher.h>

namespace Graph {

class ListController;

class FunctionModelsParameterController : public ViewController, public ListViewDataSource, public SelectableTableViewDataSource {
public:
  FunctionModelsParameterController(Responder * parentResponder, ListController * listController);
  const char * title() override;
  View * view() override { return &m_selectableTableView; }
  void viewWillAppear() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;

  // ListViewDataSource
  int numberOfRows() const override;
  KDCoordinate rowHeight(int j) override;
  KDCoordinate cumulatedHeightFromIndex(int j) override;
  int indexFromCumulatedHeight(KDCoordinate offsetY) override;
  HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  int typeAtLocation(int i, int j) override;

private:
  constexpr static int k_numberOfModels = 8;
  constexpr static int k_numberOfExpressionCells = k_numberOfModels - 1;
  static constexpr const char * k_modelDescriptions[k_numberOfExpressionCells] = {
    "f(x)=x", "r=f(θ)", "[[x][y]]=f(t)", "y<f(x)", "y≤f(x)", "y>f(x)", "y≥f(x)"
  };
  MessageTableCell<> m_emptyModelCell;
  ExpressionTableCell m_modelCells[k_numberOfExpressionCells];
  SelectableTableView m_selectableTableView;
  ListController * m_listController;
};

}

#endif
