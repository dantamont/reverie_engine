#include "core/GSettings.h"
#include <fstream>
#include <QGuiApplication>

#include "fortress/containers/GSortingLayer.h"
#include "fortress/types/GString.h"
#include "core/GCoreEngine.h"
#include "core/resource/GFileManager.h"
#include "fortress/json/GJson.h"


/// @note OpenGL extensions viewer is a useful way of finding supported version
#define OPENGL_MAJOR_VERSION 4
#define OPENGL_MINOR_VERSION 4 // Seems to need to be 5, but unsure of why. Thinkpad supports OpenGL 4.4, but the app seems unhappy with it
#define OPENGL_ES_MAJOR_VERSION 3
#define OPENGL_ES_MINOR_VERSION 2

namespace rev {

GApplicationSettings::GApplicationSettings():
    LoadableInterface(FileManager::GetIniPath())
{
    // Get JSON from settings file
    GJson::FromFile(getPath(), s_settingsJson);
}

GApplicationSettings::~GApplicationSettings()
{
}

GString GApplicationSettings::getWorkingDirectory() const
{
    return value<GString>({ "system", "workingDirectory" });
}

GString GApplicationSettings::getRecentProject()
{
    return value<GString>({ "system", "mostRecentProject" }, "");
}

void GApplicationSettings::setRecentProject(const GString & filepath)
{
    setValue({ "system", "mostRecentProject" }, filepath);
}

Uint32_t GApplicationSettings::applicationGatewayReceivePort() const
{
    return value<Uint32_t>({"network", "gateways", "mainApplication", "receivePort"});
}

Uint32_t GApplicationSettings::widgetGatewayReceivePort() const
{
    return value<Uint32_t>({ "network", "gateways", "widgets", "receivePort" });
}

int GApplicationSettings::getMajorVersion()
{
    return value<Int32_t>({ "rendering", "renderVersionMain" }, OPENGL_MAJOR_VERSION);
}

int GApplicationSettings::getMinorVersion()
{
    return value<Int32_t>({ "rendering", "renderVersionMinor" }, OPENGL_MINOR_VERSION);
}

RenderingBackend GApplicationSettings::getRenderingBackend()
{
    return RenderingBackend(value<Int32_t>({ "rendering", "renderBackend" }, 0));
}

QSurfaceFormat GApplicationSettings::configOpenGLES()
{
	QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);

    setRenderingBackend(RenderingBackend::kGL_ES, OPENGL_ES_MAJOR_VERSION, OPENGL_ES_MINOR_VERSION);

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

QSurfaceFormat GApplicationSettings::configOpenGLESAngle()
{
	setRenderingBackend(RenderingBackend::kGL_ES, OPENGL_ES_MAJOR_VERSION, OPENGL_ES_MINOR_VERSION);

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

QSurfaceFormat GApplicationSettings::configOpenGL()
{
    setRenderingBackend(RenderingBackend::kGL, OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION);

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

void GApplicationSettings::setRenderingBackend(RenderingBackend mode, int major, int minor)
{
    setValue({ "rendering", "renderBackend" }, int(mode));
    setValue({ "rendering", "renderVersionMain" }, major);
    setValue({ "rendering", "renderVersionMinor" }, minor);
}

void GApplicationSettings::write()
{
    // Write to file, with indents and a trailing newline
    std::ofstream file(getPath().c_str());
    file << std::setw(3) << s_settingsJson << std::endl;;
}

json GApplicationSettings::s_settingsJson{};

std::shared_mutex GApplicationSettings::s_mutex{};

} // End rev namespace