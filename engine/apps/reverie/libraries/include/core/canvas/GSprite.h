/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SPRITE_H
#define GB_SPRITE_H

// std
#include <atomic>

// Internal
#include "GGlyph.h"
#include "fortress/time/GStopwatchTimer.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SpriteAnimationProcess;
class Sprite;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class SpriteFlag {
    kAnimated = 1 << 0 // Flagged if the sprite is animated
};
MAKE_FLAGS(SpriteFlag, SpriteFlags)
MAKE_BITWISE(SpriteFlag)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Describes playback mode of a sprite animation
enum class SpritePlaybackMode {
    kSingleShot = 0,
    kLoop,
    kPingPong
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
class SpriteAnimation {
public:

    void updateFrame(Sprite* sprite, float timeInSec);

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const SpriteAnimation& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, SpriteAnimation& orObject);


    /// @brief Return the current frame of the animation
    uint32_t getAnimationFrame(Sprite* sprite, float timeInSec, SpritePlaybackMode mode, bool& donePlaying);

    /// @brief Get the animation time from the given inputs
    float getAnimationTime(Sprite* sprite, float timeInSec, SpritePlaybackMode mode, bool& donePlaying) const;

    /// @brief Length of each frame
    float m_frameSec = 0.0333333;

    /// @brief The playback mode for the animation
    SpritePlaybackMode m_playbackMode = SpritePlaybackMode::kLoop;

    /// @brief Number of frames
    /// @details This is needed in the case that there are empty frames in the sprite-sheet
    int m_frameCount = -1;

    /// @brief Offset of animation in sprite sheet
    int m_offset = 0;

    /// @brief Maximum number of plays
    int m_maxPlays = -1;

    /// @brief The current animation frame
    /// @note Atomic since this is updated from a process on the animation thread
    std::atomic<uint32_t> m_currentFrame = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Sprite class
class Sprite : public Glyph, public NameableInterface {
public:

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
	~Sprite();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual GlyphType glyphType() const override { return GlyphType::kSprite; };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    // TODO: Implement some control over sorting
    virtual size_t getSortID() override { return 0; }

	/// @}

     //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Sprite& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Sprite& orObject);


    /// @}

protected:

    Sprite(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex);

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    friend class SpriteAnimationProcess;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Create the animation process for the sprite
    void addAnimation();
    void removeAnimation();

    /// @brief Set mesh handle to a quad
    void initializeMesh(ResourceCache* cache);

    /// @brief Set uniforms for the sprite
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures(ShaderProgram* shaderProgram, RenderContext* context) override;

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures(ShaderProgram* shaderProgram, RenderContext* context) override;

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    SpriteFlags m_flags;

    SpriteAnimation m_animation;

    std::shared_ptr<SpriteAnimationProcess> m_animationProcess;

    /// @brief String for the material name, if waiting for resource to load
    GString m_materialName;
    
    /// @brief The current coordinates of the sprite in its texture atlas
    Vector2u m_texCoords;

    /// @brief The dimensions of this sprite in its texture atlas
    Vector2u m_texDimensions;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif