#ifndef GRAPH_FUNCTION_TEMPLATE_BUTTON_CELL_H
#define GRAPH_FUNCTION_TEMPLATE_BUTTON_CELL_H

#include <escher.h>

namespace Graph {

class FunctionTemplateButtonCell : public EvenOddCell {
public:
  FunctionTemplateButtonCell();
  void setColor(KDColor color);
  void drawRect(KDContext * ctx, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() const override;
private:
  constexpr static KDCoordinate k_buttonWidth = 22;
  constexpr static KDCoordinate k_buttonHeight = 16;
  KDColor m_color;
};

}

#endif
