#pragma once

// Qt
#include <QtWidgets>

// External
#include "fortress/json/GJson.h"

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

namespace rev {

class GTextureUsageType;
class GString;
class TexturePacker;
struct SpriteSheetInfo;
class GRequestAddTexturesToMaterialMessage;
class FileLoadWidget;

/// @class SpriteSheetWidget
/// @brief Tool for creating a sprite sheet
class SpriteSheetWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    SpriteSheetWidget(WidgetManager* wm, QWidget *parent = 0);
    ~SpriteSheetWidget();

    /// @}

protected slots:

    void addTexturesToMaterial(const GRequestAddTexturesToMaterialMessage& message);

protected:
    /// @name Private Methods
    /// @{

    /// @brief Load textures for the specified texture type
    QImage packTextures(const std::vector<GString>& fileNames, GTextureUsageType usageType);

    /// @brief Generate a material from the loaded textures
    void generateMaterial();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;


    /// @}

    /// @name Private Members
    /// @{

    Uuid m_materialId = Uuid::NullID(); ///< ID for the material created by this widget
    std::vector<std::shared_ptr<TexturePacker>> m_texturePackers; ///< Texture packers, indexed by texture type
    std::shared_ptr<SpriteSheetInfo> m_spriteInfo{ nullptr }; ///< Sprite info, should be identical for each texture type, so only need one

    Int32_t m_currentProgress{ 0 }; ///< For progress dialog
    QProgressDialog* m_progressDialog{ nullptr }; ///< Progress generating a material
    QLineEdit* m_materialName{ nullptr };
    QComboBox* m_textureType{ nullptr };
    QLineEdit* m_paddingWidget{ nullptr };
    FileLoadWidget* m_fileLoad{ nullptr };
    QLabel* m_bitmapWidget{ nullptr };
    FileLoadWidget* m_outputDirectory{ nullptr };
    QPushButton* m_confirmButton{ nullptr };
    QPushButton* m_cancelButton{ nullptr };

    /// @}
};



// End namespaces        
}
