apps += Finance::App
app_headers += apps/finance/app.h

app_finance_test_src = $(addprefix apps/finance/,\
  tvm_store.cpp\
)

app_finance_src = $(addprefix apps/finance/,\
  app.cpp\
  parameters_controller.cpp\
  result_controller.cpp\
  variable_controller.cpp\
)

app_finance_src += $(app_finance_test_src)
apps_src += $(app_finance_src)

i18n_files += $(addprefix apps/finance/,\
  base.de.i18n\
  base.en.i18n\
  base.es.i18n\
  base.fr.i18n\
  base.it.i18n\
  base.nl.i18n\
  base.pt.i18n\
  base.hu.i18n\
)

tests_src += apps/finance/test/tvm_store.cpp

$(eval $(call depends_on_image,apps/finance/app.cpp,apps/finance/finance_icon.png))
