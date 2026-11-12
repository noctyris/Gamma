#ifndef FINANCE_TVM_STORE_H
#define FINANCE_TVM_STORE_H

#include <apps/i18n.h>
#include <stdint.h>

namespace Finance {

class TvmStore {
public:
  enum class Variable : uint8_t {
    N = 0,
    IY,
    PV,
    PMT,
    FV,
    PY,
    CY,
    Count
  };

  static constexpr int k_numberOfTvmVariables = 5;
  static constexpr int k_numberOfVariables = 7;

  TvmStore();

  void reset();
  double value(Variable variable) const;
  void setValue(Variable variable, double value);

  Variable unknown() const { return m_unknown; }
  void setUnknown(Variable variable);

  bool compute();
  double result() const { return m_result; }

  static bool isTvmVariable(Variable variable) { return variable <= Variable::FV; }
  static I18n::Message labelMessage(Variable variable);

private:
  double periodicRate() const;
  double balanceAtRate(double rate) const;
  bool solveN(double rate);
  bool solveIY();
  bool solvePV(double rate);
  bool solvePMT(double rate);
  bool solveFV(double rate);
  bool setResult(double result);

  double m_values[k_numberOfVariables];
  Variable m_unknown;
  double m_result;
};

}

#endif
