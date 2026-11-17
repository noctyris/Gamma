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

void GraphView::drawImplicitSegment(KDContext * ctx, KDRect rect, float x1, float y1, float x2, float y2, KDColor color, bool dotted) const {
  if (!std::isfinite(x1) || !std::isfinite(y1) ||
      !std::isfinite(x2) || !std::isfinite(y2)) {
    return;
  }
  auto drawDot = [ctx, rect, color](float x, float y) {
    const KDCoordinate px = (KDCoordinate)std::round(x);
    const KDCoordinate py = (KDCoordinate)std::round(y);
    const KDRect dot(px - 1, py - 1, 2, 2);
    if (rect.intersects(dot)) {
      ctx->fillRect(dot, color);
    }
  };
  const float dx = x2 - x1;
  const float dy = y2 - y1;
  if (dotted) {
    const float length = std::max(std::fabs(dx), std::fabs(dy));
    if (length < 4.0f) {
      drawDot((x1 + x2) * 0.5f, (y1 + y2) * 0.5f);
      return;
    }
    const int steps = (int)std::ceil(length / 4.0f);
    for (int i = 0; i <= steps; i++) {
      const float t = (float)i / steps;
      drawDot(x1 + dx * t, y1 + dy * t);
    }
    return;
  }

  /* Draw the thick stroke as horizontal or vertical spans. Compared to the
   * antialiased stamp used by explicit curves, this avoids a pull/blend/push
   * for every pixel of the curve. On the device, pulling pixels back from the
   * display is the expensive part. */
  constexpr float k_strokeThickness = 2.5f;
  if (std::fabs(dy) < 0.01f) {
    const KDCoordinate left = std::max<KDCoordinate>((KDCoordinate)std::round(std::min(x1, x2)), rect.left());
    const KDCoordinate right = std::min<KDCoordinate>((KDCoordinate)std::round(std::max(x1, x2)), rect.right());
    const KDCoordinate top = std::max<KDCoordinate>((KDCoordinate)std::round(y1) - 1, rect.top());
    const KDCoordinate bottom = std::min<KDCoordinate>((KDCoordinate)std::round(y1), rect.bottom());
    if (left <= right && top <= bottom) {
      ctx->fillRect(KDRect(left, top, right - left + 1, bottom - top + 1), color);
    }
    return;
  }
  if (std::fabs(dx) < 0.01f) {
    const KDCoordinate left = std::max<KDCoordinate>((KDCoordinate)std::round(x1) - 1, rect.left());
    const KDCoordinate right = std::min<KDCoordinate>((KDCoordinate)std::round(x1), rect.right());
    const KDCoordinate top = std::max<KDCoordinate>((KDCoordinate)std::round(std::min(y1, y2)), rect.top());
    const KDCoordinate bottom = std::min<KDCoordinate>((KDCoordinate)std::round(std::max(y1, y2)), rect.bottom());
    if (left <= right && top <= bottom) {
      ctx->fillRect(KDRect(left, top, right - left + 1, bottom - top + 1), color);
    }
    return;
  }

  if (std::fabs(dx) >= std::fabs(dy)) {
    if (x2 < x1) {
      const float previousX = x1;
      const float previousY = y1;
      x1 = x2;
      y1 = y2;
      x2 = previousX;
      y2 = previousY;
    }
    const float slope = (y2 - y1) / (x2 - x1);
    const KDCoordinate span = (KDCoordinate)std::max(1.0f, std::round(k_strokeThickness * std::sqrt(1.0f + slope * slope)));
    const KDCoordinate xStart = std::max<KDCoordinate>((KDCoordinate)std::ceil(x1), rect.left());
    const KDCoordinate xEnd = std::min<KDCoordinate>((KDCoordinate)std::floor(x2), rect.right());
    if (xStart > xEnd) {
      drawDot((x1 + x2) * 0.5f, (y1 + y2) * 0.5f);
      return;
    }
    KDCoordinate runStart = xStart;
    KDCoordinate runTop = 0;
    KDCoordinate runHeight = 0;
    bool hasRun = false;
    for (KDCoordinate x = xStart; x <= xEnd; x++) {
      const float y = y1 + slope * (x - x1);
      KDCoordinate top = (KDCoordinate)std::round(y) - span / 2;
      KDCoordinate bottom = top + span - 1;
      if (top < rect.top()) {
        top = rect.top();
      }
      if (bottom > rect.bottom()) {
        bottom = rect.bottom();
      }
      if (bottom < top) {
        if (hasRun) {
          ctx->fillRect(KDRect(runStart, runTop, x - runStart, runHeight), color);
          hasRun = false;
        }
        continue;
      }
      const KDCoordinate height = bottom - top + 1;
      if (!hasRun) {
        runStart = x;
        runTop = top;
        runHeight = height;
        hasRun = true;
      } else if (top != runTop || height != runHeight) {
        ctx->fillRect(KDRect(runStart, runTop, x - runStart, runHeight), color);
        runStart = x;
        runTop = top;
        runHeight = height;
      }
    }
    if (hasRun) {
      ctx->fillRect(KDRect(runStart, runTop, xEnd - runStart + 1, runHeight), color);
    }
  } else {
    if (y2 < y1) {
      const float previousX = x1;
      const float previousY = y1;
      x1 = x2;
      y1 = y2;
      x2 = previousX;
      y2 = previousY;
    }
    const float slope = (x2 - x1) / (y2 - y1);
    const KDCoordinate span = (KDCoordinate)std::max(1.0f, std::round(k_strokeThickness * std::sqrt(1.0f + slope * slope)));
    const KDCoordinate yStart = std::max<KDCoordinate>((KDCoordinate)std::ceil(y1), rect.top());
    const KDCoordinate yEnd = std::min<KDCoordinate>((KDCoordinate)std::floor(y2), rect.bottom());
    if (yStart > yEnd) {
      drawDot((x1 + x2) * 0.5f, (y1 + y2) * 0.5f);
      return;
    }
    KDCoordinate runStart = yStart;
    KDCoordinate runLeft = 0;
    KDCoordinate runWidth = 0;
    bool hasRun = false;
    for (KDCoordinate y = yStart; y <= yEnd; y++) {
      const float x = x1 + slope * (y - y1);
      KDCoordinate left = (KDCoordinate)std::round(x) - span / 2;
      KDCoordinate right = left + span - 1;
      if (left < rect.left()) {
        left = rect.left();
      }
      if (right > rect.right()) {
        right = rect.right();
      }
      if (right < left) {
        if (hasRun) {
          ctx->fillRect(KDRect(runLeft, runStart, runWidth, y - runStart), color);
          hasRun = false;
        }
        continue;
      }
      const KDCoordinate width = right - left + 1;
      if (!hasRun) {
        runStart = y;
        runLeft = left;
        runWidth = width;
        hasRun = true;
      } else if (left != runLeft || width != runWidth) {
        ctx->fillRect(KDRect(runLeft, runStart, runWidth, y - runStart), color);
        runStart = y;
        runLeft = left;
        runWidth = width;
      }
    }
    if (hasRun) {
      ctx->fillRect(KDRect(runLeft, runStart, runWidth, yEnd - runStart + 1), color);
    }
  }
}

