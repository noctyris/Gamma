#include <quiz.h>
#include <cmath>
#include "../tvm_store.h"

QUIZ_CASE(finance_tvm) {
  Finance::TvmStore store;
  store.setValue(Finance::TvmStore::Variable::N, 360.0);
  store.setValue(Finance::TvmStore::Variable::IY, 5.0);
  store.setValue(Finance::TvmStore::Variable::PV, 200000.0);
  store.setValue(Finance::TvmStore::Variable::FV, 0.0);
  store.setValue(Finance::TvmStore::Variable::PY, 12.0);
  store.setValue(Finance::TvmStore::Variable::CY, 12.0);
  store.setUnknown(Finance::TvmStore::Variable::PMT);

  quiz_assert(store.compute());
  quiz_assert(std::fabs(store.result() + 1073.64) < 0.1);

  store.setUnknown(Finance::TvmStore::Variable::N);
  quiz_assert(store.compute());
  quiz_assert(std::fabs(store.result() - 360.0) < 1e-6);
}
