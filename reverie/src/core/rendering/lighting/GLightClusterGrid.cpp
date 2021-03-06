#include "GLightClusterGrid.h"
#include "GLightSettings.h"

#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../renderer/GMainRenderer.h"
#include "../../components/GCameraComponent.h"
#include "../../view/GL/GGLWidget.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../resource/GResourceCache.h"
#include "../shaders/GShaderProgram.h"
#include "../buffers/GUniformBufferObject.h"

namespace rev {   
/////////////////////////////////////////////////////////////////////////////////////////////
LightClusterGrid::LightClusterGrid(SceneCamera * camera) :
    m_camera(camera),
    m_maxLightsPerTile(100)
{
    m_clusterGridShaderProgram = camera->m_component->sceneObject()->scene()->engine()->resourceCache()->getHandleWithName("light_cluster_grid",
        ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
LightClusterGrid::~LightClusterGrid()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::bindBufferPoints()
{
    //std::vector<AABBData> aabbs;
    //int sizeAABB = sizeof(AABBData);
    //m_lightClusterBuffer->copyData(aabbs, 16*9*24);

    //std::vector<uint> indices;
    //m_activeLightIndexBuffer->copyData(indices, 50);

    //std::vector<ScreenToView> s2v;
    //m_screenToViewBuffer->copyData(s2v, 1);

    //struct Grid {
    //    uint offset;
    //    uint count;
    //};

    //std::vector<Grid> grids;
    //m_lightClusterGridBuffer->copyData(grids, 10);

    //std::vector<uint> count;
    //m_globalLightCountBuffer->copyData(count, 1);

    m_lightClusterBuffer->bindToPoint();
    m_screenToViewBuffer->bindToPoint();
    m_activeLightIndexBuffer->bindToPoint();
    m_lightClusterGridBuffer->bindToPoint();
    m_globalLightCountBuffer->bindToPoint();
    m_activeClusterBuffer->bindToPoint();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::initializeBuffers(RenderContext & rc)
{
    // FIXME: Don't hard-code this
    const unsigned int numClusters = m_gridSizeX * m_gridSizeY * m_gridSizeZ;

    // Buffer containing all the clusters
    {
        size_t size = sizeof(AABBData);
        size *= numClusters;
        m_lightClusterBuffer = std::make_shared<ShaderStorageBuffer>(rc, size);
        m_lightClusterBuffer->setBindPoint(1);
        m_lightClusterBuffer->release();
    }

    //Setting up screen2View ssbo
    {
        size_t size = sizeof(ScreenToView);
        m_screenToViewBuffer = std::make_shared<ShaderStorageBuffer>(rc, size);
        m_screenToViewBuffer->setBindPoint(2);
        m_screenToViewBuffer->release();
    }

    // TODO: Pretty big memory overhead here, especially for each camera
    // See call of duty light culling paper as a reference for multiple cameras
    //A list of indices to the lights that are active and intersect with a cluster
    {
        unsigned int totalNumLights = numClusters * m_maxLightsPerTile;
        size_t size = totalNumLights * sizeof(unsigned int);
        m_activeLightIndexBuffer = std::make_shared<ShaderStorageBuffer>(rc, size);
        m_activeLightIndexBuffer->setBindPoint(4);
        m_activeLightIndexBuffer->release();
    }

    //Every tile takes two unsigned ints one to represent the number of lights in that grid
    //Another to represent the offset to the light index list from where to begin reading light indexes from
    //This implementation is straight up from Olsson paper
    {
        size_t size = numClusters * 2 * sizeof(unsigned int);
        m_lightClusterGridBuffer = std::make_shared<ShaderStorageBuffer>(rc, size);
        m_lightClusterGridBuffer->setBindPoint(5);
        m_lightClusterGridBuffer->release();
    }

    //Setting up simplest ssbo in the world (global light count)
    {
        size_t size = 3 * sizeof(unsigned int);
        m_globalLightCountBuffer = std::make_shared<ShaderStorageBuffer>(rc, size);
        m_globalLightCountBuffer->setBindPoint(6);

        // Set max light count in buffer
        m_globalLightCountBuffer->data<Vector<unsigned int, 3>>()[0][1] = m_maxLightsPerTile;
        
        m_globalLightCountBuffer->unmap(false);
        m_globalLightCountBuffer->release();
    }

    // Set up buffer of active clusters (visible clusters)
    {
        size_t size = numClusters * sizeof(unsigned int);
        m_activeClusterBuffer = std::make_shared<ShaderStorageBuffer>(rc, size);
        m_activeClusterBuffer->setBindPoint(7);
        m_activeClusterBuffer->release();
    }

    // Populate screenToView buffer
    onResize();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::cullLights(MainRenderer & renderer)
{
    // Bind all required buffers ===============================================

    // Return if camera framebuffer uninitialized
    if (!m_camera->frameBuffers().writeBuffer().hasSize()) {
        return;
    }

    // Bind UBO for the camera, since shader needs view matrix
    m_camera->bindUniforms(&renderer);

    // Bind all required buffers from points
    UBO::getLightSettingsBuffer()->bind();
    ShaderStorageBuffer& readBuffer = renderer.renderContext().lightingSettings().lightBuffers().readBuffer();
    readBuffer.bindToPoint();
    m_lightClusterBuffer->bindToPoint();
    m_screenToViewBuffer->bindToPoint();
    m_activeLightIndexBuffer->bindToPoint();
    m_lightClusterGridBuffer->bindToPoint();
    m_globalLightCountBuffer->bindToPoint();
    m_activeClusterBuffer->bindToPoint();

    // Determine active clusters ===============================================
    // FIXME: Active clusters are breaking display for secondary cameras, due to different sizing
    ShaderProgram* activeClusterShader = renderer.getActiveClusterShader();

    ////std::vector<float> depths;
    ////m_camera->frameBuffer().readDepthPixels(depths);
    // FIXME: Need to clear active cluster buffer, at least while not compacting list

    activeClusterShader->bind();
    activeClusterShader->setUniformValue("depthTexture", 0);
    m_camera->frameBuffers().writeBuffer().bindDepthTexture(0); // Bind depth buffer
    activeClusterShader->dispatchCompute(renderer.widget()->width(), renderer.widget()->height(), 1);

    // TODO: Compact active cluster list ---------------------------------------------
    // Clear global active cluster count
    //Vector<unsigned int, 3>* data = m_globalLightCountBuffer->data< Vector<unsigned int, 3>>();
    //data[0][2] = 0;
    //m_globalLightCountBuffer->unmap(false);

    //std::shared_ptr<ShaderProgram> compactShader = renderer.getActiveClusterCompressShader();
    //compactShader->bind();
    //compactShader->dispatchCompute(1, 1, 6);

    //std::vector<Vector<unsigned int, 3>> globalCount;
    //m_globalLightCountBuffer->copyData(globalCount, 1);

    ////std::vector<unsigned int> activeClusters;
    ////m_activeClusterBuffer->copyData(activeClusters, m_gridSizeX * m_gridSizeY * m_gridSizeZ);

    // Perform light culling ===================================================

    //std::vector<Light> lights;
    //int sizeLight = sizeof(Light);
    //renderer.renderContext().lightingSettings().lightBuffer().copyData(lights, 25);
    //for (const auto& l : lights) {
    //    logInfo(QString(l.getPosition()));
    //}

    ShaderProgram* cullingShader = renderer.getLightCullingShader();

    cullingShader->bind();
    //cullingShader->setUniformValue("viewMatrix", m_camera->getViewMatrix());
    cullingShader->dispatchCompute(1, 1, 12);



    // Unbind buffers from points ==============================================
    readBuffer.releaseFromPoint();
    m_lightClusterBuffer->releaseFromPoint();
    m_screenToViewBuffer->releaseFromPoint();
    m_activeLightIndexBuffer->releaseFromPoint();
    m_lightClusterGridBuffer->releaseFromPoint();
    m_globalLightCountBuffer->releaseFromPoint();
    m_activeClusterBuffer->releaseFromPoint();

    //std::vector<size_t> visibleLights;
    //m_activeLightIndexBuffer->copyData(visibleLights, 1000);
    //std::set<size_t> set(visibleLights.begin(), visibleLights.end());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::onResize()
{
    // Get main widget dimensions to use
    View::GLWidget* mainGLWidget = m_camera->m_component->sceneObject()->scene()->engine()->mainRenderer()->widget();
    QSize widgetSize = mainGLWidget->size();

    if (!mainGLWidget->renderer()->renderContext().isCurrent()) {
        throw("Error, context not current");
    }

    // Update screen to view buffer
    updateScreenToView(widgetSize.width(), widgetSize.height());

    // Reconstruct light cluster grid
    // Necessary, since grid uses widget dimensions
    reconstructGrid();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::updateScreenToView(unsigned int widgetWidth, unsigned int widgetHeight)
{
    // Setting up tile size on both X and Y 
    // How many pixels in x does a square tile use
    float vw = m_camera->viewport().m_width;
    float vh = m_camera->viewport().m_height;
    unsigned int sizeX = (unsigned int)std::ceilf(float(widgetWidth * vw) / (float)m_gridSizeX);
    unsigned int sizeY = (unsigned int)std::ceilf(float(widgetHeight * vh) / (float)m_gridSizeY);

    // Create screen to view struct and set in SSBO
    // Map buffer and get pointer to data
    ScreenToView& screen2View = m_screenToViewBuffer->data<ScreenToView>()[0];

    // Populate data
    screen2View.m_inverseProjection = m_camera->renderProjection().inverseProjectionMatrix();
    screen2View.m_tileGridDimensions[0] = m_gridSizeX;
    screen2View.m_tileGridDimensions[1] = m_gridSizeY;
    screen2View.m_tileGridDimensions[2] = m_gridSizeZ;
    screen2View.m_tilePixelSizes[0] = sizeX;
    screen2View.m_tilePixelSizes[1] = sizeY;

    //screen2View.m_widgetDimensions[0] = Renderable::screenDimensions().width();
    //screen2View.m_widgetDimensions[1] = Renderable::screenDimensions().height();
    screen2View.m_widgetDimensions[0] = unsigned int(widgetWidth * vw);
    screen2View.m_widgetDimensions[1] = unsigned int(widgetHeight * vh);

    // Unmap buffer
    m_screenToViewBuffer->unmap(true);

    // Verify value
#ifdef DEBUG_MODE
    ScreenToView& viewToView = m_screenToViewBuffer->data<ScreenToView>()[0];
    if (viewToView.m_widgetDimensions.x() != unsigned int(widgetWidth * vw)
        ||
        viewToView.m_widgetDimensions.y() != unsigned int(widgetHeight * vh)) {
        throw("Error, mismatch in widget dimensions");
    }
    m_screenToViewBuffer->unmap(true);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::reconstructGrid()
{
    if (!m_lightClusterBuffer) return; // Buffers not yet initialized
    if (!m_clusterGridShaderProgram->handle()->isConstructed()) {
        return;
    }

    // Get main widget dimensions to use
    View::GLWidget* mainGLWidget = m_camera->m_component->sceneObject()->scene()->engine()->mainRenderer()->widget();
    QSize widgetSize = mainGLWidget->size();

    if (!mainGLWidget->renderer()->renderContext().isCurrent()) {
        mainGLWidget->renderer()->renderContext().makeCurrent();
    }
    float zFar = m_camera->renderProjection().farClipPlane();
    float zNear = m_camera->renderProjection().nearClipPlane();
    m_camera->bindUniforms(mainGLWidget->renderer().get());

    // Update scale and bias for screenToView SSBO
    // Basically reduced a log function into a simple multiplication an addition by pre-calculating these
    ScreenToView& screen2View = m_screenToViewBuffer->data<ScreenToView>()[0];
    screen2View.m_sliceScalingFactor = (float)m_gridSizeZ / std::log2f(zFar / zNear);
    screen2View.m_sliceBiasFactor = -((float)m_gridSizeZ * std::log2f(zNear) / std::log2f(zFar / zNear));
    m_screenToViewBuffer->unmap(true);

    //if (m_lightingFlags.testFlag(kClustered)) {
    // Building the grid of AABB enclosing the view frustum clusters

    m_lightClusterBuffer->bindToPoint();
    m_screenToViewBuffer->bindToPoint();
    m_clusterGridShaderProgram->bind();
    //m_clusterGridShaderProgram->setUniformValue("zNear", zNear);
    //m_clusterGridShaderProgram->setUniformValue("zFar", zFar);
    m_clusterGridShaderProgram->dispatchCompute(m_gridSizeX, m_gridSizeY, m_gridSizeZ);
    m_lightClusterBuffer->releaseFromPoint();
    m_screenToViewBuffer->releaseFromPoint();

    //std::vector<AABBData> aabbs;
    //int sizeAABB = sizeof(AABBData);
    //m_lightClusterBuffer->copyData(aabbs, m_gridSizeX * m_gridSizeY * m_gridSizeZ);
    //m_lightClusterBuffer = m_lightClusterBuffer;

    // }
}
/////////////////////////////////////////////////////////////////////////////////////////////
LightClusterGrid::ScreenToView LightClusterGrid::getScreenToView() const
{
    std::vector<ScreenToView> s2v;
    m_screenToViewBuffer->copyData(s2v, 1);
    return s2v[0];
}
/////////////////////////////////////////////////////////////////////////////////////////////
void LightClusterGrid::getGridBoxes(std::vector<AABBData>& outBoxes) const
{
    m_lightClusterBuffer->copyData(outBoxes, m_gridSizeX * m_gridSizeY * m_gridSizeZ);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool LightClusterGrid::ScreenToView::operator==(const ScreenToView & other) const
{
    bool equal = true;
    equal &= m_inverseProjection == other.m_inverseProjection;
    equal &= m_tileGridDimensions == other.m_tileGridDimensions;
    equal &= m_tilePixelSizes == other.m_tilePixelSizes;
    equal &= m_widgetDimensions == other.m_widgetDimensions;
    equal &= m_sliceBiasFactor == other.m_sliceBiasFactor;
    equal &= m_sliceScalingFactor == other.m_sliceScalingFactor;

    return equal;
}
/////////////////////////////////////////////////////////////////////////////////////////////
LightClusterGrid::ScreenToView::operator QString() const
{
    QString string;
    QString newLine = QStringLiteral("\n");
    string += QString("Inv Proj: ") + QString(m_inverseProjection) + newLine;
    string += QString("Grid Dims: ") + QString(m_tileGridDimensions) + newLine;
    string += QString("Pixel Sizes: ") + QString(m_tilePixelSizes) + newLine;
    string += QString("Widget Dims: ") + QString(m_widgetDimensions) + newLine;
    string += QString("Scale: ") + QString::number(m_sliceScalingFactor) + newLine;
    string += QString("Bias: ") + QString::number(m_sliceBiasFactor);

    return string;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}