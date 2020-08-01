#include <PythonQt.h>
#include <PythonQtConversion.h>
#include "com_trolltech_qt_webenginewidgets0.h"



void PythonQt_init_QtWebEngineWidgets(PyObject* module) {
PythonQt::priv()->registerCPPClass("QWebEngineCertificateError", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineCertificateError>, NULL, module, 0);
PythonQt::priv()->registerClass(&QWebEngineCookieStore::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineCookieStore>, NULL, module, 0);
PythonQt::priv()->registerClass(&QWebEngineDownloadItem::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineDownloadItem>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QWebEngineFullScreenRequest", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineFullScreenRequest>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QWebEngineHistory", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineHistory>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QWebEngineHistoryItem", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineHistoryItem>, NULL, module, PythonQt::Type_NonZero);
PythonQt::priv()->registerClass(&QWebEnginePage::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEnginePage>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QWebEnginePage>, module, 0);
PythonQt::priv()->registerClass(&QWebEngineProfile::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineProfile>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QWebEngineProfile>, module, 0);
PythonQt::priv()->registerCPPClass("QWebEngineScript", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineScript>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QWebEngineScriptCollection", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineScriptCollection>, NULL, module, PythonQt::Type_NonZero);
PythonQt::priv()->registerCPPClass("QWebEngineSettings", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineSettings>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QWebEngineUrlRequestInfo", "", "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineUrlRequestInfo>, NULL, module, 0);
PythonQt::priv()->registerClass(&QWebEngineUrlRequestInterceptor::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineUrlRequestInterceptor>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QWebEngineUrlRequestInterceptor>, module, 0);
PythonQt::priv()->registerClass(&QWebEngineUrlRequestJob::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineUrlRequestJob>, NULL, module, 0);
PythonQt::priv()->registerClass(&QWebEngineUrlSchemeHandler::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineUrlSchemeHandler>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QWebEngineUrlSchemeHandler>, module, 0);
PythonQt::priv()->registerClass(&QWebEngineView::staticMetaObject, "QtWebEngineWidgets", PythonQtCreateObject<PythonQtWrapper_QWebEngineView>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QWebEngineView>, module, 0);


PythonQtRegisterListTemplateConverterForKnownClass(QList, QWebEngineHistoryItem);
PythonQtRegisterListTemplateConverterForKnownClass(QList, QWebEngineScript);
}