void GraphView::drawImplicitCell(KDContext * ctx, KDRect rect,
    void * evaluatorPointer, KDCoordinate x, KDCoordinate y, KDCoordinate size,
    float fTL, float fTR, float fBR, float fBL, int inequalityValue,
    KDColor areaColor, KDColor boundaryColor, bool strict) const {
  ContinuousFunction::ImplicitEvaluator & evaluator =
      *static_cast<ContinuousFunction::ImplicitEvaluator *>(evaluatorPointer);
  ContinuousFunction::Inequality inequality =
      static_cast<ContinuousFunction::Inequality>(inequalityValue);
  const bool finiteTL = std::isfinite(fTL);
  const bool finiteTR = std::isfinite(fTR);
  const bool finiteBR = std::isfinite(fBR);
  const bool finiteBL = std::isfinite(fBL);
  if (!finiteTL && !finiteTR && !finiteBR && !finiteBL) {
    return;
  }

  /* The pixel-to-graph mapping is affine. Keep it here so that mixed edges
   * can be sampled at sub-pixel positions instead of rounding every probe. */
  const float xPixelToFloat = pixelToFloat(Axis::Horizontal, 1) -
                              pixelToFloat(Axis::Horizontal, 0);
  const float yPixelToFloat = pixelToFloat(Axis::Vertical, 1) -
                              pixelToFloat(Axis::Vertical, 0);
  const float xGraphOrigin = pixelToFloat(Axis::Horizontal, 0);
  const float yGraphOrigin = pixelToFloat(Axis::Vertical, 0);
  auto evaluateAt = [&](float px, float py) {
    return evaluator.evaluate(xGraphOrigin + px * xPixelToFloat,
                              yGraphOrigin + py * yPixelToFloat);
  };

  struct Crossing { float x; float y; };
  Crossing crossings[6];
  int numberOfCrossings = 0;
  auto addCrossingAt = [&](float px, float py) {
    if (numberOfCrossings < 6) {
      crossings[numberOfCrossings++] = {px, py};
    }
  };
  auto addFiniteEdge = [&](float a, float b, float ax, float ay, float bx, float by) {
    if (!std::isfinite(a) || !std::isfinite(b) ||
        (a > 0.0f) == (b > 0.0f) || a == b) {
      return;
    }
    const float t = a / (a - b);
    addCrossingAt(ax + (bx - ax) * t, ay + (by - ay) * t);
  };

  /* One endpoint is finite and the other is undefined. Scan the defined part
   * of the edge for a root and, if the function tends to zero at the domain
   * boundary, add that boundary point. This is what closes x=√(y) at 0. */
  auto addMixedEdge = [&](float finiteValue, float ax, float ay,
                          float bx, float by) {
    constexpr int k_samples = 6;
    float previousT = 0.0f;
    float previousValue = finiteValue;
    bool foundRoot = false;
    for (int i = 1; i <= k_samples; i++) {
      const float t = (float)i / k_samples;
      const float px = ax + (bx - ax) * t;
      const float py = ay + (by - ay) * t;
      const float value = evaluateAt(px, py);
      if (std::isfinite(value)) {
        if ((previousValue > 0.0f) != (value > 0.0f) &&
            previousValue != value) {
          const float rootT = previousT + (t - previousT) *
              (previousValue / (previousValue - value));
          addCrossingAt(ax + (bx - ax) * rootT, ay + (by - ay) * rootT);
          foundRoot = true;
        }
        previousT = t;
        previousValue = value;
        continue;
      }
      /* Find the last finite point before the undefined region. */
      float finiteT = previousT;
      float finiteAtBoundary = previousValue;
      float undefinedT = t;
      for (int iteration = 0; iteration < 6; iteration++) {
        const float middleT = (finiteT + undefinedT) * 0.5f;
        const float middleValue = evaluateAt(ax + (bx - ax) * middleT,
                                             ay + (by - ay) * middleT);
        if (std::isfinite(middleValue)) {
          finiteT = middleT;
          finiteAtBoundary = middleValue;
        } else {
          undefinedT = middleT;
        }
      }
      if (!foundRoot &&
          std::fabs(finiteAtBoundary) <
              0.25f * std::fabs(finiteValue) + 1e-6f) {
        addCrossingAt(ax + (bx - ax) * finiteT,
                      ay + (by - ay) * finiteT);
      }
      break;
    }
  };

  /* Top edge. */
  if (finiteTL && finiteTR) {
    addFiniteEdge(fTL, fTR, x, y, x + size, y);
  } else if (finiteTL) {
    addMixedEdge(fTL, x, y, x + size, y);
  } else if (finiteTR) {
    addMixedEdge(fTR, x + size, y, x, y);
  }
  /* Right edge. */
  if (finiteTR && finiteBR) {
    addFiniteEdge(fTR, fBR, x + size, y, x + size, y + size);
  } else if (finiteTR) {
    addMixedEdge(fTR, x + size, y, x + size, y + size);
  } else if (finiteBR) {
    addMixedEdge(fBR, x + size, y + size, x + size, y);
  }
  /* Bottom edge. */
  if (finiteBR && finiteBL) {
    addFiniteEdge(fBR, fBL, x + size, y + size, x, y + size);
  } else if (finiteBR) {
    addMixedEdge(fBR, x + size, y + size, x, y + size);
  } else if (finiteBL) {
    addMixedEdge(fBL, x, y + size, x + size, y + size);
  }
  /* Left edge. */
  if (finiteBL && finiteTL) {
    addFiniteEdge(fBL, fTL, x, y + size, x, y);
  } else if (finiteBL) {
    addMixedEdge(fBL, x, y + size, x, y);
  } else if (finiteTL) {
    addMixedEdge(fTL, x, y, x, y + size);
  }

  float center = 0.0f;
  int finiteCorners = 0;
  if (finiteTL) { center += fTL; finiteCorners++; }
  if (finiteTR) { center += fTR; finiteCorners++; }
  if (finiteBR) { center += fBR; finiteCorners++; }
  if (finiteBL) { center += fBL; finiteCorners++; }
  center /= finiteCorners;
  if (inequality != ContinuousFunction::Inequality::None &&
      IsInImplicitRegion(center, inequality)) {
    ctx->fillRect(KDRect(x + size / 2 - 1, y + size / 2 - 1, 2, 2), areaColor);
  }

  auto drawBoundary = [&](const Crossing & a, const Crossing & b) {
    drawImplicitSegment(ctx, rect, a.x, a.y, b.x, b.y, boundaryColor, strict);
  };
  if (numberOfCrossings == 2) {
    drawBoundary(crossings[0], crossings[1]);
  } else if (numberOfCrossings == 4) {
    /* Use the cell center as the asymptotic decider: it selects the right
     * pairing for saddle cells instead of always connecting the same
     * corners. */
    if ((center > 0.0f) == (fTL > 0.0f)) {
      drawBoundary(crossings[0], crossings[1]);
      drawBoundary(crossings[2], crossings[3]);
    } else {
      drawBoundary(crossings[0], crossings[3]);
      drawBoundary(crossings[1], crossings[2]);
    }
  }
}

