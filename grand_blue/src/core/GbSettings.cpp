#include "GbSettings.h"
#include <QDir>
#include <QGuiApplication>

#include "../core/containers/GbSortingLayer.h"
#include "../core/GbCoreEngine.h"

namespace Gb { 
namespace Settings { 

/////////////////////////////////////////////////////////////////////////////////////////////
const QString INISettings::GetINIFile() {
    // FIXME: Use directory manager, if currentPath changes, this will be wonky
    return CoreEngine::GetRootPath() + "/grand_blue.ini";
}

/////////////////////////////////////////////////////////////////////////////////////////////
INISettings::INISettings(QObject *parent):
    QSettings(GetINIFile(), QSettings::IniFormat, parent)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
INISettings::~INISettings()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString INISettings::getRecentProject()
{
    return value("mostRecentProject", "").toString();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void INISettings::setRecentProject(const QString & filepath)
{
    setValue("mostRecentProject", filepath);
}
/////////////////////////////////////////////////////////////////////////////////////////////
int INISettings::getMajorVersion()
{
    return value("renderVersionMain", OPENGL_MAJOR_VERSION).toInt();
}
/////////////////////////////////////////////////////////////////////////////////////////////
int INISettings::getMinorVersion()
{
    return value("renderVersionMinor", OPENGL_MINOR_VERSION).toInt();
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderingMode INISettings::getRenderingMode()
{
    return RenderingMode(value("renderMode", 0).toInt());
}
/////////////////////////////////////////////////////////////////////////////////////////////
QSurfaceFormat INISettings::configOpenGLES()
{
	QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);

    setRenderingMode(Gb::Settings::kGL_ES, OPENGL_ES_MAJOR_VERSION, OPENGL_ES_MINOR_VERSION);

    // Set format, which specifies:

    QSurfaceFormat format;    // GL Version
    // Minimum depth buffer size
    format.setVersion(getMajorVersion(), getMinorVersion());
    format.setSamples(4);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);

    return format;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QSurfaceFormat INISettings::configOpenGLESAngle()
{
	setRenderingMode(Gb::Settings::kGL_ES, OPENGL_ES_MAJOR_VERSION, OPENGL_ES_MINOR_VERSION);

	// Set format, which specifies:
	// GL Version
	// Minimum depth buffer size
	QSurfaceFormat format;
	format.setVersion(getMajorVersion(), getMinorVersion());
	format.setSamples(4);
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setProfile(QSurfaceFormat::CompatibilityProfile);

	return format;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QSurfaceFormat INISettings::configOpenGL()
{
    setRenderingMode(Gb::Settings::kGL, OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION);

    // Set format, which specifies:
    // GL Version
    // Minimum depth buffer size
    QSurfaceFormat format;
    format.setVersion(getMajorVersion(), getMinorVersion());
    format.setSamples(4);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);

    return format;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void INISettings::setRenderingMode(RenderingMode mode, int major, int minor)
{
    setValue("renderMode", int(mode));
    setValue("renderVersionMain", major);
    setValue("renderVersionMinor", minor);
}


// End namespacing
}
}