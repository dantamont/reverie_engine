#include "GMainWindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QLibrary>

#include "core/resource/GFileManager.h"

#include "core/GSettings.h"
#include "geppetto/qt/style/GStyles.h"

// Visual memory leak detection
/// \see https://kinddragon.github.io/vld/
// FIXME: There ARE leaks, causing occasional crashes. Find them~
// VLD is causing crashes, unfortunate
//#include <vld.h>

/// @brief Sets GL settings for the application
//// \see https://stackoverflow.com/questions/40385482/why-cant-i-use-opengl-es-3-0-in-qt
static QSurfaceFormat createSurfaceFormat() {
    // Store settings
    rev::GApplicationSettings& settings = rev::GApplicationSettings::Instance();
    // Set rendering mode via format
    QSurfaceFormat format;
    #if defined(QT_OPENGL_ES_3)
        format = settings->configOpenGLES();
    #elif defined(QT_OPENGL_ES_2)
        Logger::Throw("Error, Open GL ES 2.0 is not supported by this hardware");
	//#elif defined(QT_OPENGL_ES_2_ANGLE)	
	//	// Actually use Open GL ES 3.0 with angle
	//	format = settings->configOpenGLESAngle();
	#else
		format = settings.configOpenGL();
    #endif
	
    return format;
}

int main(int argc, char *argv[])
{
    /// Initialize application paths
    rev::FileManager::SetApplicationPaths(argc, argv);

	/// Initialize GL settings 
	/// \see https://stackoverflow.com/questions/41451089/blank-window-when-using-qt-with-angle-on-win-7
	//QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QSurfaceFormat::setDefaultFormat(createSurfaceFormat());

    /// Create Qt application 
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents); // To fix missed mouse release events
	QApplication application(argc, argv);
    application.setStyle(new rev::DarkStyle);

    /// Load all resources 
    Q_INIT_RESOURCE(fonts);
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(scripts);
    Q_INIT_RESOURCE(shaders);
    Q_INIT_RESOURCE(styles);
    Q_INIT_RESOURCE(textures);

    /// Create main window 
	rev::MainWindow w(argc, argv);
	w.showMaximized();

    /// Run main window 
	return application.exec();
}