void GraphView::drawImplicit(KDContext * ctx, KDRect rect, ContinuousFunction * function) const {
  /* The previous 4 px grid sampled the whole view. An 8 px grid is four times
   * cheaper and the sub-pixel interpolation keeps the thick contour smooth. */
  constexpr KDCoordinate step = 8;
  constexpr int k_maxNumberOfColumns = Ion::Display::Width / step + 3;
  const int numberOfColumns = (rect.width() + step - 1) / step;
  const int numberOfRows = (rect.height() + step - 1) / step;
  if (numberOfColumns <= 0 || numberOfRows <= 0) {
    return;
  }
  assert(numberOfColumns + 1 <= k_maxNumberOfColumns);
  const ContinuousFunction::Inequality inequality = function->inequality();
  const bool strict = inequality == ContinuousFunction::Inequality::Inferior || inequality == ContinuousFunction::Inequality::Superior;
  const KDColor areaColor = KDColor::blend(function->color(), KDColorWhite, 0x60);
  const KDColor boundaryColor = function->color();

  /* Reuse the same context for the whole grid: x and y are mutated in place
   * instead of allocating two Poincare nodes for every sample. */
  ContinuousFunction::ImplicitEvaluator evaluator(function, context());
  auto xAtColumn = [this, rect](int column) {
    return pixelToFloat(Axis::Horizontal, rect.left() + column * step);
  };
  auto yAtRow = [this, rect](int row) {
    return pixelToFloat(Axis::Vertical, rect.top() + row * step);
  };

  float previousRow[k_maxNumberOfColumns];
  float currentRow[k_maxNumberOfColumns];
  for (int column = 0; column <= numberOfColumns; column++) {
    previousRow[column] = evaluator.evaluate(xAtColumn(column), yAtRow(0));
  }

  for (int row = 0; row < numberOfRows; row++) {
    for (int column = 0; column <= numberOfColumns; column++) {
      currentRow[column] = evaluator.evaluate(xAtColumn(column), yAtRow(row + 1));
    }
    const KDCoordinate y = rect.top() + row * step;
    for (int column = 0; column < numberOfColumns; column++) {
      const KDCoordinate x = rect.left() + column * step;
      drawImplicitCell(ctx, rect, &evaluator, x, y, step,
                       previousRow[column], previousRow[column + 1],
                       currentRow[column + 1], currentRow[column],
                       static_cast<int>(inequality), areaColor, boundaryColor,
                       strict);
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
