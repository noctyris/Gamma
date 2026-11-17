#include <quiz.h>
#include "helper.h"
#include <cmath>

using namespace Poincare;
using namespace Shared;

namespace Graph {

namespace {

void assertNear(double actual, double expected) {
  quiz_assert(std::isfinite(actual));
  quiz_assert(std::abs(actual - expected) < 1e-6);
}

}

QUIZ_CASE(graph_implicit) {
  GlobalContext context;
  ContinuousFunctionStore store;

  ContinuousFunction * line = addFunction("x+y+1=0", ContinuousFunction::PlotType::Implicit, &store, &context);
  quiz_assert(line->isImplicit());
  assertNear(line->evaluateImplicitAt(0.0, -1.0, &context), 0.0);
  assertNear(line->evaluateImplicitAt(2.0, -3.0, &context), 0.0);
  assertNear(line->evaluateImplicitAt(0.0, 0.0, &context), 1.0);
  store.removeAll();

  ContinuousFunction * conic = addFunction("x^2+y^2=1", ContinuousFunction::PlotType::Implicit, &store, &context);
  assertNear(conic->evaluateImplicitAt(1.0, 0.0, &context), 0.0);
  /* The reusable evaluator must keep working when x and y are mutated between
   * evaluations. */
  ContinuousFunction::ImplicitEvaluator evaluator(conic, &context);
  assertNear(evaluator.evaluate(1.0f, 0.0f), 0.0);
  assertNear(evaluator.evaluate(0.0f, 1.0f), 0.0);
  assertNear(evaluator.evaluate(0.0f, 0.0f), -1.0);
  store.removeAll();

  /* Typing an equation in a Cartesian function must switch it to an implicit
   * plot automatically. */
  ContinuousFunction * typedConic = addFunction("x^2+y^2=1", Cartesian, &store, &context);
  quiz_assert(typedConic->isImplicit());
  assertNear(typedConic->evaluateImplicitAt(0.0, 1.0, &context), 0.0);

  /* The exact rotated conic must switch to implicit when typed in a regular
   * Cartesian row, not only when the implicit template is selected. */
  ContinuousFunction * typedRotatedConic = addFunction(
      "x^2+y^2+x*y+x+y=0", Cartesian, &store, &context);
  quiz_assert(typedRotatedConic->isImplicit());
  assertNear(typedRotatedConic->evaluateImplicitAt(0.0, -1.0, &context), 0.0);
  float xMin, xMax, yMin, yMax;
  typedRotatedConic->rangeForDisplay(&xMin, &xMax, &yMin, &yMax, 0.8f,
                                     &context);
  quiz_assert(std::isfinite(xMin) && std::isfinite(xMax) &&
              std::isfinite(yMin) && std::isfinite(yMax));
  assertNear(typedConic->evaluateImplicitAt(0.0, 0.0, &context), -1.0);
  assertNear(typedConic->evaluateImplicitAt(0.6, 0.8, &context), 0.0);
  store.removeAll();

  /* A bare expression depending on y is implicit as well: it must not keep a
   * meaningless f(x)= prefix. */
  ContinuousFunction * typedBare = addFunction("x+y", Cartesian, &store, &context);
  quiz_assert(typedBare->isImplicit());
  assertNear(typedBare->evaluateImplicitAt(1.0, -1.0, &context), 0.0);
  store.removeAll();

  /* Removing y from an implicit expression turns it back into f(x). */
  ContinuousFunction * typedFunction = addFunction("x^2", ContinuousFunction::PlotType::Implicit, &store, &context);
  quiz_assert(!typedFunction->isImplicit());
  store.removeAll();

  ContinuousFunction * inverse = addFunction("x=√(y)", ContinuousFunction::PlotType::Implicit, &store, &context);
  assertNear(inverse->evaluateImplicitAt(2.0, 4.0, &context), 0.0);
  assertNear(inverse->evaluateImplicitAt(0.0, 0.0, &context), 0.0);
  assertNear(inverse->evaluateImplicitAt(0.1, 0.01, &context), 0.0);
  assertNear(inverse->evaluateImplicitAt(0.1, 0.0, &context), 0.1);
  quiz_assert(!std::isfinite(inverse->evaluateImplicitAt(2.0, -4.0, &context)));
  store.removeAll();

  /* Applying the conic template to an existing parametric row goes through
   * an empty content and a plot-type change, like the template menu. */
  ContinuousFunction * conicAfterParametric = addFunction(
      "f(t)=(cos(t),sin(t))", Parametric, &store, &context);
  const Preferences::AngleUnit angleUnit =
      Preferences::sharedPreferences()->angleUnit();
  conicAfterParametric->setContent("", &context);
  conicAfterParametric->setPlotType(ContinuousFunction::PlotType::Implicit,
                                    angleUnit, &context);
  conicAfterParametric->setContent("x^2+y^2+x*y+x+y=0", &context);
  quiz_assert(conicAfterParametric->isImplicit());
  assertNear(conicAfterParametric->evaluateImplicitAt(0.0, -1.0, &context),
             0.0);
  store.removeAll();

  ContinuousFunction * inequality = addFunction("x+y", ContinuousFunction::PlotType::Implicit, &store, &context);
  inequality->setInequality(ContinuousFunction::Inequality::InferiorEqual);
  quiz_assert(inequality->isInequality());
  assertNear(inequality->evaluateImplicitAt(-1.0, -1.0, &context), -2.0);
  assertNear(inequality->evaluateImplicitAt(1.0, 1.0, &context), 2.0);
  store.removeAll();
}

}
