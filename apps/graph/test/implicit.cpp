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
  assertNear(conic->evaluateImplicitAt(0.0, 0.0, &context), -1.0);
  assertNear(conic->evaluateImplicitAt(0.6, 0.8, &context), 0.0);
  store.removeAll();

  ContinuousFunction * inverse = addFunction("x=√(y)", ContinuousFunction::PlotType::Implicit, &store, &context);
  assertNear(inverse->evaluateImplicitAt(2.0, 4.0, &context), 0.0);
  assertNear(inverse->evaluateImplicitAt(0.0, 0.0, &context), 0.0);
  quiz_assert(!std::isfinite(inverse->evaluateImplicitAt(2.0, -4.0, &context)));
  store.removeAll();

  ContinuousFunction * inequality = addFunction("x+y", ContinuousFunction::PlotType::Implicit, &store, &context);
  inequality->setInequality(ContinuousFunction::Inequality::InferiorEqual);
  quiz_assert(inequality->isInequality());
  assertNear(inequality->evaluateImplicitAt(-1.0, -1.0, &context), -2.0);
  assertNear(inequality->evaluateImplicitAt(1.0, 1.0, &context), 2.0);
  store.removeAll();
}

}
