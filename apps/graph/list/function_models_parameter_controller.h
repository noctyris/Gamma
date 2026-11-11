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
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;
  int typeAtLocation(int i, int j) override;

private:
  constexpr static int k_numberOfModels = 15;
  constexpr static int k_numberOfExpressionCells = k_numberOfModels - 1;
  constexpr static int k_numberOfReusableCells = 5;
  static constexpr const char * k_modelDescriptions[k_numberOfExpressionCells] = {
    "f(x)=x",
    "x+y+1=0",
    "x+y≤0",
    "x=√(y)",
    "f(x)={-x,x<0 \\\\ x,x≥0}",
    "(2,3)",
    "{(2,3),(3,5)}",
    "x^2+y^2+x*y+x+y=0",
    "f(t)=(cos(t),sin(t))",
    "r1(θ)=cos(θ)",
    "y<f(x)",
    "y≤f(x)",
    "y>f(x)",
    "y≥f(x)"
  };
  MessageTableCell<> m_emptyModelCell;
  ExpressionTableCell m_modelCells[k_numberOfReusableCells];
  SelectableTableView m_selectableTableView;
  ListController * m_listController;
};

}

#endif
