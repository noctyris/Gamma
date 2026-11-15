#include "tvm_store.h"
#include <cmath>

namespace Finance {

TvmStore::TvmStore() {
  reset();
}

void TvmStore::reset() {
  for (int i = 0; i < k_numberOfVariables; i++) {
    m_values[i] = 0.0;
  }
  m_values[static_cast<int>(Variable::PY)] = 1.0;
  m_values[static_cast<int>(Variable::CY)] = 1.0;
  m_unknown = Variable::N;
  m_result = 0.0;
}

double TvmStore::value(Variable variable) const {
  return m_values[static_cast<int>(variable)];
}

void TvmStore::setValue(Variable variable, double value) {
  if (variable < Variable::Count) {
    m_values[static_cast<int>(variable)] = value;
  }
}

void TvmStore::setUnknown(Variable variable) {
  if (isTvmVariable(variable)) {
    m_unknown = variable;
  }
}

I18n::Message TvmStore::labelMessage(Variable variable) {
  switch (variable) {
    case Variable::N:
      return I18n::Message::FinanceN;
    case Variable::IY:
      return I18n::Message::FinanceIY;
    case Variable::PV:
      return I18n::Message::FinancePV;
    case Variable::PMT:
      return I18n::Message::FinancePMT;
    case Variable::FV:
      return I18n::Message::FinanceFV;
    case Variable::PY:
      return I18n::Message::FinancePY;
    case Variable::CY:
      return I18n::Message::FinanceCY;
    default:
      return I18n::Message::FinanceTvm;
  }
}

double TvmStore::periodicRate() const {
  const double nominalRate = value(Variable::IY) / 100.0;
  const double compoundsPerYear = value(Variable::CY);
  const double paymentsPerYear = value(Variable::PY);
  if (compoundsPerYear <= 0.0 || paymentsPerYear <= 0.0) {
    return NAN;
  }
  const double ratePerCompounding = nominalRate / compoundsPerYear;
  if (ratePerCompounding <= -1.0) {
    return NAN;
  }
  return std::pow(1.0 + ratePerCompounding, compoundsPerYear / paymentsPerYear) - 1.0;
}

double TvmStore::balanceAtRate(double rate) const {
  const double numberOfPayments = value(Variable::N);
  const double presentValue = value(Variable::PV);
  const double payment = value(Variable::PMT);
  const double futureValue = value(Variable::FV);
  if (numberOfPayments < 0.0 || rate <= -1.0 || std::isnan(rate)) {
    return NAN;
  }
  if (std::fabs(rate) < 1e-10) {
    return presentValue + payment * numberOfPayments + futureValue;
  }
  const double growth = std::pow(1.0 + rate, numberOfPayments);
  return presentValue * growth + payment * (growth - 1.0) / rate + futureValue;
}

bool TvmStore::setResult(double result) {
  if (!std::isfinite(result)) {
    return false;
  }
  m_result = result;
  return true;
}

bool TvmStore::solveN(double rate) {
  const double presentValue = value(Variable::PV);
  const double payment = value(Variable::PMT);
  const double futureValue = value(Variable::FV);
  if (std::fabs(rate) < 1e-10) {
    if (std::fabs(payment) < 1e-12) {
      return false;
    }
    return setResult(-(presentValue + futureValue) / payment);
  }
  const double paymentOverRate = payment / rate;
  const double denominator = presentValue + paymentOverRate;
  if (std::fabs(denominator) < 1e-12) {
    return false;
  }
  const double ratio = (paymentOverRate - futureValue) / denominator;
  if (ratio <= 0.0) {
    return false;
  }
  const double numberOfPayments = std::log(ratio) / std::log(1.0 + rate);
  if (numberOfPayments < 0.0) {
    return false;
  }
  return setResult(numberOfPayments);
}

bool TvmStore::solveIY() {
  if (value(Variable::N) <= 0.0) {
    return false;
  }

  double low = -0.999999;
  double high = 10.0;
  double lowBalance = balanceAtRate(low);
  double highBalance = balanceAtRate(high);
  if (std::isnan(lowBalance) || std::isnan(highBalance)) {
    return false;
  }

  if (lowBalance * highBalance > 0.0) {
    bool bracketed = false;
    double previousRate = low;
    double previousBalance = lowBalance;
    for (int i = 1; i <= 2000; i++) {
      const double rate = low + (high - low) * i / 2000.0;
      const double balance = balanceAtRate(rate);
      if (std::isnan(balance)) {
        continue;
      }
      if (previousBalance * balance <= 0.0) {
        low = previousRate;
        high = rate;
        lowBalance = previousBalance;
        highBalance = balance;
        bracketed = true;
        break;
      }
      previousRate = rate;
      previousBalance = balance;
    }
    if (!bracketed) {
      return false;
    }
  }

  for (int i = 0; i < 200; i++) {
    const double middle = (low + high) / 2.0;
    const double middleBalance = balanceAtRate(middle);
    if (std::isnan(middleBalance)) {
      return false;
    }
    if (lowBalance * middleBalance <= 0.0) {
      high = middle;
      highBalance = middleBalance;
    } else {
      low = middle;
      lowBalance = middleBalance;
    }
  }

  const double periodicRateValue = (low + high) / 2.0;
  const double paymentsPerYear = value(Variable::PY);
  const double compoundsPerYear = value(Variable::CY);
  if (paymentsPerYear <= 0.0 || compoundsPerYear <= 0.0) {
    return false;
  }
  const double annualRate = (std::pow(1.0 + periodicRateValue, paymentsPerYear / compoundsPerYear) - 1.0) * compoundsPerYear * 100.0;
  return setResult(annualRate);
}

bool TvmStore::solvePV(double rate) {
  const double numberOfPayments = value(Variable::N);
  const double payment = value(Variable::PMT);
  const double futureValue = value(Variable::FV);
  if (numberOfPayments < 0.0) {
    return false;
  }
  if (std::fabs(rate) < 1e-10) {
    return setResult(-futureValue - payment * numberOfPayments);
  }
  const double growth = std::pow(1.0 + rate, numberOfPayments);
  if (std::fabs(growth) < 1e-12) {
    return false;
  }
  return setResult((-futureValue - payment * (growth - 1.0) / rate) / growth);
}

bool TvmStore::solvePMT(double rate) {
  const double numberOfPayments = value(Variable::N);
  const double presentValue = value(Variable::PV);
  const double futureValue = value(Variable::FV);
  if (numberOfPayments < 0.0) {
    return false;
  }
  if (std::fabs(rate) < 1e-10) {
    if (numberOfPayments == 0.0) {
      return false;
    }
    return setResult((-futureValue - presentValue) / numberOfPayments);
  }
  const double growth = std::pow(1.0 + rate, numberOfPayments);
  const double annuityFactor = (growth - 1.0) / rate;
  if (std::fabs(annuityFactor) < 1e-12) {
    return false;
  }
  return setResult((-futureValue - presentValue * growth) / annuityFactor);
}

bool TvmStore::solveFV(double rate) {
  const double numberOfPayments = value(Variable::N);
  const double presentValue = value(Variable::PV);
  const double payment = value(Variable::PMT);
  if (numberOfPayments < 0.0) {
    return false;
  }
  if (std::fabs(rate) < 1e-10) {
    return setResult(-(presentValue + payment * numberOfPayments));
  }
  const double growth = std::pow(1.0 + rate, numberOfPayments);
  return setResult(-(presentValue * growth + payment * (growth - 1.0) / rate));
}

bool TvmStore::compute() {
  const double rate = periodicRate();
  if (std::isnan(rate)) {
    return false;
  }

  bool solved = false;
  switch (m_unknown) {
    case Variable::N:
      solved = solveN(rate);
      break;
    case Variable::IY:
      solved = solveIY();
      break;
    case Variable::PV:
      solved = solvePV(rate);
      break;
    case Variable::PMT:
      solved = solvePMT(rate);
      break;
    case Variable::FV:
      solved = solveFV(rate);
      break;
    default:
      break;
  }

  if (!solved) {
    return false;
  }
  m_values[static_cast<int>(m_unknown)] = m_result;
  return true;
}

}
