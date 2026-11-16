#include "helper.h"

using namespace Poincare;

namespace {

/* These vectors are deliberately small and explicit. They are not generated
 * from the implementation: changing one is a reviewed behaviour change. */
struct GoldenCase {
  const char * input;
  const char * simplified;
  const char * serialized;
  const char * layout;
};

constexpr GoldenCase kGoldenCases[] = {
  {"1+2", "3", "1+2", "1+2"},
  {"2+3×4", "14", "2+3×4", "2+3×4"},
  {"1/3", "1/3", "1/3", "\u0012\u00121\u0013/\u00123\u0013\u0013"},
  {"√(2)", "√(2)", "√(2)", "√\u00122\u0013"},
  {"sin(0)", "0", "sin(0)", "sin(0)"},
  {"cos(0)", "1", "cos(0)", "cos(0)"},
  {"1.5+2.5", "4", "1.5+2.5", "1.5+2.5"},
  {"3_s", "3×_s", "3×_s", "3 _s"},
  {"x^2", "x^2", "x^2", "x^\u00122\u0013"},
  {"[[1,2][3,4]]", "[[1,2][3,4]]", "[[1,2][3,4]]", "[[1,2][3,4]]"},
};

}

QUIZ_CASE(poincare_golden_expressions) {
  for (const GoldenCase & golden : kGoldenCases) {
    assert_parsed_expression_simplify_to(golden.input, golden.simplified);
    Expression expression = parse_expression(golden.input, nullptr, false);
    assert_expression_serialize_to(expression, golden.serialized);
    assert_layout_serialize_to(
        expression.createLayout(DecimalMode, PrintFloat::k_numberOfStoredSignificantDigits),
        golden.layout);
  }
}
