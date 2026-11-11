#include "graph_view.h"
#include "../app.h"
#include <assert.h>
#include <algorithm>

using namespace Shared;

namespace Graph {

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
      if (((x + py) & 1) == 0) {
        ctx->fillRect(KDRect(x, py, 1, 1), color);
      }
    }
  }
}

void GraphView::drawRect(KDContext * ctx, KDRect rect) const {
  FunctionGraphView::drawRect(ctx, rect);
  ContinuousFunctionStore * functionStore = App::app()->functionStore();
  const int activeFunctionsCount = functionStore->numberOfActiveFunctions();
  /* Draw the inequalities first so that the curves stay visible above them. */
  for (int i = 0; i < activeFunctionsCount; i++) {
    Ion::Storage::Record record = functionStore->activeRecordAtIndex(i);
    ExpiringPointer<ContinuousFunction> f = functionStore->modelForRecord(record);
    if (!f->isInequality()) {
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
    ContinuousFunctionCache * cch = functionStore->cacheAtIndex(i);
    Shared::ContinuousFunction::PlotType type = f->plotType();
    Poincare::Expression e = f->expressionReduced(context());
    if (e.isUndefined() || (
        type == Shared::ContinuousFunction::PlotType::Parametric &&
        e.childAtIndex(0).isUndefined() &&
        e.childAtIndex(1).isUndefined())) {
      continue;
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
      if (m_tangent && record == m_selectedRecord) {
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
}

}
