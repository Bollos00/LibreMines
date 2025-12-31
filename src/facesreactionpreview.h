#ifndef FACESREACTIONPREVIEW_H
#define FACESREACTIONPREVIEW_H

#include "facesreaction.h"

#include <QWidget>
#include <QLabel>
#include <QList>

namespace Ui {
class FacesReactionPreview;
}

/**
 * @brief Preview widget for faces reaction themes
 * 
 * FacesReactionPreview is a UI component that displays all available
 * face reactions for a given theme, allowing users to preview how
 * the faces will appear before selecting a theme. It follows the same
 * design pattern as MinefieldThemePreview for consistency across the
 * application.
 * 
 * @since v1.5.0
 */
class FacesReactionPreview : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct a FacesReactionPreview widget
     * 
     * @param parent Parent widget (optional)
     */
    explicit FacesReactionPreview(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~FacesReactionPreview();

    /**
     * @brief Set and preview a faces reaction theme
     * 
     * Loads the specified theme and displays all face reactions
     * (lost, won, pressed, etc.) in the preview widget.
     * 
     * @param theme Name of the theme to preview
     */
    void vSetFacesReactionTheme(const QString& theme, int length);

private:
    Ui::FacesReactionPreview *ui;

    FacesReaction facesReaction;
    int iFaceLength;
    QList<QLabel*> listLbFaces;
};

#endif // FACESREACTIONPREVIEW_H
