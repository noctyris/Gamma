#include "graph_view.h"
#include "../app.h"
#include <assert.h>
#include <ion/display.h>
#include <string.h>
#include <algorithm>
#include <cmath>

using namespace Shared;

namespace Graph {

namespace {

bool IsInImplicitRegion(double value, Shared::ContinuousFunction::Inequality inequality) {
  switch (inequality) {
  case Shared::ContinuousFunction::Inequality::Inferior:
    return value < 0.0;
  case Shared::ContinuousFunction::Inequality::InferiorEqual:
    return value <= 0.0;
  case Shared::ContinuousFunction::Inequality::Superior:
    return value > 0.0;
  case Shared::ContinuousFunction::Inequality::SuperiorEqual:
    return value >= 0.0;
  default:
    return false;
  }
}

}

GraphView::GraphView(InteractiveCurveViewRange * graphRange,
  CurveViewCursor * cursor, Shared::BannerView * bannerView, CursorView * cursorView) :
  FunctionGraphView(graphRange, cursor, bannerView, cursorView),
  m_tangent(false)
{
}

void GraphView::reload() {
  if (m_tangent) {
    KDRect dirtyZone(KDRect(0, 0, bounds().width(), bounds().height()-m_bannerView->bounds().height()));
    markRectAsDirty(dirtyZone);
  }
  return FunctionGraphView::reload();
}

void GraphView::drawInequalityArea(KDContext * ctx, KDRect rect, ContinuousFunction * function) const {
  ContinuousFunction::Inequality inequality = function->inequality();
  if (inequality == ContinuousFunction::Inequality::None) {
    return;
  }
  // Draw a light checkerboard pattern instead of a plain fill.
  KDColor color = KDColor::blend(function->color(), KDColorWhite, 0x60);
  for (KDCoordinate x = rect.left(); x <= rect.right(); x++) {
    float xValue = pixelToFloat(Axis::Horizontal, x);
    double yValue = function->evaluateXYAtParameter(xValue, context()).x2();
    float yPixel = floatToPixel(Axis::Vertical, yValue);
    if (!std::isfinite(yValue) || !std::isfinite(yPixel)) {
      continue;
    }
    KDCoordinate y = (KDCoordinate)std::round(yPixel);
    KDCoordinate start = rect.top();
    KDCoordinate end = rect.bottom();
    if (inequality == ContinuousFunction::Inequality::Inferior || inequality == ContinuousFunction::Inequality::InferiorEqual) {
      // y < f(x): shade below the curve, ie at larger pixel ordinates.
      start = std::max(start, y);
    } else {
      end = std::min(end, y);
    }
    for (KDCoordinate py = start; py <= end; py++) {
      /* Keep the fill light, like the latest Epsilon pattern. */
      if (((x + 2 * py) & 7) == 0) {
        ctx->fillRect(KDRect(x, py, 1, 1), color);
      }
    }
  }
}

void GraphView::drawImplicit(KDContext * ctx, KDRect rect, ContinuousFunction * function) const {
  constexpr KDCoordinate step = 4;
  /* Sample the implicit expression on a grid and reuse each sample for the
   * four neighbouring cells. This divides the number of expression
   * evaluations by four compared to evaluating every cell corner. */
  constexpr int k_maxNumberOfSamples = Ion::Display::Width / step + 2;
  const int numberOfColumns = rect.width() / step;
  const int numberOfRows = rect.height() / step;
  if (numberOfColumns <= 0 || numberOfRows <= 0) {
    return;
  }
  assert(numberOfColumns + 1 <= k_maxNumberOfSamples);
  const ContinuousFunction::Inequality inequality = function->inequality();
  const bool strict = inequality == ContinuousFunction::Inequality::Inferior || inequality == ContinuousFunction::Inequality::Superior;
  const KDColor areaColor = KDColor::blend(function->color(), KDColorWhite, 0x60);
  const KDColor boundaryColor = function->color();

  /* Use the same thick, antialiased stroke as the explicit curves instead of
   * drawing one-pixel marching-squares segments. */
  auto drawSolidBoundary = [this, ctx, rect, boundaryColor](float x1, float y1, float x2, float y2) {
    drawSegment(ctx, rect,
        pixelToFloat(Axis::Horizontal, x1), pixelToFloat(Axis::Vertical, y1),
        pixelToFloat(Axis::Horizontal, x2), pixelToFloat(Axis::Vertical, y2),
        boundaryColor, true);
  };
  auto drawDottedBoundary = [this, ctx, rect, boundaryColor](float x1, float y1, float x2, float y2) {
    const int steps = (int)std::max(std::abs(x2 - x1), std::abs(y2 - y1));
    if (steps <= 0) {
      return;
    }
    for (int i = 0; i <= steps; i += 2) {
      const float x = x1 + (x2 - x1) * i / steps;
      const float y = y1 + (y2 - y1) * i / steps;
      drawDot(ctx, rect,
          pixelToFloat(Axis::Horizontal, x), pixelToFloat(Axis::Vertical, y),
          boundaryColor, Size::Tiny);
    }
  };

  float previousRow[k_maxNumberOfSamples];
  float currentRow[k_maxNumberOfSamples];

  auto xAtColumn = [this, rect](int column) {
    return pixelToFloat(Axis::Horizontal, rect.left() + column * step);
  };
  auto yAtRow = [this, rect](int row) {
    return pixelToFloat(Axis::Vertical, rect.top() + row * step);
  };

  for (int column = 0; column <= numberOfColumns; column++) {
    previousRow[column] = (float)function->evaluateImplicitAt(
        xAtColumn(column), yAtRow(0), context());
  }

  for (int row = 0; row < numberOfRows; row++) {
    for (int column = 0; column <= numberOfColumns; column++) {
      currentRow[column] = (float)function->evaluateImplicitAt(
          xAtColumn(column), yAtRow(row + 1), context());
    }
    const KDCoordinate y = rect.top() + row * step;
    for (int column = 0; column < numberOfColumns; column++) {
      const KDCoordinate x = rect.left() + column * step;
      const float fTL = previousRow[column];
      const float fTR = previousRow[column + 1];
      const float fBR = currentRow[column + 1];
      const float fBL = currentRow[column];
      if (!std::isfinite(fTL) || !std::isfinite(fTR) ||
          !std::isfinite(fBR) || !std::isfinite(fBL)) {
        continue;
      }

      if (inequality != ContinuousFunction::Inequality::None) {
        const float center = (fTL + fTR + fBR + fBL) * 0.25f;
        if (IsInImplicitRegion(center, inequality)) {
          ctx->fillRect(KDRect(x + step / 2, y + step / 2, 1, 1), areaColor);
        }
      }

      struct Crossing { float x; float y; };
      Crossing crossings[4];
      int numberOfCrossings = 0;
      auto addCrossing = [&](float a, float b, float ax, float ay, float bx, float by) {
        if ((a > 0.0f) == (b > 0.0f) || a == b || numberOfCrossings >= 4) {
          return;
        }
        const float t = a / (a - b);
        crossings[numberOfCrossings++] = {ax + (bx - ax) * t, ay + (by - ay) * t};
      };
      addCrossing(fTL, fTR, x, y, x + step, y);
      addCrossing(fTR, fBR, x + step, y, x + step, y + step);
      addCrossing(fBR, fBL, x + step, y + step, x, y + step);
      addCrossing(fBL, fTL, x, y + step, x, y);

      auto drawBoundary = [&](const Crossing & a, const Crossing & b) {
        if (strict) {
          drawDottedBoundary(a.x, a.y, b.x, b.y);
        } else {
          drawSolidBoundary(a.x, a.y, b.x, b.y);
        }
      };
      if (numberOfCrossings == 2) {
        drawBoundary(crossings[0], crossings[1]);
      } else if (numberOfCrossings == 4) {
        drawBoundary(crossings[0], crossings[1]);
        drawBoundary(crossings[2], crossings[3]);
      }
    }
    memcpy(previousRow, currentRow, sizeof(float) * (numberOfColumns + 1));
  }
}

void GraphView::drawScatterPlot(KDContext * ctx, KDRect rect, ContinuousFunction * function) const {
  int numberOfPoints = function->numberOfPoints();
  for (int i = 0; i < numberOfPoints; i++) {
    Poincare::Coordinate2D<double> point;
    if (!function->pointAtIndex(i, &point, context())) {
      continue;
    }
    drawDot(ctx, rect, point.x1(), point.x2(), function->color(), Size::Small);
  }
}

void GraphView::drawRect(KDContext * ctx, KDRect rect) const {
  FunctionGraphView::drawRect(ctx, rect);
  ContinuousFunctionStore * functionStore = App::app()->functionStore();
  const int activeFunctionsCount = functionStore->numberOfActiveFunctions();
  /* Draw the implicit curves first so that the explicit curves stay visible. */
  for (int i = 0; i < activeFunctionsCount; i++) {
    Ion::Storage::Record record = functionStore->activeRecordAtIndex(i);
    ExpiringPointer<ContinuousFunction> f = functionStore->modelForRecord(record);
    if (f->isImplicit()) {
      drawImplicit(ctx, rect, f.operator->());
    }
  }
  /* Draw the inequalities first so that the curves stay visible above them. */
  for (int i = 0; i < activeFunctionsCount; i++) {
    Ion::Storage::Record record = functionStore->activeRecordAtIndex(i);
    ExpiringPointer<ContinuousFunction> f = functionStore->modelForRecord(record);
    if (f->isImplicit() || !f->isInequality()) {
      continue;
    }
    ContinuousFunctionCache * cch = functionStore->cacheAtIndex(i);
    float tmin = f->tMin();
    float rectLeft = pixelToFloat(Axis::Horizontal, rect.left() - k_externRectMargin);
    float tCacheMin = std::isnan(rectLeft) ? tmin : std::max(tmin, rectLeft);
    ContinuousFunctionCache::PrepareForCaching(f.operator->(), cch, tCacheMin, pixelWidth());
    drawInequalityArea(ctx, rect, f.operator->());
  }
  for (int i = 0; i < activeFunctionsCount ; i++) {
    Ion::Storage::Record record = functionStore->activeRecordAtIndex(i);
    ExpiringPointer<ContinuousFunction> f = functionStore->modelForRecord(record);
    if (f->isImplicit() || f->isScatter()) {
      continue;
    }
    ContinuousFunctionCache * cch = functionStore->cacheAtIndex(i);
    Shared::ContinuousFunction::PlotType type = f->plotType();
    if (!f->isPiecewise()) {
      Poincare::Expression e = f->expressionReduced(context());
      if (e.isUndefined() || (
          type == Shared::ContinuousFunction::PlotType::Parametric &&
          e.childAtIndex(0).isUndefined() &&
          e.childAtIndex(1).isUndefined())) {
        continue;
      }
    }
    float tmin = f->tMin();
    float tmax = f->tMax();

    float tCacheMin, tCacheStep, tStepNonCartesian;
    if (type == ContinuousFunction::PlotType::Cartesian) {
      float rectLeft = pixelToFloat(Axis::Horizontal, rect.left() - k_externRectMargin);
      /* Here, tCacheMin can depend on rect (and change as the user move)
       * because cache can be panned for cartesian curves, instead of being
       * entirely invalidated. */
      tCacheMin = std::isnan(rectLeft) ? tmin : std::max(tmin, rectLeft);
      tCacheStep = pixelWidth();
    } else {
      tCacheMin = tmin;
      // Compute tCacheStep and tStepNonCartesian
      ContinuousFunctionCache::ComputeNonCartesianSteps(&tStepNonCartesian, &tCacheStep, tmax, tmin);
    }
    ContinuousFunctionCache::PrepareForCaching(f.operator->(), cch, tCacheMin, tCacheStep);

    if (type == Shared::ContinuousFunction::PlotType::Cartesian) {
      // Cartesian
      drawCartesianCurve(ctx, rect, tmin, tmax, [](float t, void * model, void * context) {
            ContinuousFunction * f = (ContinuousFunction *)model;
            Poincare::Context * c = (Poincare::Context *)context;
            return f->evaluateXYAtParameter(t, c);
          }, f.operator->(), context(), f->color(), true, record == m_selectedRecord, m_highlightedStart, m_highlightedEnd,
          [](double t, void * model, void * context) {
            ContinuousFunction * f = (ContinuousFunction *)model;
            Poincare::Context * c = (Poincare::Context *)context;
            return f->evaluateXYAtParameter(t, c);
          });
      /* Draw tangent */
      if (m_tangent && !f->isPiecewise() && record == m_selectedRecord) {
        float tangentParameterA = f->approximateDerivative(m_curveViewCursor->x(), context());
        float tangentParameterB = -tangentParameterA*m_curveViewCursor->x()+f->evaluateXYAtParameter(m_curveViewCursor->x(), context()).x2();
        // To represent the tangent, we draw segment from and to abscissas at the extremity of the drawn rect
        float minAbscissa = pixelToFloat(Axis::Horizontal, rect.left());
        float maxAbscissa = pixelToFloat(Axis::Horizontal, rect.right());
        drawSegment(ctx, rect, minAbscissa, tangentParameterA*minAbscissa+tangentParameterB, maxAbscissa, tangentParameterA*maxAbscissa+tangentParameterB, Palette::GraphTangent, false);
      }
    } else if (type == Shared::ContinuousFunction::PlotType::Polar) {
      // Polar
      drawPolarCurve(ctx, rect, tmin, tmax, tStepNonCartesian, [](float t, void * model, void * context) {
            ContinuousFunction * f = (ContinuousFunction *)model;
            Poincare::Context * c = (Poincare::Context *)context;
            return f->evaluateXYAtParameter(t, c);
          }, f.operator->(), context(), false, f->color());
    } else {
      // Parametric
      assert(type == Shared::ContinuousFunction::PlotType::Parametric);
      drawCurve(ctx, rect, tmin, tmax, tStepNonCartesian, [](float t, void * model, void * context) {
          ContinuousFunction * f = (ContinuousFunction *)model;
          Poincare::Context * c = (Poincare::Context *)context;
          return f->evaluateXYAtParameter(t, c);
        }, f.operator->(), context(), false, f->color());
    }
  }
  /* Scatter plots are drawn last so that their points stay visible above the
   * curves. */
  for (int i = 0; i < activeFunctionsCount; i++) {
    Ion::Storage::Record record = functionStore->activeRecordAtIndex(i);
    ExpiringPointer<ContinuousFunction> f = functionStore->modelForRecord(record);
    if (f->isScatter()) {
      drawScatterPlot(ctx, rect, f.operator->());
    }
  }
}

}
