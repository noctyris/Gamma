#ifndef SHARED_FUNCTION_EQUATION_H
#define SHARED_FUNCTION_EQUATION_H

#include <poincare/context.h>
#include <poincare/expression.h>

namespace Shared {

namespace FunctionEquation {

struct Point2D {
  double x;
  double y;
};

/* Latest Epsilon accepts the piecewise syntax
 * "piecewise(-x,x<0,x,x>=0)" and the brace notation
 * "{-x,x<0\\x,x>=0}". Gamma's Poincare parser has no comparison or
 * piecewise nodes, so the graph application parses these forms itself. */
bool IsPiecewise(const char * text);
bool IsPoint(const char * text);
bool IsPointList(const char * text);
int NumberOfPoints(const char * text);
bool PointAtIndex(const char * text, int index, Point2D * point,
                  Poincare::Context * context);

/* Turn "(cos(t),sin(t))" into "[[cos(t)][sin(t)]]", the representation used
 * by Gamma for parametric functions. Returns false when the text is not a
 * tuple. */
bool ParametricTupleToMatrix(const char * text, char * buffer,
                             size_t bufferSize);

/* Strip a leading function assignment such as "f(x)=", "f(t)=" or
 * "r1(θ)=". Implicit equations such as "x+y+1=0" are left untouched. */
bool StripFunctionAssignment(const char * text, char * buffer,
                             size_t bufferSize);

class Piecewise {
 public:
  constexpr static int k_maxNumberOfBranches = 6;
  enum class Comparison : uint8_t {
    Equal,
    Inferior,
    InferiorEqual,
    Superior,
    SuperiorEqual
  };

  Piecewise() { reset(); }
  void reset();
  bool parse(const char * text, Poincare::Context * context);
  bool isParsed() const { return m_parsed; }
  bool isValid() const { return m_numberOfBranches > 0; }
  double evaluate(double x, Poincare::Context * context) const;

 private:
  struct Branch {
    Poincare::Expression expression;
    Poincare::Expression left;
    Poincare::Expression right;
    Comparison comparison;
    bool hasCondition;
  };
  bool conditionHolds(const Branch & branch, double x,
                      Poincare::Context * context) const;

  Branch m_branches[k_maxNumberOfBranches];
  int m_numberOfBranches;
  bool m_parsed;
};

}  // namespace FunctionEquation

}  // namespace Shared

#endif
