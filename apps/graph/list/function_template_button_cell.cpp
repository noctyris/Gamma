#include "function_template_button_cell.h"

namespace Graph {

FunctionTemplateButtonCell::FunctionTemplateButtonCell() :
  EvenOddCell(),
  m_color(Palette::PrimaryText)
{
}

void FunctionTemplateButtonCell::setColor(KDColor color) {
  if (m_color != color) {
    m_color = color;
    reloadCell();
  }
}

void FunctionTemplateButtonCell::drawRect(KDContext * ctx, KDRect rect) const {
  EvenOddCell::drawRect(ctx, rect);
  KDColor background = backgroundColor();
  KDColor separatorColor = m_even ? Palette::BackgroundApps : Palette::BackgroundHard;
  ctx->fillRect(KDRect(0, bounds().height()-Metric::CellSeparatorThickness, bounds().width(), Metric::CellSeparatorThickness), separatorColor);

  KDRect button((bounds().width() - k_buttonWidth) / 2,
                (bounds().height() - k_buttonHeight) / 2,
                k_buttonWidth, k_buttonHeight);
  // Draw a thin frame around the badge.
  ctx->fillRect(KDRect(button.x(), button.y(), button.width(), 1), m_color);
  ctx->fillRect(KDRect(button.x(), button.bottom(), button.width(), 1), m_color);
  ctx->fillRect(KDRect(button.x(), button.y()+1, 1, button.height()-2), m_color);
  ctx->fillRect(KDRect(button.right(), button.y()+1, 1, button.height()-2), m_color);

  const char * label = "fx";
  KDSize labelSize = KDFont::SmallFont->stringSize(label);
  KDPoint labelPosition(button.x() + (button.width() - labelSize.width()) / 2,
                        button.y() + (button.height() - labelSize.height()) / 2);
  ctx->drawString(label, labelPosition, KDFont::SmallFont, m_color, background);
}

KDSize FunctionTemplateButtonCell::minimalSizeForOptimalDisplay() const {
  return KDSize(k_buttonWidth, Metric::StoreRowHeight);
}

}
