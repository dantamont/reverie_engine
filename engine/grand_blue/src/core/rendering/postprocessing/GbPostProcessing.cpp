//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Class for a post-processing effect
//visPostProcessingEffect::visPostProcessingEffect(
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer,
//    std::shared_ptr<visGLLayerPostProcessing> layer,
//    PostProcessingEffect type) :
//    m_vertexBuffer(vertexBuffer),
//    m_indexBuffer(indexBuffer),
//    m_type(type)
//{
//    initializeOpenGLFunctions();
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visPostProcessingEffect::setShaderAttributes(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    // Obtain attribute indixes ------------------------------------------------------------
//    int posAttrIndex = layer->getEffectsShaderParams()->m_attributeParamIndices[kPosAttr];
//    int uvAttrIndex = layer->getEffectsShaderParams()->m_attributeParamIndices[kUVAttr];
//
//    // Bind attributes  --------------------------------------------------------------------
//    quintptr offset = 0;
//    if (layer->getEffectsShaderProgram()) {
//        // Load position attribute
//        offset = visGLLayer::loadBufferIntoShader(layer->getEffectsShaderProgram(),
//            posAttrIndex,
//            GL_FLOAT,
//            offset,
//            3,
//            sizeof(QVector3D),
//            sizeof(visPointUVData));
//
//        // Load uv coord attribute
//        offset = visGLLayer::loadBufferIntoShader(layer->getEffectsShaderProgram(),
//            uvAttrIndex,
//            GL_FLOAT,
//            offset,
//            2,
//            sizeof(QVector2D),
//            sizeof(visPointUVData));
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visPostProcessingEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    if (layer->getEffectsShaderProgram()) {
//        // Set bloom intensity uniform
//        int bloomIntensityUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kBloomIntensity];
//        layer->getEffectsShaderProgram()->setUniformValue(bloomIntensityUniform, 0.0f);
//
//        // Set color texture uniform
//        int samplerUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kSampler0];
//        layer->getEffectsShaderProgram()->setUniformValue(samplerUniform,
//            layer->getReadFBO()->m_colorTextureUnit);
//
//        // Set contrast uniform
//        int contrastUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[visShaderParamType::kContrast];
//        layer->getEffectsShaderProgram()->setUniformValue(contrastUniform, 0.0f);
//
//        // Set screen resolution
//        int screenResUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kScreenResolution];
//        layer->getEffectsShaderProgram()->setUniformValue(screenResUniform,
//            QVector2D(layer->getRenderContext()->getWidth(),
//                layer->getRenderContext()->getHeight()));
//
//        // Set to effect type
//        int effectTypeUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kEffectType];
//        layer->getEffectsShaderProgram()->setUniformValue(effectTypeUniform, int(m_type));
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visPostProcessingEffect::applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    // Bind horizontal blur framebuffer
//    layer->getWriteFBO()->bind();
//
//    // Set GL Settings ---------------------------------------------------------------------------
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//    // Bind shader -------------------------------------------------------------------------------
//    if (layer->getEffectsShaderProgram()) {
//        layer->getEffectsShaderProgram()->bind();
//
//        // Bind vertex and index buffers -------------------------------------------------------------
//        m_vertexBuffer->bind();
//        m_indexBuffer->bind();
//
//        // Render post-processed scene quad -----------------------------------------------------
//        layer->getReadFBO()->bindColorTexture();
//
//        // Set shader params
//        setShaderAttributes(layer);
//        setShaderUniforms(layer);
//
//        // Paints a quad to the blue texture
//        for (std::shared_ptr<visGeometryElement> elem : layer->getGeometryOffsets()) {
//            elem->draw();
//        }
//
//        layer->printGLError("Warning, GL error drawing blur geometry");
//
//        // Release vertex and index buffers ----------------------------------------------------------
//        m_vertexBuffer->release();
//        m_indexBuffer->release();
//
//        // Release shader and reset settings ---------------------------------------------------------
//        layer->getEffectsShaderProgram()->release();
//    }
//    layer->getWriteFBO()->unBind();
//
//    // Switch read and write buffers for ping-pong approach
//    layer->switchCurrentFBO();
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//visContrastEffect::visContrastEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer,
//    std::shared_ptr<visGLLayerPostProcessing> layer,
//    float contrast) :
//    visPostProcessingEffect(vertexBuffer, indexBuffer, layer, PostProcessingEffect::kContrast),
//    m_contrast(contrast)
//{
//}
//
//void visContrastEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    visPostProcessingEffect::setShaderUniforms(layer);
//
//    if (layer->getEffectsShaderProgram()) {
//
//        // Set contrast uniform
//        int contrastUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[visShaderParamType::kContrast];
//        layer->getEffectsShaderProgram()->setUniformValue(contrastUniform, m_contrast);
//
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//visBrightnessFilterEffect::visBrightnessFilterEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer,
//    std::shared_ptr<visGLLayerPostProcessing> layer) :
//    visPostProcessingEffect(vertexBuffer, indexBuffer, layer, PostProcessingEffect::kBrightnessFilter)
//{
//}
//
//void visBrightnessFilterEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    visPostProcessingEffect::setShaderUniforms(layer);
//
//    if (layer->getEffectsShaderProgram()) {
//        // Set screen resolution for better performance
//        int screenResUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kScreenResolution];
//        layer->getEffectsShaderProgram()->setUniformValue(screenResUniform,
//            QVector2D(layer->getRenderContext()->getWidth() / 2.0,
//                layer->getRenderContext()->getHeight() / 2.0));
//    }
//}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Class for blur post-processing effect
//visBlurEffect::visBlurEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer,
//    std::shared_ptr<visGLLayerPostProcessing> layer,
//    PostProcessingEffect type) :
//    visPostProcessingEffect(vertexBuffer, indexBuffer, layer)
//{
//    m_type = type;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visBlurEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    visPostProcessingEffect::setShaderUniforms(layer);
//    if (layer->getEffectsShaderProgram()) {
//        // Set screen resolution lower for more blur (and performance)
//        int screenResUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kScreenResolution];
//        layer->getEffectsShaderProgram()->setUniformValue(screenResUniform,
//            QVector2D(layer->getRenderContext()->getWidth() / 5.0,
//                layer->getRenderContext()->getHeight() / 5.0));
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//visBloomEffect::visBloomEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer,
//    std::shared_ptr<visGLLayerPostProcessing> layer,
//    float intensity) :
//    visPostProcessingEffect(vertexBuffer, indexBuffer, layer, PostProcessingEffect::kBloom),
//    m_bloomIntensity(intensity)
//{
//    // Add brightness filter
//    std::shared_ptr<visBrightnessFilterEffect> brightnessEffect = std::make_shared<visBrightnessFilterEffect>(
//        vertexBuffer,
//        indexBuffer,
//        layer);
//    m_effects.push_back(brightnessEffect);
//
//    // Add Gaussian blur
//    std::shared_ptr<visBlurEffect> horizontalBlurEffect = std::make_shared<visBlurEffect>(
//        vertexBuffer,
//        indexBuffer,
//        layer);
//
//    std::shared_ptr<visBlurEffect> verticalBlurEffect = std::make_shared<visBlurEffect>(
//        vertexBuffer,
//        indexBuffer,
//        layer,
//        visPostProcessingEffect::kVerticalBlur);
//
//    // Use one blur pass, more would increase blur quaity
//    for (int i = 0; i < 1; i++) {
//        m_effects.push_back(horizontalBlurEffect);
//        m_effects.push_back(verticalBlurEffect);
//    }
//}
//
//void visBloomEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    visPostProcessingEffect::setShaderUniforms(layer);
//
//    if (layer->getEffectsShaderProgram()) {
//        // Set bloom intensity uniform
//        int bloomIntensityUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kBloomIntensity];
//        layer->getEffectsShaderProgram()->setUniformValue(bloomIntensityUniform, m_bloomIntensity);
//
//        // Set scene texture uniform
//        int sceneTextureUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kSampler1];
//        layer->getEffectsShaderProgram()->setUniformValue(sceneTextureUniform,
//            layer->getSceneFBO()->m_colorTextureUnit);
//
//    }
//}
//
//void visBloomEffect::applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    for (std::shared_ptr<visPostProcessingEffect> effect : m_effects) {
//        // Apply effect depending on type
//        switch (effect->getType()) {
//        case visPostProcessingEffect::kNone:
//        case visPostProcessingEffect::kContrast:
//            break;
//        case visPostProcessingEffect::kHorizontalBlur:
//        case visPostProcessingEffect::kVerticalBlur: {
//            std::shared_ptr<visBlurEffect> blurEffect =
//                std::static_pointer_cast<visBlurEffect>(effect);
//            blurEffect->applyEffect(layer);
//        }
//                                                     break;
//        case visPostProcessingEffect::kBrightnessFilter: {
//            std::shared_ptr<visBrightnessFilterEffect> brightnessFilter =
//                std::static_pointer_cast<visBrightnessFilterEffect>(effect);
//            brightnessFilter->applyEffect(layer);
//        }
//                                                         break;
//        case visPostProcessingEffect::kBloom:
//            // Bloom shouldn't contain another bloom effect
//            break;
//        }
//    }
//
//    // Bind the texture of the raw scene
//    layer->getSceneFBO()->bindColorTexture();
//
//    // Apply the actual bloom effect to combine scene and bloom textures
//    visPostProcessingEffect::applyEffect(layer);
//}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Depth effect
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//visDepthEffect::visDepthEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer,
//    std::shared_ptr<visGLLayerPostProcessing> layer) :
//    visPostProcessingEffect(vertexBuffer, indexBuffer, layer)
//{
//    m_type = kDepthToScreen;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visDepthEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    visPostProcessingEffect::setShaderUniforms(layer);
//
//    if (layer->getEffectsShaderProgram()) {
//
//        // Set clipping uniforms
//        int nearUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kZNear];
//        layer->getEffectsShaderProgram()->setUniformValue(nearUniform,
//            float(layer->getRenderContext()->getNear()));
//        int farUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kZFar];
//        layer->getEffectsShaderProgram()->setUniformValue(farUniform,
//            float(layer->getRenderContext()->getMinFar()));
//
//        // Set depth texture uniform
//        int depthTexUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kSampler2];
//        //layer->getEffectsShaderProgram()->setUniformValue(depthTexUniform,
//        //    layer->getSceneFBO()->m_depthTextureUnit);
//        layer->getEffectsShaderProgram()->setUniformValue(depthTexUniform,
//            layer->getSceneFBO()->m_depthTextureUnit);
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visDepthEffect::applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer)
//{
//    layer->getSceneFBO()->bindDepthTexture();
//    visPostProcessingEffect::applyEffect(layer);
//}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Order-independent transparency
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////visTransparencyEffect::visTransparencyEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
////    std::shared_ptr<QOpenGLBuffer> indexBuffer,
////    std::shared_ptr<visGLLayerPostProcessing> layer) :
////    visPostProcessingEffect(vertexBuffer, indexBuffer, layer)
////{
////    m_type = kTranparencyBlend;
////}
////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////void visTransparencyEffect::setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer)
////{
////    visPostProcessingEffect::setShaderUniforms(layer);
////
////    if (layer->getEffectsShaderProgram()) {
////
////        // Set clipping uniforms
////        int nearUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kZNear];
////        layer->getEffectsShaderProgram()->setUniformValue(nearUniform,
////            float(layer->getRenderContext()->getMinNear()));
////        int farUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kZFar];
////        layer->getEffectsShaderProgram()->setUniformValue(farUniform,
////            float(layer->getRenderContext()->getMinFar()));
////
////        // Set depth texture uniform
////        int depthTexUniform = layer->getEffectsShaderParams()->m_uniformParamIndices[kSampler2];
////        layer->getEffectsShaderProgram()->setUniformValue(depthTexUniform,
////            layer->getSceneFBO()->m_depthTextureUnit);
////    }
////}
////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////void visTransparencyEffect::applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer)
////{
////    if (!layer->getEffectsShaderProgram()) return;
////
////    // Bind horizontal blur framebuffer
////    layer->getSecondSceneFBO()->bind();
////
////    // Set GL Settings ---------------------------------------------------------------------------
////    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer
////    glEnable(GL_BLEND);
////    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
////    glDepthMask(true);
////    glDepthFunc(GL_LEQUAL);
////
////    // Bind shader -------------------------------------------------------------------------------
////    layer->getEffectsShaderProgram()->bind();
////
////    // Bind vertex and index buffers -------------------------------------------------------------
////    m_vertexBuffer->bind();
////    m_indexBuffer->bind();
////
////    // Render post-processed scene quad -----------------------------------------------------
////    layer->getReadFBO()->bindColorTexture();
////    layer->getSceneFBO()->bindDepthTexture();
////
////    // Set shader params
////    setShaderAttributes(layer);
////    setShaderUniforms(layer);
////
////    // Paints a quad to the blue texture
////    for (std::shared_ptr<visGeometryElement> elem : layer->getGeometryOffsets()) {
////        elem->draw();
////    }
////
////    layer->printGLError("Warning, GL error drawing first transparency geometry layer");
////
////    // Release vertex and index buffers ----------------------------------------------------------
////    m_vertexBuffer->release();
////    m_indexBuffer->release();
////
////    // Release shader and reset settings ---------------------------------------------------------
////    layer->getEffectsShaderProgram()->release();
////
////    layer->getWriteFBO()->unBind();
////
////    // Switch read and write buffers for ping-pong approach
////    layer->switchCurrentFBO();
////}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// visPostProcessingChain
//void visPostProcessingChain::addEffect(std::shared_ptr<visPostProcessingEffect> effect)
//{
//    m_effects.push_back(effect);
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visPostProcessingChain::applyPostProcessing()
//{
//    // Populate first ping-pong framebuffer with image from scene ////////////////////////////////
//    m_layer->getReadFBO()->bind();
//
//    // Set GL Settings ---------------------------------------------------------------------------
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//    // Bind shader -------------------------------------------------------------------------------
//    if (!m_layer->getShaderProgram()) {
//        qWarning() << "Warning, no shader program for post-processing layer";
//        return;
//    }
//
//    m_layer->getShaderProgram()->bind();
//
//    // Bind vertex and index buffers (same for all effects) ----------------------------------
//    m_vertexBuffer->bind();
//    m_indexBuffer->bind();
//
//    // Render post-processed scene quad -----------------------------------------------------
//    m_layer->getSceneFBO()->bindColorTexture();
//
//    // Set shader params
//    if (m_layer->getShaderProgram()) {
//        m_layer->setShaderAttributes(m_layer->getShaderProgram());
//        m_layer->setShaderUniforms(m_layer->getShaderProgram(),
//            m_layer->getSceneFBO()->m_colorTextureUnit,
//            false);
//
//        // Paints a quad to the screen
//        for (std::shared_ptr<visGeometryElement> elem : m_layer->getGeometryOffsets()) {
//            elem->draw();
//        }
//    }
//
//    // Release vertex and index buffers ----------------------------------------------------------
//    m_vertexBuffer->release();
//    m_indexBuffer->release();
//
//    // Release shader and reset settings ---------------------------------------------------------
//    m_layer->getShaderProgram()->release();
//
//    // Apply post-processing effects /////////////////////////////////////////////////////////////////
//    for (std::shared_ptr<visPostProcessingEffect> effect : m_effects) {
//        // Apply effect depending on type
//        switch (effect->getType()) {
//        case visPostProcessingEffect::kNone:
//            break;
//        case visPostProcessingEffect::kContrast: {
//            std::shared_ptr<visContrastEffect> contrastEffect =
//                std::static_pointer_cast<visContrastEffect>(effect);
//            contrastEffect->applyEffect(m_layer);
//        }
//                                                 break;
//        case visPostProcessingEffect::kHorizontalBlur:
//        case visPostProcessingEffect::kVerticalBlur: {
//            std::shared_ptr<visBlurEffect> blurEffect =
//                std::static_pointer_cast<visBlurEffect>(effect);
//            blurEffect->applyEffect(m_layer);
//        }
//                                                     break;
//        case visPostProcessingEffect::kBrightnessFilter: {
//            std::shared_ptr<visBrightnessFilterEffect> brightnessFilter =
//                std::static_pointer_cast<visBrightnessFilterEffect>(effect);
//            brightnessFilter->applyEffect(m_layer);
//        }
//                                                         break;
//        case visPostProcessingEffect::kBloom: {
//            std::shared_ptr<visBloomEffect> bloomEffect =
//                std::static_pointer_cast<visBloomEffect>(effect);
//            bloomEffect->applyEffect(m_layer);
//        }
//                                              break;
//        case visPostProcessingEffect::kDepthToScreen: {
//            std::shared_ptr<visDepthEffect> depthEffect =
//                std::static_pointer_cast<visDepthEffect>(effect);
//            depthEffect->applyEffect(m_layer);
//        }
//                                                      //case visPostProcessingEffect::kTranparencyBlend: {
//                                                      //    std::shared_ptr<visTransparencyEffect> transparencyEffect =
//                                                      //        std::static_pointer_cast<visTransparencyEffect>(effect);
//                                                      //    transparencyEffect->applyEffect(m_layer);
//                                                      //}
//                                                      break;
//        }
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////!visGLLayerPostProcessing
///* Constructor for visGLLayerPostProcessing
//@Param dataModel: pass in an already instantiated dataModel for processing data
//@Param parent: pass the parent object if needed
//*/
//visGLLayerPostProcessing::visGLLayerPostProcessing(visLayerType layerType,
//    visMapLayerType mapLayerType,
//    visGLModelProjType modelProjType,
//    visCoordinates::visCoordinateSystem coordFrameType,
//    const std::string &name,
//    std::shared_ptr<visNavigator> navigation,
//    std::shared_ptr<visMapModel> mapModel,
//    std::shared_ptr<veDataModel> dataModel) :
//    visGLLayer(kGLLayerPostProcessing,
//        layerType,
//        mapLayerType,
//        modelProjType,
//        coordFrameType,
//        name,
//        visShaderProgramType::kPostProcessing,
//        visShaderProgramType::kPostProcessing,
//        visBufferType::kScreenQuadVerts,
//        navigation,
//        mapModel,
//        dataModel),
//    m_effectsShaderParams(std::make_shared<visShaderParams>()),
//    m_effectsShaderType(kPostProcEffects),
//    m_contrast(0.1f),
//    m_fboIndexInMap(0),
//    m_useTransparency(true)
//{
//
//    // initializeShadersParams  
//    if (m_shaderParams.use_count() == 0) {
//        qWarning() << "visGLLayerPostProcessing:: shader parameters have not been declared";
//    }
//    initShaderParams();
//}
//
//visGLLayerPostProcessing::~visGLLayerPostProcessing() {
//}
//
//std::shared_ptr<QOpenGLShaderProgram> visGLLayerPostProcessing::getEffectsShaderProgram()
//{
//    if (std::shared_ptr<QOpenGLShaderProgram> shaderProgram = m_effectsShaderProgram.lock()) {
//        return shaderProgram;
//    }
//    else {
//        return std::shared_ptr<QOpenGLShaderProgram>(nullptr);
//    }
//}
//
//void visGLLayerPostProcessing::bindSceneFrameBuffer()
//{
//    m_sceneFBO->bind();
//}
//
//void visGLLayerPostProcessing::bindDefaultFrameBuffer()
//{
//    visFrameBufferObject::bindDefaultFrameBuffer();
//}
//
//
//
//void visGLLayerPostProcessing::initGeometryEngine(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer) {
//
//    // Initialize frame buffers --------------------------------------------------------------------------
//    generateFBOs();
//    printGLError("Error initializing framebuffers ");
//
//
//    // Initialize quad geometry to store frame buffer display ---------------------------------------
//    generateGeometry();
//
//    printGLError("Error generating geometry ");
//
//    // Initialize post-processing effects ///////////////////////////////////////////////////////////
//    // Init effects chain
//    m_postProcessingEffects = std::make_shared<visPostProcessingChain>(
//        std::shared_ptr<visGLLayerPostProcessing>(this),
//        vertexBuffer,
//        indexBuffer);
//
//    // Init individual effects
//    if (std::shared_ptr<QOpenGLShaderProgram> blurShaderProgram = m_effectsShaderProgram.lock()) {
//
//        // Add bloom
//        std::shared_ptr<visBloomEffect> bloomEffect = std::make_shared<visBloomEffect>(
//            vertexBuffer,
//            indexBuffer,
//            shared_from_this());
//        m_postProcessingEffects->addEffect(bloomEffect);
//
//        // Add contrast
//        std::shared_ptr<visContrastEffect> contrastEffect = std::make_shared<visContrastEffect>(
//            vertexBuffer,
//            indexBuffer,
//            shared_from_this());
//        m_postProcessingEffects->addEffect(contrastEffect);
//
//        //// Write to depth
//        //std::shared_ptr<visDepthEffect> depthEffect = std::make_shared<visDepthEffect>(
//        //    vertexBuffer,
//        //    indexBuffer,
//        //    shared_from_this());
//        //m_postProcessingEffects->addEffect(depthEffect);
//
//        //std::shared_ptr<visTransparencyEffect> transparencyEffect = std::make_shared<visTransparencyEffect>(
//        //    vertexBuffer,
//        //    indexBuffer,
//        //    shared_from_this());
//        //m_postProcessingEffects->addEffect(transparencyEffect);
//
//    }
//    else {
//        qWarning() << "Warning, no blur shader found in initGeometryEngine";
//    }
//
//}
//
//void visGLLayerPostProcessing::resetGeometryEngine(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer) {
//    //write out 
//    //QMutexLocker locker(&m_layerPostProcessingMutex);
//
//    //deleteFrameBuffers();
//}
//
//void visGLLayerPostProcessing::initShaderParams() {
//
//    if (m_shaderParams.use_count() == 0) {
//        m_shaderParams = std::make_shared<visShaderParams>();
//    }
//
//    // Main post-processing shader /////////////////////////////////////////////////////////////////////
//    // Support only open GL ES
//    m_shaderParams->m_vertexShaderFileName = "share/resources/glsl/postProcessing/PostProcessing.vert";
//    m_shaderParams->m_fragmentShaderFileName = "share/resources/glsl/postProcessing/PostProcessing.frag";
//
//    // Initialize uniforms
//    m_shaderParams->m_uniformParamNames[kSampler0] = "screenTexture";
//    m_shaderParams->m_uniformParamNames[kUseHDR] = "useHDR";
//
//    // Initialize attributes
//    m_shaderParams->m_attributeParamNames[kPosAttr] = "aPos";
//    m_shaderParams->m_attributeParamNames[kUVAttr] = "aTexCoords";
//
//    printGLError("Error initializing main shader params");
//
//    // Effect shader /////////////////////////////////////////////////////////////////////
//    if (m_effectsShaderParams.use_count() == 0) {
//        m_effectsShaderParams = std::make_shared<visShaderParams>();
//    }
//    m_effectsShaderParams->m_vertexShaderFileName = "share/resources/glsl/postProcessing/Effects.vert";
//    m_effectsShaderParams->m_fragmentShaderFileName = "share/resources/glsl/postProcessing/Effects.frag";
//    m_effectsShaderParams->m_uniformParamNames[kScreenResolution] = "screenRes";
//    m_effectsShaderParams->m_uniformParamNames[kSampler0] = "screenTexture";
//    m_effectsShaderParams->m_uniformParamNames[kSampler1] = "sceneTexture";
//    m_effectsShaderParams->m_uniformParamNames[kSampler2] = "depthTexture";
//    m_effectsShaderParams->m_attributeParamNames[kPosAttr] = "aPos";
//    m_effectsShaderParams->m_attributeParamNames[kUVAttr] = "aTexCoords";
//    m_effectsShaderParams->m_uniformParamNames[kContrast] = "contrast";
//    m_effectsShaderParams->m_uniformParamNames[kEffectType] = "effectType";
//    m_effectsShaderParams->m_uniformParamNames[kBloomIntensity] = "bloomIntensity";
//    m_effectsShaderParams->m_uniformParamNames[kZNear] = "zNear";
//    m_effectsShaderParams->m_uniformParamNames[kZFar] = "zFar";
//
//    printGLError("Error initializing effects shader params");
//
//}
//
//void visGLLayerPostProcessing::updateTimeDependentGeometryCache(
//    mapBBox & region,
//    int zoomLevel,
//    std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer) {
//
//    // Lock other threads from accessing post-processing
//    QMutexLocker locker(&m_layerPostProcessingMutex);
//    printGLError("Error starting tdc");
//
//    shaderProgram->bind();
//
//    printGLError("Error binding shader program");
//
//    // Regenerate/bind buffers --------------------------------------------------------------
//    bool buffers = generateBuffers(vertexBuffer, indexBuffer);
//    if (!buffers) return;
//
//    printGLError("Error generating buffers");
//
//    // Set shader attributes for rendering ------------------------------------------------------
//    setShaderAttributes(shaderProgram);
//
//    printGLError("Error setting shader attributes");
//
//
//    shaderProgram->release();
//
//    printGLError("Error updating time-dependent geometry cache");
//
//}
//
//
//void visGLLayerPostProcessing::updateTexture(const QPixmap &newImage) {
//    Q_UNUSED(newImage);
//}
//
//void visGLLayerPostProcessing::updateGeometryCache(mapBBox &region,
//    int zoomLevel,
//    std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer) {
//
//    updateTimeDependentGeometryCache(region, zoomLevel, shaderProgram, vertexBuffer, indexBuffer);
//}
//
//void visGLLayerPostProcessing::updateViewDependentGeometryCache(mapBBox & region,
//    int zoomLevel,
//    std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer) {
//}
//
//void visGLLayerPostProcessing::drawGeometry(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//    std::shared_ptr<QOpenGLBuffer> indexBuffer) {
//    QMutexLocker locker(&m_layerPostProcessingMutex);
//
//    // Apply post-processing effects /////////////////////////////////////////////////////////////
//    m_postProcessingEffects->applyPostProcessing();
//
//    // Unbind the post-processing frame buffers to render to the screen //////////////////////////
//    bindDefaultFrameBuffer();
//
//    printGLError("Warning, GL error binding framebuffer");
//
//    // Set GL Settings ///////////////////////////////////////////////////////////////////////////
//    glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//    // Enable dithering to reduce banding artifacts
//    glEnable(GL_DITHER);
//
//    // Bind shader -------------------------------------------------------------------------------
//    shaderProgram->bind();
//
//    // Bind vertex and index buffers -------------------------------------------------------------
//    vertexBuffer->bind();
//    indexBuffer->bind();
//
//    // Render post-processed scene quad -----------------------------------------------------
//    // Bind the texture that the scene is rendered to to the active slot 
//    getReadFBO()->bindColorTexture();
//
//    // Set attributes in the shader from buffer 
//    setShaderAttributes(shaderProgram);
//
//    // Set shader uniforms 
//    setShaderUniforms(shaderProgram,
//        getReadFBO()->m_colorTextureUnit,
//        false);
//
//    // Paints a quad to the screen for rendering contents of the FBO
//    for (std::shared_ptr<visGeometryElement> elem : m_geometryOffsets) {
//        elem->draw();
//    }
//
//    printGLError("Warning, GL error drawing post-processing geometry");
//
//    // Release vertex and index buffers ----------------------------------------------------------
//    vertexBuffer->release();
//    indexBuffer->release();
//
//    // Release shader and reset settings ---------------------------------------------------------
//    shaderProgram->release();
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visGLLayerPostProcessing::exportDiagText(QString &outText) {
//    Q_UNUSED(outText);
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<visFrameBufferObject> visGLLayerPostProcessing::getReadFBO()
//{
//    return m_fbos[m_fboIndexInMap];
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<visFrameBufferObject> visGLLayerPostProcessing::getWriteFBO()
//{
//    return m_fbos[1 - m_fboIndexInMap];
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<QOpenGLShaderProgram> visGLLayerPostProcessing::getShaderProgram()
//{
//    if (std::shared_ptr<QOpenGLShaderProgram> shaderProgram = m_shaderProgram.lock()) {
//        return shaderProgram;
//    }
//    else {
//        return std::shared_ptr<QOpenGLShaderProgram>(nullptr);
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visGLLayerPostProcessing::switchCurrentFBO()
//{
//    m_fboIndexInMap = 1 - m_fboIndexInMap;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visGLLayerPostProcessing::setShaderAttributes(std::shared_ptr<QOpenGLShaderProgram> shaderProgram)
//{
//    // Obtain attribute indixes ------------------------------------------------------------
//    int posAttrIndex = m_shaderParams->m_attributeParamIndices[kPosAttr];
//    int uvAttrIndex = m_shaderParams->m_attributeParamIndices[kUVAttr];
//
//    // Bind attributes  --------------------------------------------------------------------
//    quintptr offset = 0;
//
//    // Load position attribute
//    offset = loadBufferIntoShader(shaderProgram,
//        posAttrIndex,
//        GL_FLOAT,
//        offset,
//        3,
//        sizeof(QVector3D),
//        sizeof(visPointUVData));
//
//    // Load uv coord attribute
//    offset = loadBufferIntoShader(shaderProgram,
//        uvAttrIndex,
//        GL_FLOAT,
//        offset,
//        2,
//        sizeof(QVector2D),
//        sizeof(visPointUVData));
//
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visGLLayerPostProcessing::setShaderUniforms(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//    int samplerTexUnit,
//    bool useHDR)
//{
//    // Set color texture uniform
//    int samplerUniform = m_shaderParams->m_uniformParamIndices[kSampler0];
//    shaderProgram->setUniformValue(samplerUniform, samplerTexUnit);
//
//    // Don't use HDR for open GL ES 2.0
//    int useHDRUniform = m_shaderParams->m_uniformParamIndices[kUseHDR];
//    shaderProgram->setUniformValue(useHDRUniform, useHDR);
//
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//bool visGLLayerPostProcessing::generateBuffers(std::shared_ptr<QOpenGLBuffer> vertexBuffer, std::shared_ptr<QOpenGLBuffer> indexBuffer)
//{
//    vertexBuffer->destroy();
//    vertexBuffer->create();
//    vertexBuffer->bind();
//    vertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
//
//    indexBuffer->destroy();
//    indexBuffer->create();
//    indexBuffer->bind();
//    indexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
//
//    // Return if no point data
//    if (!m_pointData.size()) return false;
//    if (!m_indexData.size()) return false;
//
//    // allocate vertex buffer based on type of line rendering
//    unsigned int size = m_pointData.size();
//    vertexBuffer->allocate(&m_pointData[0], size * sizeof(visPointUVData));
//
//    // Allocate index buffer
//    indexBuffer->allocate(&m_indexData[0], m_indexData.size() * sizeof(GLuint));
//
//    return true;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void visGLLayerPostProcessing::generateFBOs()
//{
//    // Create framebuffers for scene rendering
//    m_sceneFBO = std::make_shared<visFrameBufferObject>(m_navigation, 1, 5, false);
//    m_sceneFBO2 = std::make_shared<visFrameBufferObject>(m_navigation, 2, 6, false);
//
//    // NOTE: only ever need two with the ping-pong approach
//    m_fbos.clear();
//    m_fbos.push_back(std::make_shared<visFrameBufferObject>(m_navigation, 3, 7, false));
//    m_fbos.push_back(std::make_shared<visFrameBufferObject>(m_navigation, 4, 8, false));
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//bool visGLLayerPostProcessing::generateGeometry()
//{
//    // Create point data for quad
//    m_pointData = {
//        visPointUVData(QVector3D(-1.0f,  1.0f, 0.0f), QVector2D(0.0f, 1.0f)),
//        visPointUVData(QVector3D(-1.0f, -1.0f, 0.0f), QVector2D(0.0f, 0.0f)),
//        visPointUVData(QVector3D(1.0f, -1.0f, 0.0f), QVector2D(1.0f, 0.0f)),
//        visPointUVData(QVector3D(1.0f,  1.0f, 0.0f), QVector2D(1.0f, 1.0f))
//    };
//
//    // Create index data for front-facing quad
//    m_indexData = { 0, 1, 2, 0, 2, 3 };
//
//    // Create geometry element for quad
//    m_geometryOffsets.clear();
//    m_geometryOffsets.push_back(
//        std::make_shared<visGeometryElement>(GL_TRIANGLES, 0, 6)
//    );
//
//    return true;
//}