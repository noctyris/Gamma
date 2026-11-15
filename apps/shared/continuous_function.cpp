#include "continuous_function.h"
#include "poincare_helpers.h"
#include <apps/constant.h>
#include <poincare/derivative.h>
#include <poincare/float.h>
#include <poincare/integer.h>
#include <poincare/horizontal_layout.h>
#include <poincare/integral.h>
#include <poincare/layout_helper.h>
#include <poincare/matrix.h>
#include <poincare/multiplication.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>
#include <poincare/trigonometry.h>
#include <poincare/undefined.h>
#include <escher/palette.h>
#include <ion/unicode/utf8_helper.h>
#include <ion/unicode/utf8_decoder.h>
#include <apps/i18n.h>
#include <float.h>
#include <cmath>
#include <string.h>
#include <algorithm>

using namespace Poincare;

namespace Shared {

namespace {

bool FindTopLevelInequality(const char * text, int * position,
                            int * operatorLength,
                            ContinuousFunction::Inequality * kind) {
  int parenthesisDepth = 0;
  int bracketDepth = 0;
  int braceDepth = 0;
  for (int i = 0; text[i] != 0; i++) {
    char c = text[i];
    if (c == '(') {
      parenthesisDepth++;
      continue;
    }
    if (c == ')') {
      parenthesisDepth--;
      continue;
    }
    if (c == '[') {
      bracketDepth++;
      continue;
    }
    if (c == ']') {
      bracketDepth--;
      continue;
    }
    if (c == '{') {
      braceDepth++;
      continue;
    }
    if (c == '}') {
      braceDepth--;
      continue;
    }
    if (parenthesisDepth != 0 || bracketDepth != 0 || braceDepth != 0) {
      continue;
    }
    if (c == '<' || c == '>') {
      bool equal = text[i + 1] == '=';
      *operatorLength = equal ? 2 : 1;
      *position = i;
      if (c == '<') {
        *kind = equal ? ContinuousFunction::Inequality::InferiorEqual
                      : ContinuousFunction::Inequality::Inferior;
      } else {
        *kind = equal ? ContinuousFunction::Inequality::SuperiorEqual
                      : ContinuousFunction::Inequality::Superior;
      }
      return true;
    }
    if (static_cast<unsigned char>(c) == 0xE2 &&
        static_cast<unsigned char>(text[i + 1]) == 0x89 &&
        (static_cast<unsigned char>(text[i + 2]) == 0xA4 ||
         static_cast<unsigned char>(text[i + 2]) == 0xA5)) {
      *position = i;
      *operatorLength = 3;
      *kind = static_cast<unsigned char>(text[i + 2]) == 0xA4
                  ? ContinuousFunction::Inequality::InferiorEqual
                  : ContinuousFunction::Inequality::SuperiorEqual;
      return true;
    }
  }
  return false;
}

bool IsEmptyText(const char * text) {
  if (!text) {
    return true;
  }
  while (*text == ' ' || *text == '\t' || *text == '\n' || *text == '\r') {
    text++;
  }
  return *text == 0;
}

}  // namespace

void ContinuousFunction::DefaultName(char buffer[], size_t bufferSize) {
  constexpr int k_maxNumberOfDefaultLetterNames = 4;
  static constexpr const char k_defaultLetterNames[k_maxNumberOfDefaultLetterNames] = {
    'f', 'g', 'h', 'p'
  };
  /* First default names are f, g, h, p and then f0, f1... ie, "f[number]",
   * for instance "f12", that does not exist yet in the storage. */
  size_t constantNameLength = 1; // 'f', no null-terminating char
  assert(bufferSize > constantNameLength+1);
  // Find the next available name
  int currentNumber = -k_maxNumberOfDefaultLetterNames;
  int currentNumberLength = 0;
  int availableBufferSize = bufferSize - constantNameLength;
  while (currentNumberLength < availableBufferSize) {
    // Choose letter
    buffer[0] = currentNumber < 0 ? k_defaultLetterNames[k_maxNumberOfDefaultLetterNames+currentNumber] : k_defaultLetterNames[0];
    // Choose number if required
    if (currentNumber >= 0) {
      currentNumberLength = Poincare::Integer(currentNumber).serialize(&buffer[1], availableBufferSize);
    } else {
      buffer[1] = 0;
    }
    if (GlobalContext::SymbolAbstractNameIsFree(buffer)) {
      // Name found
      break;
    }
    currentNumber++;
  }
  assert(currentNumberLength >= 0 && currentNumberLength < availableBufferSize);
}

void ContinuousFunction::DefaultPolarName(char buffer[], size_t bufferSize) {
  buffer[0] = 'r';
  for (int number = 1; number < 100; number++) {
    int length = 1 + Poincare::Integer(number).serialize(buffer + 1,
                                                         bufferSize - 1);
    if (GlobalContext::SymbolAbstractNameIsFree(buffer)) {
      return;
    }
    (void)length;
  }
  DefaultName(buffer, bufferSize);
}

ContinuousFunction ContinuousFunction::NewModel(Ion::Storage::Record::ErrorStatus * error, const char * baseName) {
  static int s_colorIndex = 0;
  // Create the record
  char nameBuffer[SymbolAbstract::k_maxNameSize];
  RecordDataBuffer data(Palette::nextDataColor(&s_colorIndex));
  if (baseName == nullptr) {
    DefaultName(nameBuffer, SymbolAbstract::k_maxNameSize);
    baseName = nameBuffer;
  }
  *error = Ion::Storage::sharedStorage()->createRecordWithExtension(baseName, Ion::Storage::funcExtension, &data, sizeof(data));

  // Return if error
  if (*error != Ion::Storage::Record::ErrorStatus::None) {
    return ContinuousFunction();
  }

  // Return the ContinuousFunction withthe new record
  return ContinuousFunction(Ion::Storage::sharedStorage()->recordBaseNamedWithExtension(baseName, Ion::Storage::funcExtension));
}

int ContinuousFunction::derivativeNameWithArgument(char * buffer, size_t bufferSize) {
  // Fill buffer with f(x). Keep size for derivative sign.
  int derivativeSize = UTF8Decoder::CharSizeOfCodePoint('\'');
  int numberOfChars = nameWithArgument(buffer, bufferSize - derivativeSize);
  assert(numberOfChars + derivativeSize < (int)bufferSize);
  char * firstParenthesis = const_cast<char *>(UTF8Helper::CodePointSearch(buffer, '('));
  if (!UTF8Helper::CodePointIs(firstParenthesis, '(')) {
    return numberOfChars;
  }
  memmove(firstParenthesis + derivativeSize, firstParenthesis, numberOfChars - (firstParenthesis - buffer) + 1);
  UTF8Decoder::CodePointToChars('\'', firstParenthesis, derivativeSize);
  return numberOfChars + derivativeSize;
}

Poincare::Expression ContinuousFunction::expressionReduced(Poincare::Context * context) const {
  if (isRawContent()) {
    /* Piecewise and scatter plots are parsed by the application layer. Their
     * storage is not a Poincare tree. */
    return Undefined::Builder();
  }
  Poincare::Expression result = ExpressionModelHandle::expressionReduced(context);
  if (plotType() == PlotType::Parametric && (
      result.type() != Poincare::ExpressionNode::Type::Matrix ||
      static_cast<Poincare::Matrix&>(result).numberOfRows() != 2 ||
      static_cast<Poincare::Matrix&>(result).numberOfColumns() != 1)
     ) {
    return Poincare::Expression::Parse("[[undef][undef]]", nullptr);
  }
  return result;
}

Poincare::Expression ContinuousFunction::expressionClone() const {
  if (isRawContent()) {
    return Poincare::Expression();
  }
  return ExpressionModelHandle::expressionClone();
}

void ContinuousFunction::text(char * buffer, size_t bufferSize) const {
  if (isRawContent()) {
    strlcpy(buffer, rawContent(), bufferSize);
    return;
  }
  if (plotType() == PlotType::Parametric) {
    Expression e = expressionClone();
    if (!e.isUninitialized() &&
        e.type() == ExpressionNode::Type::Matrix &&
        static_cast<Matrix &>(e).numberOfRows() == 2 &&
        static_cast<Matrix &>(e).numberOfColumns() == 1) {
      e = e.replaceSymbolWithExpression(Symbol::Builder(UCodePointUnknown),
                                        Symbol::Builder(symbol()));
      int written = 0;
      if (bufferSize > 0) {
        buffer[written++] = '(';
      }
      written += e.childAtIndex(0).serialize(buffer + written,
                                             bufferSize - written);
      if (written < static_cast<int>(bufferSize) - 1) {
        buffer[written++] = ',';
      }
      written += e.childAtIndex(1).serialize(buffer + written,
                                             bufferSize - written);
      if (written < static_cast<int>(bufferSize) - 1) {
        buffer[written++] = ')';
        buffer[written] = 0;
      }
      return;
    }
  }
  ExpressionModelHandle::text(buffer, bufferSize);
}

Poincare::Layout ContinuousFunction::layout() {
  if (isRawContent()) {
    const char * source = rawContent();
    return Poincare::LayoutHelper::String(source, strlen(source));
  }
  if (plotType() == PlotType::Parametric) {
    Expression e = expressionClone();
    if (!e.isUninitialized() &&
        e.type() == ExpressionNode::Type::Matrix &&
        static_cast<Matrix &>(e).numberOfRows() == 2 &&
        static_cast<Matrix &>(e).numberOfColumns() == 1) {
      e = e.replaceSymbolWithExpression(Symbol::Builder(UCodePointUnknown),
                                        Symbol::Builder(symbol()));
      Layout first = PoincareHelpers::CreateLayout(e.childAtIndex(0));
      Layout second = PoincareHelpers::CreateLayout(e.childAtIndex(1));
      Layout comma = LayoutHelper::String(",", 1);
      Layout tuple = HorizontalLayout::Builder(first, comma, second);
      return LayoutHelper::Parentheses(tuple, true);
    }
  }
  return ExpressionModelHandle::layout();
}

void ContinuousFunction::tidy() {
  m_piecewise.reset();
  ExpressionModelHandle::tidy();
}

I18n::Message ContinuousFunction::parameterMessageName() const {
  return ParameterMessageForPlotType(plotType());
}

CodePoint ContinuousFunction::symbol() const {
  switch (plotType()) {
  case PlotType::Cartesian:
  case PlotType::Implicit:
  case PlotType::Scatter:
    return 'x';
  case PlotType::Polar:
    return UCodePointGreekSmallLetterTheta;
  default:
    assert(plotType() == PlotType::Parametric);
    return 't';
  }
}

ContinuousFunction::PlotType ContinuousFunction::plotType() const {
  return (PlotType)(static_cast<uint8_t>(recordData()->plotType()) & 0x0F);
}

ContinuousFunction::Inequality ContinuousFunction::inequality() const {
  return (Inequality)(static_cast<uint8_t>(recordData()->plotType()) >> 4);
}

void ContinuousFunction::setInequality(Inequality inequality) {
  recordData()->setInequality(inequality);
  setCache(nullptr);
  m_piecewise.reset();
  m_model.tidy();
}

void ContinuousFunction::setPlotType(PlotType newPlotType, Poincare::Preferences::AngleUnit angleUnit, Context * context) {
  PlotType currentPlotType = plotType();
  if (newPlotType == currentPlotType || isRawContent()) {
    return;
  }
  if (newPlotType == PlotType::Scatter) {
    recordData()->setInequality(Inequality::None);
    recordData()->setPlotType(PlotType::Scatter);
    setCache(nullptr);
    m_model.tidy();
    m_piecewise.reset();
    return;
  }
  if (currentPlotType == PlotType::Implicit || currentPlotType == PlotType::Scatter) {
    /* These models have no parameter to convert to another curve type. */
    return;
  }
  if (newPlotType == PlotType::Implicit) {
    recordData()->setInequality(Inequality::None);
    recordData()->setPlotType(newPlotType);
    setCache(nullptr);
    m_model.tidy();
    m_piecewise.reset();
    setTMin(-INFINITY);
    setTMax(INFINITY);
    return;
  }

  /* Changing the curve type discards any inequality: the new curve has its own
   * parameter, so an inequality on the old expression would be meaningless. */
  recordData()->setInequality(Inequality::None);
  recordData()->setPlotType(newPlotType);

  setCache(nullptr);
  m_piecewise.reset();

  // Recompute the layouts
  m_model.tidy();

  // Recompute the definition domain
  double tMin = newPlotType == PlotType::Cartesian ? -INFINITY : 0.0;
  double tMax = newPlotType == PlotType::Cartesian ? INFINITY : 2.0*Trigonometry::PiInAngleUnit(angleUnit);
  setTMin(tMin);
  setTMax(tMax);

  /* Recompute the unknowns. For instance, if the function was f(x) = xθ, it is
   * stored as f(?) = ?θ. When switching to polar type, it should be stored as
   * f(?) = ?? */
  constexpr int previousTextContentMaxSize = Constant::MaxSerializedExpressionSize;
  char previousTextContent[previousTextContentMaxSize];
  m_model.text(this, previousTextContent, previousTextContentMaxSize, symbol());
  setContent(previousTextContent, context);

  // Handle parametric function switch
  if (currentPlotType == PlotType::Parametric) {
    Expression e = expressionClone();
    // Change [x(t) y(t)] to y(t)
    if (!e.isUninitialized()
        && e.type() == ExpressionNode::Type::Matrix
        && static_cast<Poincare::Matrix&>(e).numberOfRows() == 2
        && static_cast<Poincare::Matrix&>(e).numberOfColumns() == 1)
    {
      Expression nextContent = e.childAtIndex(1);
      /* We need to detach it, otherwise nextContent will think it has a parent
       * when we retrieve it from the storage. */
      nextContent.detachFromParent();
      setExpressionContent(nextContent);
    }
    return;
  } else if (newPlotType == PlotType::Parametric) {
    Expression e = expressionClone();
    // Change y(t) to [t y(t)]
    Matrix newExpr = Matrix::Builder();
    newExpr.addChildAtIndexInPlace(Symbol::Builder(UCodePointUnknown), 0, 0);
    // if y(t) was not uninitialized, insert [t 2t] to set an example
    e = e.isUninitialized() ? Multiplication::Builder(Rational::Builder(2), Symbol::Builder(UCodePointUnknown)) : e;
    newExpr.addChildAtIndexInPlace(e, newExpr.numberOfChildren(), newExpr.numberOfChildren());
    newExpr.setDimensions(2, 1);
    setExpressionContent(newExpr);
  }
}

I18n::Message ContinuousFunction::ParameterMessageForPlotType(PlotType plotType) {
  if (plotType == PlotType::Cartesian || plotType == PlotType::Implicit ||
      plotType == PlotType::Scatter) {
    return I18n::Message::X;
  }
  if (plotType == PlotType::Polar) {
    return I18n::Message::Theta;
  }
  assert(plotType == PlotType::Parametric);
  return I18n::Message::T;
}

int ContinuousFunction::numberOfPoints() const {
  return isScatterPlot() ? FunctionEquation::NumberOfPoints(rawContent()) : 0;
}

bool ContinuousFunction::pointAtIndex(int index,
                                      Poincare::Coordinate2D<double> * point,
                                      Poincare::Context * context) const {
  if (!isScatterPlot()) {
    return false;
  }
  FunctionEquation::Point2D parsedPoint;
  if (!FunctionEquation::PointAtIndex(rawContent(), index, &parsedPoint,
                                      context)) {
    return false;
  }
  *point = Poincare::Coordinate2D<double>(parsedPoint.x, parsedPoint.y);
  return true;
}

template <typename T>
Poincare::Coordinate2D<T> ContinuousFunction::privateEvaluateXYAtParameter(T t, Poincare::Context * context) const {
  if (isScatter()) {
    return Coordinate2D<T>(NAN, NAN);
  }
  if (isPiecewise()) {
    double y = piecewise(context).evaluate((double)t, context);
    return Coordinate2D<T>(t, static_cast<T>(y));
  }
  Coordinate2D<T> x1x2 = templatedApproximateAtParameter(t, context);
  PlotType type = plotType();
  if (type == PlotType::Cartesian || type == PlotType::Parametric || type == PlotType::Implicit) {
    return x1x2;
  }
  assert(type == PlotType::Polar);
  T factor = (T)1.0;
  Preferences::AngleUnit angleUnit = Preferences::sharedPreferences()->angleUnit();
  if (angleUnit == Preferences::AngleUnit::Degree) {
    factor = (T) (M_PI/180.0);
  } else if (angleUnit == Preferences::AngleUnit::Gradian) {
    factor = (T) (M_PI/200.0);
  } else {
    assert(angleUnit == Preferences::AngleUnit::Radian);
  }
  const float angle = x1x2.x1()*factor;
  return Coordinate2D<T>(x1x2.x2() * std::cos(angle), x1x2.x2() * std::sin(angle));
}

bool ContinuousFunction::displayDerivative() const {
  return recordData()->displayDerivative();
}

void ContinuousFunction::setDisplayDerivative(bool display) {
  return recordData()->setDisplayDerivative(display);
}

int ContinuousFunction::printValue(double cursorT, double cursorX, double cursorY, char * buffer, int bufferSize, int precision, Poincare::Context * context) {
  PlotType type = plotType();
  if (type == PlotType::Cartesian || type == PlotType::Scatter) {
    return Function::printValue(cursorT, cursorX, cursorY, buffer, bufferSize, precision, context);
  }
  if (type == PlotType::Polar) {
    return PoincareHelpers::ConvertFloatToText<double>(evaluate2DAtParameter(cursorT, context).x2(), buffer, bufferSize, precision);
  }
  assert(type == PlotType::Parametric);
  int result = 0;
  result += UTF8Decoder::CodePointToChars('(', buffer+result, bufferSize-result);
  result += PoincareHelpers::ConvertFloatToText<double>(cursorX, buffer+result, bufferSize-result, precision);
  result += UTF8Decoder::CodePointToChars(';', buffer+result, bufferSize-result);
  result += PoincareHelpers::ConvertFloatToText<double>(cursorY, buffer+result, bufferSize-result, precision);
  result += UTF8Decoder::CodePointToChars(')', buffer+result, bufferSize-result);
  return result;
}

double ContinuousFunction::approximateDerivative(double x, Poincare::Context * context) const {
  if (isPiecewise() || isScatter()) {
    return NAN;
  }
  if (plotType() != PlotType::Cartesian || x < tMin() || x > tMax()) {
    return NAN;
  }
  Poincare::Derivative derivative = Poincare::Derivative::Builder(expressionReduced(context).clone(), Symbol::Builder(UCodePointUnknown), Poincare::Float<double>::Builder(x)); // derivative takes ownership of Poincare::Float<double>::Builder(x) and the clone of expression
  /* TODO: when we approximate derivative, we might want to simplify the
   * derivative here. However, we might want to do it once for all x (to avoid
   * lagging in the derivative table. */
  return PoincareHelpers::ApproximateToScalar<double>(derivative, context);
}

float ContinuousFunction::tMin() const {
  return recordData()->tMin();
}

float ContinuousFunction::tMax() const {
  return recordData()->tMax();
}

void ContinuousFunction::setTMin(float tMin) {
  recordData()->setTMin(tMin);
  setCache(nullptr);
}

void ContinuousFunction::setTMax(float tMax) {
  recordData()->setTMax(tMax);
  setCache(nullptr);
}

void ContinuousFunction::rangeForDisplay(float * xMin, float * xMax, float * yMin, float * yMax, float targetRatio, Poincare::Context * context) const {
  if (isScatterPlot()) {
    float minX = INFINITY;
    float maxX = -INFINITY;
    float minY = INFINITY;
    float maxY = -INFINITY;
    int points = numberOfPoints();
    for (int i = 0; i < points; i++) {
      Coordinate2D<double> point;
      if (pointAtIndex(i, &point, context)) {
        minX = std::min(minX, static_cast<float>(point.x1()));
        maxX = std::max(maxX, static_cast<float>(point.x1()));
        minY = std::min(minY, static_cast<float>(point.x2()));
        maxY = std::max(maxY, static_cast<float>(point.x2()));
      }
    }
    if (!std::isfinite(minX) || !std::isfinite(minY)) {
      *xMin = -10.0f;
      *xMax = 10.0f;
      *yMin = -10.0f;
      *yMax = 10.0f;
      return;
    }
    float xMargin = std::max(1.0f, (maxX - minX) * 0.15f);
    float yMargin = std::max(1.0f, (maxY - minY) * 0.15f);
    *xMin = minX - xMargin;
    *xMax = maxX + xMargin;
    *yMin = minY - yMargin;
    *yMax = maxY + yMargin;
    return;
  }
  if (isPiecewise()) {
    constexpr int k_numberOfSamples = 200;
    constexpr float k_halfRange = 10.0f;
    float minY = INFINITY;
    float maxY = -INFINITY;
    for (int i = 0; i <= k_numberOfSamples; i++) {
      float x = -k_halfRange + (2.0f * k_halfRange * i) / k_numberOfSamples;
      double y = piecewise(context).evaluate(x, context);
      if (std::isfinite(y)) {
        minY = std::min(minY, static_cast<float>(y));
        maxY = std::max(maxY, static_cast<float>(y));
      }
    }
    *xMin = -k_halfRange;
    *xMax = k_halfRange;
    if (std::isfinite(minY) && std::isfinite(maxY)) {
      float margin = std::max(1.0f, (maxY - minY) * 0.15f);
      *yMin = minY - margin;
      *yMax = maxY + margin;
    } else {
      *yMin = -k_halfRange;
      *yMax = k_halfRange;
    }
    return;
  }
  if (plotType() == PlotType::Implicit) {
    /* Implicit curves are evaluated on a grid, so their extent cannot be
     * derived from a parameter range. Use a sensible default window. */
    *xMin = -10.0f;
    *xMax = 10.0f;
    *yMin = -10.0f;
    *yMax = 10.0f;
    return;
  }
  if (plotType() != PlotType::Cartesian) {
    assert(std::isfinite(tMin()) && std::isfinite(tMax()) && std::isfinite(rangeStep()) && rangeStep() > 0);
    protectedFullRangeForDisplay(tMin(), tMax(), rangeStep(), xMin, xMax, context, true);
    protectedFullRangeForDisplay(tMin(), tMax(), rangeStep(), yMin, yMax, context, false);
    return;
  }

  if (!basedOnCostlyAlgorithms(context)) {
    Zoom::ValueAtAbscissa evaluation = [](float x, Context * context, const void * auxiliary) -> float {
      /* When evaluating sin(x)/x close to zero using the standard sine function,
       * one can detect small variations, while the cardinal sine is supposed to be
       * locally monotonous. To smooth our such variations, we round the result of
       * the evaluations. As we are not interested in precise results but only in
       * ordering, this approximation is sufficient. */
      constexpr float precision = 1e-5;
      return precision * std::round(static_cast<const Function *>(auxiliary)->evaluateXYAtParameter(x, context).x2() / precision);
    };
    bool fullyComputed = Zoom::InterestingRangesForDisplay(evaluation, xMin, xMax, yMin, yMax, tMin(), tMax(), context, this);

    evaluation = [](float x, Context * context, const void * auxiliary) {
      return static_cast<const Function *>(auxiliary)->evaluateXYAtParameter(x, context).x2();
    };

    if (fullyComputed) {
      /* The function has points of interest. */
      Zoom::RefinedYRangeForDisplay(evaluation, xMin, xMax, yMin, yMax, context, this);
      return;
    }

    /* Try to display an orthonormal range. */
    Zoom::RangeWithRatioForDisplay(evaluation, targetRatio, xMin, xMax, yMin, yMax, context, this);
    if (std::isfinite(*xMin) && std::isfinite(*xMax) && std::isfinite(*yMin) && std::isfinite(*yMax)) {
      return;
    }

    /* The function's profile is not great for an orthonormal range.
     * Try a basic range. */
    *xMin = - Zoom::k_defaultHalfRange;
    *xMax = Zoom::k_defaultHalfRange;
    Zoom::RefinedYRangeForDisplay(evaluation, xMin, xMax, yMin, yMax, context, this);
    if (std::isfinite(*xMin) && std::isfinite(*xMax) && std::isfinite(*yMin) && std::isfinite(*yMax)) {
      return;
    }

    /* The function's order of magnitude cannot be computed. Try to just display
     * the full function. */
    float step =  (*xMax - *xMin) / k_polarParamRangeSearchNumberOfPoints;
    Zoom::FullRange(evaluation, *xMin, *xMax, step, yMin, yMax, context, this);
    if (std::isfinite(*xMin) && std::isfinite(*xMax) && std::isfinite(*yMin) && std::isfinite(*yMax)) {
      return;
    }
  }

  /* The function makes use of some costly algorithms and cannot be computed in
   * a timely manner, or it is probably undefined. */
  *xMin = NAN;
  *xMax = NAN;
  *yMin = NAN;
  *yMax = NAN;
}

void * ContinuousFunction::Model::expressionAddress(const Ion::Storage::Record * record) const {
  return (char *)record->value().buffer+sizeof(RecordDataBuffer);
}

size_t ContinuousFunction::Model::expressionSize(const Ion::Storage::Record * record) const {
  return record->value().size-sizeof(RecordDataBuffer);
}

ContinuousFunction::RecordDataBuffer * ContinuousFunction::recordData() const {
  assert(!isNull());
  Ion::Storage::Record::Data d = value();
  return reinterpret_cast<RecordDataBuffer *>(const_cast<void *>(d.buffer));
}

template<typename T>
Coordinate2D<T> ContinuousFunction::templatedApproximateAtParameter(T t, Poincare::Context * context) const {
  if (isScatter()) {
    return Coordinate2D<T>(NAN, NAN);
  }
  if (isPiecewise()) {
    return Coordinate2D<T>(t, static_cast<T>(piecewise(context).evaluate((double)t, context)));
  }
  if (t < tMin() || t > tMax()) {
    return Coordinate2D<T>(plotType() == PlotType::Cartesian ? t : NAN, NAN);
  }
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknown[bufferSize];
  Poincare::SerializationHelper::CodePoint(unknown, bufferSize, UCodePointUnknown);
  PlotType type = plotType();
  if (type == PlotType::Implicit) {
    /* An implicit curve is evaluated on a two-dimensional grid by
     * evaluateImplicitAt, not with a single parameter. */
    return Coordinate2D<T>(t, NAN);
  }
  Expression e = expressionReduced(context);
  if (type != PlotType::Parametric) {
    assert(type == PlotType::Cartesian || type == PlotType::Polar);
    return Coordinate2D<T>(t, PoincareHelpers::ApproximateWithValueForSymbol(e, unknown, t, context));
  }
  assert(e.type() == ExpressionNode::Type::Matrix);
  assert(static_cast<Poincare::Matrix&>(e).numberOfRows() == 2);
  assert(static_cast<Poincare::Matrix&>(e).numberOfColumns() == 1);
  return Coordinate2D<T>(
      PoincareHelpers::ApproximateWithValueForSymbol(e.childAtIndex(0), unknown, t, context),
      PoincareHelpers::ApproximateWithValueForSymbol(e.childAtIndex(1), unknown, t, context));
}

Coordinate2D<double> ContinuousFunction::nextMinimumFrom(double start, double step, double max, Context * context) const {
  if (isPiecewise() || isScatter()) {
    return Coordinate2D<double>(NAN, NAN);
  }
  return nextPointOfInterestFrom(start, step, max, context, [](Expression e, char * symbol, double start, double step, double max, Context * context) { return PoincareHelpers::NextMinimum(e, symbol, start, step, max, context); });
}

Coordinate2D<double> ContinuousFunction::nextMaximumFrom(double start, double step, double max, Context * context) const {
  if (isPiecewise() || isScatter()) {
    return Coordinate2D<double>(NAN, NAN);
  }
  return nextPointOfInterestFrom(start, step, max, context, [](Expression e, char * symbol, double start, double step, double max, Context * context) { return PoincareHelpers::NextMaximum(e, symbol, start, step, max, context); });
}

Coordinate2D<double> ContinuousFunction::nextRootFrom(double start, double step, double max, Context * context) const {
  if (isPiecewise() || isScatter()) {
    return Coordinate2D<double>(NAN, NAN);
  }
  return nextPointOfInterestFrom(start, step, max, context, [](Expression e, char * symbol, double start, double step, double max, Context * context) { return Coordinate2D<double>(PoincareHelpers::NextRoot(e, symbol, start, step, max, context), 0.0); });
}

Coordinate2D<double> ContinuousFunction::nextIntersectionFrom(double start, double step, double max, Poincare::Context * context, Poincare::Expression e, double eDomainMin, double eDomainMax) const {
  if (isPiecewise() || isScatter() || plotType() != PlotType::Cartesian) {
    return Coordinate2D<double>(NAN, NAN);
  }
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownX[bufferSize];
  SerializationHelper::CodePoint(unknownX, bufferSize, UCodePointUnknown);
  double domainMin = std::max<double>(tMin(), eDomainMin);
  double domainMax = std::min<double>(tMax(), eDomainMax);
  if (step > 0.0f) {
    start = std::max(start, domainMin);
    max = std::min(max, domainMax);
  } else {
    start = std::min(start, domainMax);
    max = std::max(max, domainMin);
  }
  return PoincareHelpers::NextIntersection(expressionReduced(context), unknownX, start, step, max, context, e);
}

Coordinate2D<double> ContinuousFunction::nextPointOfInterestFrom(double start, double step, double max, Context * context, ComputePointOfInterest compute) const {
  assert(plotType() == PlotType::Cartesian && !isPiecewise() && !isScatter());
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownX[bufferSize];
  SerializationHelper::CodePoint(unknownX, bufferSize, UCodePointUnknown);
  if (step > 0.0f) {
    start = std::max<double>(start, tMin());
    max = std::min<double>(max, tMax());
  } else {
    start = std::min<double>(start, tMax());
    max = std::max<double>(max, tMin());
  }
  return compute(expressionReduced(context), unknownX, start, step, max, context);
}

Poincare::Expression ContinuousFunction::sumBetweenBounds(double start, double end, Poincare::Context * context) const {
  if (isPiecewise() || isScatter() || plotType() != PlotType::Cartesian) {
    return Undefined::Builder();
  }
  start = std::max<double>(start, tMin());
  end = std::min<double>(end, tMax());
  return Poincare::Integral::Builder(expressionReduced(context).clone(), Poincare::Symbol::Builder(UCodePointUnknown), Poincare::Float<double>::Builder(start), Poincare::Float<double>::Builder(end)); // Integral takes ownership of args
  /* TODO: when we approximate integral, we might want to simplify the integral
   * here. However, we might want to do it once for all x (to avoid lagging in
   * the derivative table. */
}

double ContinuousFunction::evaluateImplicitAt(double x, double y, Poincare::Context * context) const {
  if (isRawContent()) {
    return NAN;
  }
  /* Work on a clone: replaceSymbolWithExpression mutates the tree in place and
   * expressionReduced returns a cached handle. Mutating the cache would turn
   * the implicit equation into a constant after the first evaluation. */
  Expression e = expressionClone();
  if (e.isUninitialized()) {
    return NAN;
  }
  /* The model stores either the full equation (x^2+y^2=1) or a single
   * expression to compare to zero (x+y for x+y<=0). */
  auto approximateSide = [context, x, y](Expression side) -> double {
    side = side.replaceSymbolWithExpression(Symbol::Builder('x'), Float<double>::Builder(x));
    side = side.replaceSymbolWithExpression(Symbol::Builder('y'), Float<double>::Builder(y));
    return PoincareHelpers::ApproximateToScalar<double>(side, context);
  };
  if (e.type() == ExpressionNode::Type::Equal) {
    return approximateSide(e.childAtIndex(0)) - approximateSide(e.childAtIndex(1));
  }
  return approximateSide(e);
}

const FunctionEquation::Piecewise & ContinuousFunction::piecewise(
    Poincare::Context * context) const {
  if (!m_piecewise.isParsed() && isPiecewise()) {
    m_piecewise.parse(rawContent(), context);
  }
  return m_piecewise;
}

Ion::Storage::Record::ErrorStatus ContinuousFunction::setRawContent(
    const char * text, Inequality kind) {
  Ion::Storage::Record::Data data = value();
  const size_t previousExpressionSize = data.size - sizeof(RecordDataBuffer);
  const size_t newExpressionSize = strlen(text) + 1;
  const size_t previousDataSize = data.size;
  const size_t newDataSize =
      previousDataSize - previousExpressionSize + newExpressionSize;
  data.size = std::max(previousDataSize, newDataSize);
  Ion::Storage::Record::ErrorStatus error = setValue(data);
  if (error != Ion::Storage::Record::ErrorStatus::None) {
    return error;
  }
  Ion::Storage::Record::Data resizedData = value();
  void * expressionAddress = const_cast<char *>(
      reinterpret_cast<const char *>(resizedData.buffer)) +
      sizeof(RecordDataBuffer);
  memmove(expressionAddress, text, newExpressionSize);
  resizedData.size = newDataSize;
  error = setValue(resizedData);
  if (error != Ion::Storage::Record::ErrorStatus::None) {
    return error;
  }
  recordData()->setPlotType(kind == Inequality::Point ||
                                    kind == Inequality::PointList
                                ? PlotType::Scatter
                                : PlotType::Cartesian);
  recordData()->setInequality(kind);
  setTMin(-INFINITY);
  setTMax(INFINITY);
  setCache(nullptr);
  m_piecewise.reset();
  m_model.tidy();
  return Ion::Storage::Record::ErrorStatus::None;
}

Ion::Storage::Record::ErrorStatus ContinuousFunction::setExpressionText(
    const char * text, Poincare::Context * context, CodePoint symbol) {
  if (symbol == 0) {
    return editableModel()->setContent(this, text, context, 0);
  }
  return ExpressionModelHandle::setContent(text, context);
}

void ContinuousFunction::resetRawFlag() {
  const bool wasScatter = plotType() == PlotType::Scatter;
  recordData()->setInequality(Inequality::None);
  if (wasScatter) {
    recordData()->setPlotType(PlotType::Cartesian);
    recordData()->setTMin(-INFINITY);
    recordData()->setTMax(INFINITY);
  }
  setCache(nullptr);
  m_piecewise.reset();
  m_model.tidy();
}

Ion::Storage::Record::ErrorStatus ContinuousFunction::setContent(const char * c, Poincare::Context * context) {
  setCache(nullptr);
  m_piecewise.reset();
  m_model.tidy();

  if (IsEmptyText(c)) {
    resetRawFlag();
    return setExpressionText("", context,
                             isImplicit() ? CodePoint(0) : symbol());
  }

  char strippedContent[Constant::MaxSerializedExpressionSize];
  if (FunctionEquation::StripFunctionAssignment(c, strippedContent,
                                                 sizeof(strippedContent))) {
    c = strippedContent;
  }

  if (FunctionEquation::IsPiecewise(c)) {
    return setRawContent(c, Inequality::Piecewise);
  }

  /* Check the parametric tuple before the point syntax: both are written with
   * parentheses and a comma. */
  if (plotType() == PlotType::Parametric) {
    char matrixText[Constant::MaxSerializedExpressionSize];
    if (FunctionEquation::ParametricTupleToMatrix(c, matrixText,
                                                   sizeof(matrixText))) {
      resetRawFlag();
      return setExpressionText(matrixText, context, symbol());
    }
  }

  if (FunctionEquation::IsPointList(c)) {
    return setRawContent(c, Inequality::PointList);
  }
  if (FunctionEquation::IsPoint(c)) {
    return setRawContent(c, Inequality::Point);
  }

  if (isImplicit()) {
    int inequalityPosition = 0;
    int inequalityLength = 0;
    Inequality inequality = Inequality::None;
    if (FindTopLevelInequality(c, &inequalityPosition, &inequalityLength,
                               &inequality)) {
      char leftMember[Constant::MaxSerializedExpressionSize];
      int leftLength = inequalityPosition;
      if (leftLength > 0 &&
          leftLength < static_cast<int>(sizeof(leftMember))) {
        memcpy(leftMember, c, leftLength);
        leftMember[leftLength] = 0;
        resetRawFlag();
        setInequality(inequality);
        return setExpressionText(leftMember, context, 0);
      }
    }
  }

  resetRawFlag();
  return setExpressionText(c, context,
                           isImplicit() ? CodePoint(0) : symbol());
}

bool ContinuousFunction::basedOnCostlyAlgorithms(Context * context) const {
  if (isRawContent()) {
    return false;
  }
  return expressionReduced(context).hasExpression([](const Expression e, const void * context) {
        return e.type() == ExpressionNode::Type::Sequence
            || e.type() == ExpressionNode::Type::Integral
            || e.type() == ExpressionNode::Type::Derivative;
      }, nullptr);
}

template Coordinate2D<float> ContinuousFunction::templatedApproximateAtParameter<float>(float, Poincare::Context *) const;
template Coordinate2D<double> ContinuousFunction::templatedApproximateAtParameter<double>(double, Poincare::Context *) const;

template Poincare::Coordinate2D<float> ContinuousFunction::privateEvaluateXYAtParameter<float>(float, Poincare::Context *) const;
template Poincare::Coordinate2D<double> ContinuousFunction::privateEvaluateXYAtParameter<double>(double, Poincare::Context *) const;

}
