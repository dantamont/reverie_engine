#include "GbSettings.h"
#include <QDir>
#include <QGuiApplication>

#include "../core/containers/GbSortingLayer.h"

namespace Gb { 
namespace Settings { 

/////////////////////////////////////////////////////////////////////////////////////////////

const QString INISettings::INI_FILE(QDir::currentPath() + "/grand_blue.ini");

/////////////////////////////////////////////////////////////////////////////////////////////
INISettings::INISettings(QObject *parent):
    QSettings(INI_FILE, QSettings::IniFormat, parent)
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
    return value("renderVersionMain", 3).toInt();
}
/////////////////////////////////////////////////////////////////////////////////////////////
int INISettings::getMinorVersion()
{
    return value("renderVersionMinor", 2).toInt();
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

    setRenderingMode(Gb::Settings::kGL_ES, 3, 2);

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
QSurfaceFormat INISettings::configOpenGLESAngle()
{
	setRenderingMode(Gb::Settings::kGL_ES, 3, 2);

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
    setRenderingMode(Gb::Settings::kGL, 4, 3);

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