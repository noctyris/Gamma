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

QUIZ_CASE(graph_equations) {
  GlobalContext context;
  ContinuousFunctionStore store;

  // f(x)=x
  ContinuousFunction * cartesian = addFunction("f(x)=x", Cartesian, &store, &context);
  assertNear(cartesian->evaluateXYAtParameter(-2.0, &context).x2(), -2.0);
  assertNear(cartesian->evaluateXYAtParameter(3.0, &context).x2(), 3.0);
  store.removeAll();

  // f(x)={-x,x<0 \\ x,x>=0}
  ContinuousFunction * piecewise = addFunction(
      "f(x)={-x,x<0\\\\x,x>=0}", Cartesian, &store, &context);
  quiz_assert(piecewise->isPiecewise());
  assertNear(piecewise->evaluateXYAtParameter(-2.0, &context).x2(), 2.0);
  assertNear(piecewise->evaluateXYAtParameter(0.0, &context).x2(), 0.0);
  assertNear(piecewise->evaluateXYAtParameter(3.0, &context).x2(), 3.0);
  store.removeAll();

  // The function-style piecewise spelling from latest Epsilon.
  ContinuousFunction * piecewiseFunction = addFunction(
      "piecewise(-x,x<0,x,x>=0)", Cartesian, &store, &context);
  quiz_assert(piecewiseFunction->isPiecewise());
  assertNear(piecewiseFunction->evaluateXYAtParameter(-2.0, &context).x2(), 2.0);
  assertNear(piecewiseFunction->evaluateXYAtParameter(3.0, &context).x2(), 3.0);
  store.removeAll();

  // (2,3)
  ContinuousFunction * point = addFunction("(2,3)", Cartesian, &store, &context);
  quiz_assert(point->isPoint());
  quiz_assert(point->numberOfPoints() == 1);
  Coordinate2D<double> pointValue;
  quiz_assert(point->pointAtIndex(0, &pointValue, &context));
  assertNear(pointValue.x1(), 2.0);
  assertNear(pointValue.x2(), 3.0);
  store.removeAll();

  // {(2,3),(3,5)}
  ContinuousFunction * points = addFunction(
      "{(2,3),(3,5)}", Cartesian, &store, &context);
  quiz_assert(points->isPointList());
  quiz_assert(points->numberOfPoints() == 2);
  quiz_assert(points->pointAtIndex(1, &pointValue, &context));
  assertNear(pointValue.x1(), 3.0);
  assertNear(pointValue.x2(), 5.0);
  store.removeAll();

  // x+y+1=0
  ContinuousFunction * line = addFunction(
      "x+y+1=0", ContinuousFunction::PlotType::Implicit, &store, &context);
  assertNear(line->evaluateImplicitAt(2.0, -3.0, &context), 0.0);
  store.removeAll();

  // x+y<=0
  ContinuousFunction * implicitInequality = addFunction(
      "x+y<=0", ContinuousFunction::PlotType::Implicit, &store, &context);
  quiz_assert(implicitInequality->isInequality());
  assertNear(implicitInequality->evaluateImplicitAt(-1.0, -1.0, &context), -2.0);
  store.removeAll();

  // x=√(y)
  ContinuousFunction * inverse = addFunction(
      "x=√(y)", ContinuousFunction::PlotType::Implicit, &store, &context);
  assertNear(inverse->evaluateImplicitAt(2.0, 4.0, &context), 0.0);
  store.removeAll();

  // x²+y²+xy+x+y=0
  ContinuousFunction * conic = addFunction(
      "x^2+y^2+x*y+x+y=0", ContinuousFunction::PlotType::Implicit, &store,
      &context);
  assertNear(conic->evaluateImplicitAt(0.0, 0.0, &context), 0.0);
  store.removeAll();

  // f(t)=(cos(t),sin(t))
  ContinuousFunction * parametric = addFunction(
      "f(t)=(cos(t),sin(t))", Parametric, &store, &context);
  quiz_assert(parametric->plotType() == Parametric);
  quiz_assert(!parametric->isScatter());
  Coordinate2D<double> parametricValue =
      parametric->evaluate2DAtParameter(0.0, &context);
  assertNear(parametricValue.x1(), 1.0);
  assertNear(parametricValue.x2(), 0.0);
  store.removeAll();

  // r1(θ)=cos(θ)
  ContinuousFunction * polar = addFunction(
      "r1(θ)=cos(θ)", Polar, &store, &context);
  Coordinate2D<double> polarValue = polar->evaluateXYAtParameter(0.0, &context);
  assertNear(polarValue.x1(), 1.0);
  assertNear(polarValue.x2(), 0.0);
  store.removeAll();
}

}
