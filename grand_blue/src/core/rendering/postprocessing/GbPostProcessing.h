//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Class for encapsualting a post-processing effect
//class visPostProcessingEffect : protected QOpenGLFunctions {
//public:
//    // Enums/ Constants
//    enum PostProcessingEffect {
//        kNone,
//        kContrast,
//        kHorizontalBlur,
//        kVerticalBlur,
//        kBrightnessFilter,
//        kBloom,
//        kTranparencyBlend,
//        kDepthToScreen
//    };
//
//    // Constructor/ Destructor
//    visPostProcessingEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer,
//        std::shared_ptr<visGLLayerPostProcessing> layer,
//        PostProcessingEffect type = PostProcessingEffect::kNone);
//    ~visPostProcessingEffect() {}
//
//    // Properties
//    PostProcessingEffect getType() { return m_type; }
//
//    // Methods
//    virtual void setShaderAttributes(std::shared_ptr<visGLLayerPostProcessing> layer);
//    virtual void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer);
//    virtual void applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer);
//
//protected:
//    std::shared_ptr<QOpenGLBuffer> m_vertexBuffer;
//    std::shared_ptr<QOpenGLBuffer> m_indexBuffer;
//    PostProcessingEffect m_type;
//};
//
//class visContrastEffect : public visPostProcessingEffect {
//public:
//    // Constructor/ Destructor
//    visContrastEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer,
//        std::shared_ptr<visGLLayerPostProcessing> layer,
//        float contrast = 0.15f);
//    ~visContrastEffect() {}
//
//    // Methods
//    virtual void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer);
//    //virtual void applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer);
//
//protected:
//    float m_contrast;
//};
//
//class visBrightnessFilterEffect : public visPostProcessingEffect {
//public:
//    // Constructor/ Destructor
//    visBrightnessFilterEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer,
//        std::shared_ptr<visGLLayerPostProcessing> layer);
//    ~visBrightnessFilterEffect() {}
//
//    // Methods
//    virtual void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer);
//
//};
//
//class visBlurEffect : public visPostProcessingEffect {
//public:
//    // Constructor/ Destructor
//    visBlurEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer,
//        std::shared_ptr<visGLLayerPostProcessing> layer,
//        PostProcessingEffect type = PostProcessingEffect::kHorizontalBlur);
//    ~visBlurEffect() {}
//
//    // Methods
//    virtual void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer);
//
//};
//
//class visBloomEffect : public visPostProcessingEffect {
//public:
//    // Constructor/ Destructor
//    visBloomEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer,
//        std::shared_ptr<visGLLayerPostProcessing> layer,
//        float intensity = 0.2f);
//    ~visBloomEffect() {}
//
//    // Methods
//    void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer) override;
//    void applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer) override;
//
//protected:
//    // Bloom intensity
//    float m_bloomIntensity;
//
//    // Stack of effects for bloom
//    std::vector<std::shared_ptr<visPostProcessingEffect>> m_effects;
//};
//
//// Render depth buffer to screen
//class visDepthEffect : public visPostProcessingEffect {
//public:
//    // Constructor/ Destructor
//    visDepthEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer,
//        std::shared_ptr<visGLLayerPostProcessing> layer);
//    ~visDepthEffect() {}
//
//    // Methods
//    void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer) override;
//    void applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer) override;
//
//protected:
//};
//
//// Renders order-independent transparency
//// FIXME: Doesn't do anything valuable right now
////class visTransparencyEffect : public visPostProcessingEffect {
////public:
////    // Constructor/ Destructor
////    visTransparencyEffect(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
////        std::shared_ptr<QOpenGLBuffer> indexBuffer,
////        std::shared_ptr<visGLLayerPostProcessing> layer);
////    ~visTransparencyEffect() {}
////
////    // Methods
////    void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer) override;
////    //void setShaderUniforms(std::shared_ptr<visGLLayerPostProcessing> layer) override;
////    void applyEffect(std::shared_ptr<visGLLayerPostProcessing> layer) override;
////
////protected:
////};
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Class for encapsulating a chain of post-processing effects
//class visPostProcessingChain {
//public:
//    visPostProcessingChain(std::shared_ptr<visGLLayerPostProcessing> layer,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer) :
//        m_layer(layer),
//        m_vertexBuffer(vertexBuffer),
//        m_indexBuffer(indexBuffer)
//    {}
//    ~visPostProcessingChain() {}
//
//    std::shared_ptr<QOpenGLBuffer> vertexBuffer() { return m_vertexBuffer; }
//    std::shared_ptr<QOpenGLBuffer> indexBuffer() { return m_indexBuffer; }
//
//    void addEffect(std::shared_ptr<visPostProcessingEffect> effect);
//
//    void applyPostProcessing();
//
//    void clear() { m_effects.clear(); }
//    bool isEmpty() { return m_effects.empty(); }
//
//private:
//    std::shared_ptr<visGLLayerPostProcessing> m_layer;
//    std::shared_ptr<QOpenGLBuffer> m_vertexBuffer;
//    std::shared_ptr<QOpenGLBuffer> m_indexBuffer;
//    std::vector<std::shared_ptr<visPostProcessingEffect>> m_effects;
//};
//
//
//class PostProcessingLayer{
//public:
//
//    PostProcessingLayer();
//    virtual ~PostProcessingLayer();
//
//    // Properties /////////////////////////////////////////////////////////////////////////////
//    std::vector<std::shared_ptr<visGeometryElement>>& getGeometryOffsets() { return m_geometryOffsets; }
//    std::shared_ptr<visRenderContext> getRenderContext() { return m_navigation->renderContext(); }
//
//    // Get shader params
//    std::shared_ptr<visShaderParams> getEffectsShaderParams() { return m_effectsShaderParams; }
//
//    visShaderProgramType getEffectsShaderType() { return m_effectsShaderType; }
//
//    // Get FBO with scene texture
//    std::shared_ptr<visFrameBufferObject> getSceneFBO() { return m_sceneFBO; }
//    std::shared_ptr<visFrameBufferObject> getSecondSceneFBO() { return m_sceneFBO2; }
//
//    // Get FBO with most current texture for reading
//    std::shared_ptr<visFrameBufferObject> getReadFBO();
//
//    // Get FBO with texture to write to using the ping-pong approach
//    std::shared_ptr<visFrameBufferObject> getWriteFBO();
//
//    std::shared_ptr<QOpenGLShaderProgram> getShaderProgram();
//    void setShaderProgram(std::shared_ptr<QOpenGLShaderProgram> shaderProgram) {
//        m_shaderProgram = shaderProgram;
//    }
//    std::shared_ptr<QOpenGLShaderProgram> getEffectsShaderProgram();
//    void setEffectsShaderProgram(std::shared_ptr<QOpenGLShaderProgram> effectsShaderProgram) {
//        m_effectsShaderProgram = effectsShaderProgram;
//    }
//    // Methods ////////////////////////////////////////////////////////////////////////////////
//    // Bind the framebuffer object to be used for post-processing
//    void bindSceneFrameBuffer();
//
//    // Bind the default screen framebuffer
//    void bindDefaultFrameBuffer();
//
//    // initialize the Geometry Engine
//    virtual void initGeometryEngine(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//    virtual void resetGeometryEngine(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//    //updates the geometry vertex data and indices for processing
//    virtual void updateGeometryCache(mapBBox &region,
//        int zoomLevel,
//        std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//
//    //updates the geometry vertex data and indices for processing only when the view changes
//    virtual void updateViewDependentGeometryCache(mapBBox &region,
//        int zoomLevel,
//        std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//    /*updates the geometry vertex data and indices for processing only when the view changes
//    */
//    virtual void updateTimeDependentGeometryCache(mapBBox &region,
//        int zoomLevel,
//        std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//    virtual void updateTexture(const QPixmap &newImage);
//
//    // Renders the scene as a texture onto a screen quad
//    virtual void drawGeometry(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//    //export diagnostic text for on screen display
//    virtual void exportDiagText(QString &outText);
//
//    // Generate frame buffers
//    void generateFBOs();
//
//    // Switch the read and write FBOs 
//    void switchCurrentFBO();
//
//    // Method to bind all required attributes to the shader program
//    void setShaderAttributes(std::shared_ptr<QOpenGLShaderProgram> shaderProgram);
//
//    // Set uniforms for the given shader
//    void setShaderUniforms(std::shared_ptr<QOpenGLShaderProgram> shaderProgram,
//        int samplerTexUnit,
//        bool useHDR);
//
//protected:
//
//    // Method to regenerate vertex and index buffers
//    bool generateBuffers(std::shared_ptr<QOpenGLBuffer> vertexBuffer,
//        std::shared_ptr<QOpenGLBuffer> indexBuffer);
//
//    // Generate quad geometry for rendering of texture to screen
//    bool generateGeometry();
//
//    /* initialize the bespoke shader params associated with the vertex and fragment shaders
//    */
//    virtual void initShaderParams();
//
//    QMutex m_layerPostProcessingMutex;
//
//    // Framebuffer for original scene rendering
//    std::shared_ptr<visFrameBufferObject> m_sceneFBO;
//
//    // Frame buffers (two for ping-pong approach)
//    std::vector<std::shared_ptr<visFrameBufferObject>> m_fbos;
//
//    // Index of the most current FBO in the FBO map
//    unsigned int m_fboIndexInMap;
//
//    // Value for contrast
//    float m_contrast;
//
//    // Whether or not to apply order-independent transparency
//    bool m_useTransparency;
//
//    // Shader parameters
//    std::weak_ptr<QOpenGLShaderProgram> m_shaderProgram;
//
//    std::shared_ptr<visShaderParams> m_effectsShaderParams;
//    std::weak_ptr<QOpenGLShaderProgram> m_effectsShaderProgram;
//    visShaderProgramType m_effectsShaderType;
//
//    std::shared_ptr<visPostProcessingChain> m_postProcessingEffects;
//
//    // Screen quad for rendering FBO
//    std::vector<visPointUVData> m_pointData;
//    std::vector<GLuint> m_indexData;
//    std::vector<std::shared_ptr<visGeometryElement>> m_geometryOffsets;
//
//};