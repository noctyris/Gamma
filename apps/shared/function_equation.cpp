#include "function_equation.h"
#include "poincare_helpers.h"

#include <cmath>
#include <string.h>

using namespace Poincare;

namespace Shared {

namespace FunctionEquation {

namespace {

constexpr int k_maxNumberOfArguments = 2 * Piecewise::k_maxNumberOfBranches + 1;

bool IsSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void SkipSpaces(const char * text, int * index) {
  while (text[*index] != 0 && IsSpace(text[*index])) {
    (*index)++;
  }
}

int MatchingParenthesis(const char * text, int opening) {
  int depth = 0;
  for (int i = opening; text[i] != 0; i++) {
    if (text[i] == '(') {
      depth++;
    } else if (text[i] == ')') {
      depth--;
      if (depth == 0) {
        return i;
      }
    }
  }
  return -1;
}

int TopLevelComma(const char * text, int length) {
  int parenthesisDepth = 0;
  int bracketDepth = 0;
  int braceDepth = 0;
  for (int i = 0; i < length; i++) {
    char c = text[i];
    if (c == '(') {
      parenthesisDepth++;
    } else if (c == ')') {
      parenthesisDepth--;
    } else if (c == '[') {
      bracketDepth++;
    } else if (c == ']') {
      bracketDepth--;
    } else if (c == '{') {
      braceDepth++;
    } else if (c == '}') {
      braceDepth--;
    } else if (c == ',' && parenthesisDepth == 0 && bracketDepth == 0 &&
               braceDepth == 0) {
      return i;
    }
  }
  return -1;
}

int TopLevelComparison(const char * text, int length, int * operatorLength,
                       Piecewise::Comparison * comparison) {
  int parenthesisDepth = 0;
  int bracketDepth = 0;
  int braceDepth = 0;
  for (int i = 0; i < length; i++) {
    char c = text[i];
    if (c == '(') {
      parenthesisDepth++;
    } else if (c == ')') {
      parenthesisDepth--;
    } else if (c == '[') {
      bracketDepth++;
    } else if (c == ']') {
      bracketDepth--;
    } else if (c == '{') {
      braceDepth++;
    } else if (c == '}') {
      braceDepth--;
    } else if (parenthesisDepth == 0 && bracketDepth == 0 &&
               braceDepth == 0) {
      if (c == '<' || c == '>') {
        bool equal = i + 1 < length && text[i + 1] == '=';
        *operatorLength = equal ? 2 : 1;
        if (c == '<') {
          *comparison = equal ? Piecewise::Comparison::InferiorEqual
                              : Piecewise::Comparison::Inferior;
        } else {
          *comparison = equal ? Piecewise::Comparison::SuperiorEqual
                              : Piecewise::Comparison::Superior;
        }
        return i;
      }
      if (c == '=') {
        *operatorLength = 1;
        *comparison = Piecewise::Comparison::Equal;
        return i;
      }
      if (static_cast<unsigned char>(c) == 0xE2 && i + 2 < length &&
          static_cast<unsigned char>(text[i + 1]) == 0x89 &&
          (static_cast<unsigned char>(text[i + 2]) == 0xA4 ||
           static_cast<unsigned char>(text[i + 2]) == 0xA5)) {
        *operatorLength = 3;
        *comparison = static_cast<unsigned char>(text[i + 2]) == 0xA4
                          ? Piecewise::Comparison::InferiorEqual
                          : Piecewise::Comparison::SuperiorEqual;
        return i;
      }
    }
  }
  return -1;
}

bool ParseExpression(const char * text, int length, Context * context,
                     Expression * expression) {
  char buffer[256];
  if (length <= 0 || length >= static_cast<int>(sizeof(buffer))) {
    return false;
  }
  memcpy(buffer, text, length);
  buffer[length] = 0;
  *expression = Expression::Parse(buffer, context);
  return !expression->isUninitialized();
}

bool SplitArguments(const char * text, int length, int * starts, int * lengths,
                    int maxNumberOfArguments, int * numberOfArguments) {
  int count = 0;
  int start = 0;
  int parenthesisDepth = 0;
  int bracketDepth = 0;
  int braceDepth = 0;
  for (int i = 0; i <= length; i++) {
    bool atEnd = i == length;
    char c = atEnd ? ',' : text[i];
    if (!atEnd) {
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
    }
    if (atEnd || (c == ',' && parenthesisDepth == 0 && bracketDepth == 0 &&
                  braceDepth == 0)) {
      if (count >= maxNumberOfArguments) {
        return false;
      }
      starts[count] = start;
      lengths[count] = i - start;
      count++;
      start = i + 1;
    }
  }
  *numberOfArguments = count;
  return count > 0;
}

bool ParsePointAt(const char * text, int * index, Point2D * point,
                  Context * context) {
  int i = *index;
  SkipSpaces(text, &i);
  if (text[i] != '(') {
    return false;
  }
  int closing = MatchingParenthesis(text, i);
  if (closing < 0) {
    return false;
  }
  int comma = TopLevelComma(text + i + 1, closing - i - 1);
  if (comma < 0) {
    return false;
  }
  comma += i + 1;
  Expression xExpression;
  Expression yExpression;
  if (!ParseExpression(text + i + 1, comma - i - 1, context, &xExpression) ||
      !ParseExpression(text + comma + 1, closing - comma - 1, context,
                       &yExpression)) {
    return false;
  }
  if (context != nullptr) {
    point->x =
        PoincareHelpers::ApproximateToScalar<double>(xExpression, context);
    point->y =
        PoincareHelpers::ApproximateToScalar<double>(yExpression, context);
    if (!std::isfinite(point->x) || !std::isfinite(point->y)) {
      return false;
    }
  } else {
    point->x = 0.0;
    point->y = 0.0;
  }
  *index = closing + 1;
  return true;
}

void RemoveOuterBraces(const char * text, int * start, int * end) {
  int i = *start;
  int j = *end;
  while (i < j && IsSpace(text[i])) {
    i++;
  }
  while (j > i && IsSpace(text[j - 1])) {
    j--;
  }
  if (i < j && text[i] == '{') {
    i++;
    if (j > i && text[j - 1] == '}') {
      j--;
    }
  }
  *start = i;
  *end = j;
}

bool StartsWith(const char * text, const char * prefix) {
  return strncmp(text, prefix, strlen(prefix)) == 0;
}

}  // namespace

bool IsPiecewise(const char * text) {
  if (!text) {
    return false;
  }
  int i = 0;
  SkipSpaces(text, &i);
  if (StartsWith(text + i, "piecewise(")) {
    return true;
  }
  if (text[i] != '{') {
    return false;
  }
  int operatorLength;
  Piecewise::Comparison comparison;
  int length = strlen(text);
  return TopLevelComparison(text + i + 1, length - i - 1, &operatorLength,
                            &comparison) >= 0;
}

bool IsPointList(const char * text) {
  if (!text) {
    return false;
  }
  int i = 0;
  SkipSpaces(text, &i);
  if (text[i] != '{') {
    return false;
  }
  int closing = strlen(text) - 1;
  while (closing > i && IsSpace(text[closing])) {
    closing--;
  }
  return text[closing] == '}' && text[i + 1] != 0 &&
         (text[i + 1] == '(' || text[i + 1] == ' ');
}

bool IsPoint(const char * text) {
  if (!text) {
    return false;
  }
  Point2D point;
  int index = 0;
  return ParsePointAt(text, &index, &point, nullptr) &&
         text[index] == 0;
}

int NumberOfPoints(const char * text) {
  if (!text) {
    return 0;
  }
  int start = 0;
  int end = strlen(text);
  if (text[start] == '{') {
    RemoveOuterBraces(text, &start, &end);
  }
  int index = start;
  int count = 0;
  while (index < end) {
    SkipSpaces(text, &index);
    if (index >= end) {
      break;
    }
    Point2D point;
    if (!ParsePointAt(text, &index, &point, nullptr)) {
      return 0;
    }
    count++;
    SkipSpaces(text, &index);
    if (index < end && text[index] == ',') {
      index++;
    }
  }
  return count;
}

bool PointAtIndex(const char * text, int index, Point2D * point,
                  Context * context) {
  if (!text || index < 0) {
    return false;
  }
  int start = 0;
  int end = strlen(text);
  if (text[start] == '{') {
    RemoveOuterBraces(text, &start, &end);
  }
  int cursor = start;
  for (int i = 0; i <= index; i++) {
    SkipSpaces(text, &cursor);
    if (cursor >= end || !ParsePointAt(text, &cursor, point, context)) {
      return false;
    }
    SkipSpaces(text, &cursor);
    if (i < index) {
      if (cursor >= end || text[cursor] != ',') {
        return false;
      }
      cursor++;
    }
  }
  return true;
}

bool StripFunctionAssignment(const char * text, char * buffer,
                             size_t bufferSize) {
  if (!text) {
    return false;
  }
  int start = 0;
  SkipSpaces(text, &start);
  int parenthesisDepth = 0;
  int bracketDepth = 0;
  int braceDepth = 0;
  for (int i = start; text[i] != 0; i++) {
    char c = text[i];
    if (c == '(') {
      parenthesisDepth++;
    } else if (c == ')') {
      parenthesisDepth--;
    } else if (c == '[') {
      bracketDepth++;
    } else if (c == ']') {
      bracketDepth--;
    } else if (c == '{') {
      braceDepth++;
    } else if (c == '}') {
      braceDepth--;
    } else if (c == '=' && parenthesisDepth == 0 && bracketDepth == 0 &&
               braceDepth == 0) {
      /* Only strip function-like left members. "x=..." and "x+y+1=0" must
       * keep their equal sign. */
      int leftEnd = i - 1;
      while (leftEnd >= start && IsSpace(text[leftEnd])) {
        leftEnd--;
      }
      bool hasParenthesis = false;
      for (int j = start; j <= leftEnd; j++) {
        if (text[j] == '(') {
          hasParenthesis = true;
          break;
        }
      }
      if (!hasParenthesis || leftEnd < start || text[leftEnd] != ')') {
        return false;
      }
      int rightStart = i + 1;
      SkipSpaces(text, &rightStart);
      if (text[rightStart] == 0) {
        return false;
      }
      int rightEnd = strlen(text);
      while (rightEnd > rightStart && IsSpace(text[rightEnd - 1])) {
        rightEnd--;
      }
      int length = rightEnd - rightStart;
      if (length <= 0 || static_cast<size_t>(length) >= bufferSize) {
        return false;
      }
      memcpy(buffer, text + rightStart, length);
      buffer[length] = 0;
      return true;
    }
  }
  return false;
}

bool ParametricTupleToMatrix(const char * text, char * buffer,
                             size_t bufferSize) {
  if (!text) {
    return false;
  }
  int start = 0;
  int end = strlen(text);
  while (start < end && IsSpace(text[start])) {
    start++;
  }
  while (end > start && IsSpace(text[end - 1])) {
    end--;
  }
  if (text[start] != '(' || text[end - 1] != ')') {
    return false;
  }
  int comma = TopLevelComma(text + start + 1, end - start - 2);
  if (comma < 0) {
    return false;
  }
  comma += start + 1;
  int firstLength = comma - start - 1;
  int secondLength = end - comma - 2;
  if (firstLength <= 0 || secondLength <= 0) {
    return false;
  }
  size_t required = 4 + firstLength + secondLength;
  if (required >= bufferSize) {
    return false;
  }
  size_t written = 0;
  buffer[written++] = '[';
  buffer[written++] = '[';
  memcpy(buffer + written, text + start + 1, firstLength);
  written += firstLength;
  buffer[written++] = ']';
  buffer[written++] = '[';
  memcpy(buffer + written, text + comma + 1, secondLength);
  written += secondLength;
  buffer[written++] = ']';
  buffer[written++] = ']';
  buffer[written] = 0;
  return true;
}

void Piecewise::reset() {
  m_numberOfBranches = 0;
  m_parsed = false;
  for (int i = 0; i < k_maxNumberOfBranches; i++) {
    m_branches[i].expression = Expression();
    m_branches[i].left = Expression();
    m_branches[i].right = Expression();
    m_branches[i].hasCondition = false;
  }
}

bool Piecewise::parse(const char * text, Context * context) {
  if (m_parsed) {
    return isValid();
  }
  reset();
  if (!text) {
    return false;
  }
  int start = 0;
  int end = strlen(text);
  SkipSpaces(text, &start);
  while (end > start && IsSpace(text[end - 1])) {
    end--;
  }
  const char * source = text + start;
  int sourceLength = end - start;

  int argumentStarts[k_maxNumberOfArguments];
  int argumentLengths[k_maxNumberOfArguments];
  int numberOfArguments = 0;

  if (sourceLength > 10 && strncmp(source, "piecewise(", 10) == 0) {
    int closing = MatchingParenthesis(source, 9);
    if (closing < 0) {
      return false;
    }
    if (!SplitArguments(source + 10, closing - 10, argumentStarts,
                        argumentLengths, k_maxNumberOfArguments,
                        &numberOfArguments)) {
      return false;
    }
    for (int i = 0; i < numberOfArguments; i++) {
      argumentStarts[i] += 10;
    }
  } else {
    if (source[0] != '{') {
      return false;
    }
    int braceEnd = sourceLength;
    if (source[braceEnd - 1] == '}') {
      braceEnd--;
    }
    /* A double backslash separates the rows of the brace notation. Without
     * it, arguments are read as successive (expression, condition) pairs. */
    int rowStarts[k_maxNumberOfBranches];
    int rowLengths[k_maxNumberOfBranches];
    int numberOfRows = 0;
    int rowStart = 1;
    bool hasRowSeparator = false;
    for (int i = 1; i < braceEnd; i++) {
      if (source[i] == '\\' && i + 1 < braceEnd && source[i + 1] == '\\') {
        hasRowSeparator = true;
        if (numberOfRows < k_maxNumberOfBranches) {
          rowStarts[numberOfRows] = rowStart;
          rowLengths[numberOfRows] = i - rowStart;
          numberOfRows++;
        }
        rowStart = i + 2;
        i++;
      }
    }
    if (hasRowSeparator) {
      if (numberOfRows < k_maxNumberOfBranches) {
        rowStarts[numberOfRows] = rowStart;
        rowLengths[numberOfRows] = braceEnd - rowStart;
        numberOfRows++;
      }
      for (int row = 0; row < numberOfRows; row++) {
        int comma = TopLevelComma(source + rowStarts[row], rowLengths[row]);
        if (comma < 0) {
          if (numberOfArguments < k_maxNumberOfArguments) {
            argumentStarts[numberOfArguments] = rowStarts[row];
            argumentLengths[numberOfArguments++] = rowLengths[row];
          }
          continue;
        }
        if (numberOfArguments + 2 > k_maxNumberOfArguments) {
          return false;
        }
        argumentStarts[numberOfArguments] = rowStarts[row];
        argumentLengths[numberOfArguments++] = comma;
        argumentStarts[numberOfArguments] = rowStarts[row] + comma + 1;
        argumentLengths[numberOfArguments++] = rowLengths[row] - comma - 1;
      }
    } else if (!SplitArguments(source + 1, braceEnd - 1, argumentStarts,
                               argumentLengths, k_maxNumberOfArguments,
                               &numberOfArguments)) {
      return false;
    } else {
      for (int i = 0; i < numberOfArguments; i++) {
        argumentStarts[i] += 1;
      }
    }
  }

  for (int i = 0; i < numberOfArguments &&
                  m_numberOfBranches < k_maxNumberOfBranches;
       i++) {
    int expressionStart = argumentStarts[i];
    int expressionLength = argumentLengths[i];
    while (expressionLength > 0 && IsSpace(source[expressionStart])) {
      expressionStart++;
      expressionLength--;
    }
    while (expressionLength > 0 && IsSpace(source[expressionStart +
                                                       expressionLength - 1])) {
      expressionLength--;
    }
    if (expressionLength == 0) {
      continue;
    }
    Branch * branch = &m_branches[m_numberOfBranches];
    if (i + 1 < numberOfArguments) {
      /* Try to parse the next argument as a comparison condition. */
      int conditionStart = argumentStarts[i + 1];
      int conditionLength = argumentLengths[i + 1];
      while (conditionLength > 0 && IsSpace(source[conditionStart])) {
        conditionStart++;
        conditionLength--;
      }
      int comparisonLength = 0;
      Comparison ignored;
      int comparisonPosition = TopLevelComparison(
          source + conditionStart, conditionLength, &comparisonLength,
          &ignored);
      if (comparisonPosition > 0) {
        if (!ParseExpression(source + expressionStart, expressionLength,
                             context, &branch->expression) ||
            !ParseExpression(source + conditionStart, comparisonPosition,
                             context, &branch->left) ||
            !ParseExpression(source + conditionStart + comparisonPosition +
                                 comparisonLength,
                             conditionLength - comparisonPosition -
                                 comparisonLength,
                             context, &branch->right)) {
          reset();
          return false;
        }
        branch->comparison = ignored;
        branch->hasCondition = true;
        m_numberOfBranches++;
        i++;
        continue;
      }
    }
    if (!ParseExpression(source + expressionStart, expressionLength, context,
                         &branch->expression)) {
      reset();
      return false;
    }
    branch->hasCondition = false;
    m_numberOfBranches++;
  }
  m_parsed = true;
  return m_numberOfBranches > 0;
}

bool Piecewise::conditionHolds(const Branch & branch, double x,
                               Context * context) const {
  if (!branch.hasCondition) {
    return true;
  }
  double left = PoincareHelpers::ApproximateWithValueForSymbol<double>(
      branch.left, "x", x, context);
  double right = PoincareHelpers::ApproximateWithValueForSymbol<double>(
      branch.right, "x", x, context);
  if (!std::isfinite(left) || !std::isfinite(right)) {
    return false;
  }
  constexpr double epsilon = 1e-12;
  switch (branch.comparison) {
    case Comparison::Equal:
      return std::abs(left - right) <= epsilon;
    case Comparison::Inferior:
      return left < right;
    case Comparison::InferiorEqual:
      return left <= right + epsilon;
    case Comparison::Superior:
      return left > right;
    case Comparison::SuperiorEqual:
      return left >= right - epsilon;
  }
  return false;
}

double Piecewise::evaluate(double x, Context * context) const {
  if (!m_parsed) {
    return NAN;
  }
  for (int i = 0; i < m_numberOfBranches; i++) {
    const Branch & branch = m_branches[i];
    if (conditionHolds(branch, x, context)) {
      return PoincareHelpers::ApproximateWithValueForSymbol<double>(
          branch.expression, "x", x, context);
    }
  }
  return NAN;
}

}  // namespace FunctionEquation

}  // namespace Shared
