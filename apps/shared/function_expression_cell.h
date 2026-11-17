#ifndef SHARED_FUNCTION_EXPRESSION_CELL_H
#define SHARED_FUNCTION_EXPRESSION_CELL_H

#include <escher.h>

namespace Shared {

class FunctionExpressionCell : public EvenOddExpressionCell {
public:
  FunctionExpressionCell();
  void setPrefix(Poincare::Layout prefix);
  void setSuffix(Poincare::Layout suffix);
  void setBackgroundColor(KDColor backgroundColor);
  void setTextColor(KDColor textColor);
  void setEven(bool even) override;
  void setHighlighted(bool highlight) override;
  KDSize minimalSizeForOptimalDisplay() const override;
  void drawRect(KDContext * ctx, KDRect rect) const override;
  void layoutSubviews(bool force = false) override;
private:
  constexpr static KDCoordinate k_separatorThickness = Metric::CellSeparatorThickness;
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void updateAuxiliaryViewColors();
  ExpressionView m_prefixView;
  ExpressionView m_suffixView;
  KDColor m_textColor;
};

}

#endif
