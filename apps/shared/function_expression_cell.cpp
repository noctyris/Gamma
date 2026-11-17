#include "function_expression_cell.h"
#include <assert.h>
#include <algorithm>

namespace Shared {

FunctionExpressionCell::FunctionExpressionCell() :
  EvenOddExpressionCell(),
  m_prefixView(0.0f, 0.5f, Palette::PrimaryText, Palette::BackgroundHard),
  m_suffixView(0.0f, 0.5f, Palette::PrimaryText, Palette::BackgroundHard),
  m_textColor(Palette::PrimaryText)
{
}

void FunctionExpressionCell::setPrefix(Poincare::Layout prefix) {
  m_prefixView.setLayout(prefix);
  updateAuxiliaryViewColors();
  layoutSubviews();
  reloadCell();
}

void FunctionExpressionCell::setSuffix(Poincare::Layout suffix) {
  m_suffixView.setLayout(suffix);
  updateAuxiliaryViewColors();
  layoutSubviews();
  reloadCell();
}

void FunctionExpressionCell::updateAuxiliaryViewColors() {
  m_prefixView.setBackgroundColor(backgroundColor());
  m_suffixView.setBackgroundColor(backgroundColor());
  m_prefixView.setTextColor(m_textColor);
  m_suffixView.setTextColor(m_textColor);
}

void FunctionExpressionCell::setBackgroundColor(KDColor backgroundColor) {
  EvenOddExpressionCell::setBackgroundColor(backgroundColor);
  m_prefixView.setBackgroundColor(backgroundColor);
  m_suffixView.setBackgroundColor(backgroundColor);
}

void FunctionExpressionCell::setTextColor(KDColor textColor) {
  m_textColor = textColor;
  EvenOddExpressionCell::setTextColor(textColor);
  m_prefixView.setTextColor(textColor);
  m_suffixView.setTextColor(textColor);
}

void FunctionExpressionCell::setEven(bool even) {
  EvenOddExpressionCell::setEven(even);
  updateAuxiliaryViewColors();
}

void FunctionExpressionCell::setHighlighted(bool highlight) {
  EvenOddExpressionCell::setHighlighted(highlight);
  updateAuxiliaryViewColors();
}

KDSize FunctionExpressionCell::minimalSizeForOptimalDisplay() const {
  KDSize prefixSize = m_prefixView.minimalSizeForOptimalDisplay();
  KDSize expressionSize = m_expressionView.minimalSizeForOptimalDisplay();
  KDSize suffixSize = m_suffixView.minimalSizeForOptimalDisplay();
  KDCoordinate width = m_leftMargin + prefixSize.width() + expressionSize.width() + suffixSize.width() + m_rightMargin;
  KDCoordinate height = std::max(expressionSize.height(), std::max(prefixSize.height(), suffixSize.height()));
  return KDSize(width, height + k_separatorThickness);
}

void FunctionExpressionCell::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(rect, backgroundColor());
  // Color the horizontal separator
  KDColor separatorColor = m_even ? Palette::BackgroundApps : Palette::BackgroundHard;
  ctx->fillRect(KDRect(0, bounds().height()-k_separatorThickness, bounds().width(), k_separatorThickness), separatorColor);
}

int FunctionExpressionCell::numberOfSubviews() const {
  return 3;
}

View * FunctionExpressionCell::subviewAtIndex(int index) {
  if (index == 0) {
    return &m_prefixView;
  }
  if (index == 1) {
    return &m_expressionView;
  }
  assert(index == 2);
  return &m_suffixView;
}

void FunctionExpressionCell::layoutSubviews(bool force) {
  KDCoordinate height = bounds().height() - k_separatorThickness;
  KDSize prefixSize = m_prefixView.minimalSizeForOptimalDisplay();
  KDSize expressionSize = m_expressionView.minimalSizeForOptimalDisplay();
  KDSize suffixSize = m_suffixView.minimalSizeForOptimalDisplay();
  KDCoordinate prefixWidth = m_prefixView.layout().isUninitialized() ? 0 : prefixSize.width();
  KDCoordinate suffixWidth = m_suffixView.layout().isUninitialized() ? 0 : suffixSize.width();
  KDCoordinate expressionX = m_leftMargin + prefixWidth;
  KDCoordinate availableWidth = bounds().width() - expressionX - m_rightMargin;
  KDCoordinate expressionWidth = std::min(availableWidth, expressionSize.width());
  if (expressionWidth < 0) {
    expressionWidth = 0;
  }
  KDCoordinate suffixX = expressionX + expressionWidth;
  if (suffixX + suffixWidth > bounds().width() - m_rightMargin) {
    suffixWidth = std::max<KDCoordinate>(0, bounds().width() - m_rightMargin - suffixX);
  }
  m_prefixView.setFrame(KDRect(m_leftMargin, 0, prefixWidth, height), force);
  m_expressionView.setFrame(KDRect(expressionX, 0, expressionWidth, height), force);
  m_suffixView.setFrame(KDRect(suffixX, 0, suffixWidth, height), force);
}

}
