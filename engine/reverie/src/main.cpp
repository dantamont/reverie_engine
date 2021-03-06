#include "GMainWindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QLibrary>
#include "core/GSettings.h"
#include "view/style/GStyles.h"

// Visual memory leak detection
// See: https://kinddragon.github.io/vld/
// FIXME: There ARE leaks, causing occasional crashes. Find them~
// VLD is causing crashes, unfortunate
//#include <vld.h>

/// @brief Sets GL settings for the application
/// See: https://stackoverflow.com/questions/40385482/why-cant-i-use-opengl-es-3-0-in-qt
static QSurfaceFormat createSurfaceFormat() {
    // Store settings
    auto* settings = new rev::Settings::INISettings();
    // Set rendering mode via format
    QSurfaceFormat format;
    #if defined(QT_OPENGL_ES_3)
        format = settings->configOpenGLES();
    #elif defined(QT_OPENGL_ES_2)
        throw("Error, Open GL ES 2.0 is not supported by this hardware");
	//#elif defined(QT_OPENGL_ES_2_ANGLE)	
	//	// Actually use Open GL ES 3.0 with angle
	//	format = settings->configOpenGLESAngle();
	#else
		format = settings->configOpenGL();
    #endif
	
    delete settings;
    return format;
}

int main(int argc, char *argv[])
{
	// Initialize GL settings /////////////////////////////////
	// See: https://stackoverflow.com/questions/41451089/blank-window-when-using-qt-with-angle-on-win-7
	//QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QSurfaceFormat::setDefaultFormat(createSurfaceFormat());

    // Create Qt application //////////////////////////////////
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents); // To fix missed mouse release events
	QApplication application(argc, argv);
    application.setStyle(new rev::DarkStyle);

    // Load all resources /////////////////////////////////////
    Q_INIT_RESOURCE(shaders);

    // Create main window /////////////////////////////////////
	rev::MainWindow w(argc, argv);
	w.showMaximized();

    // Run main window ////////////////////////////////////////
	return application.exec();
}
