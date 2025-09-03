/**
 * @file keyboardcontroller.h
 * @brief Keyboard navigation system for LibreMines accessibility
 * @author Bruno Bollos Correa
 * @date 2020-2025
 * 
 * This file defines the KeyboardController class which provides full
 * keyboard navigation and control for the LibreMines game. It enables
 * accessibility features and alternative input methods for users who
 * prefer or require keyboard-only interaction with the game.
 */

#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include <QEvent>

#include <libreminesgameengine.h>
class LibreMinesGui;

/**
 * @brief Keyboard navigation and control system for accessibility
 * 
 * KeyboardController provides comprehensive keyboard-based interaction with
 * the minesweeper game, enabling users to navigate the grid, reveal cells,
 * place flags, and perform all game actions without requiring a mouse.
 * 
 * The controller maintains the current cursor position on the grid and
 * responds to configurable key bindings for movement and actions. It
 * integrates seamlessly with the existing game logic while providing
 * visual feedback for the current cursor position.
 * 
 * Key features:
 * - Grid navigation with arrow keys or custom key bindings
 * - Cell revealing and flag placement via keyboard
 * - Visual cursor highlighting on the current cell
 * - Configurable key mappings for accessibility
 * - Audio feedback integration for actions
 * 
 * @since 1.0
 */
class KeyboardController
{
public:
    /**
     * @brief Default constructor
     * 
     * Initializes the keyboard controller with default key bindings
     * and sets the controller to inactive state.
     */
    KeyboardController();

    /**
     * @brief Handle keyboard events from the application
     * 
     * Processes keyboard events and translates them into game actions
     * when the controller is active. Returns true if the event was
     * handled by the controller.
     * 
     * @param e Keyboard event to process
     * @param gui Pointer to the main GUI for interaction
     * @return true if the event was handled, false otherwise
     */
    bool handleEvent(QEvent* e, LibreMinesGui* gui);

    /**
     * @brief Check if the controller configuration is valid
     * 
     * Verifies that all required key bindings are properly configured
     * and not conflicting with each other.
     * 
     * @return true if the configuration is valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Get the key code for left movement
     * 
     * @return Qt key code for moving the cursor left
     */
    int getKeyLeft() const;
    
    /**
     * @brief Get the key code for downward movement
     * 
     * @return Qt key code for moving the cursor down
     */
    int getKeyDown() const;
    
    /**
     * @brief Get the key code for right movement
     * 
     * @return Qt key code for moving the cursor right
     */
    int getKeyRight() const;
    
    /**
     * @brief Get the key code for upward movement
     * 
     * @return Qt key code for moving the cursor up
     */
    int getKeyUp() const;
    
    /**
     * @brief Get the key code for revealing cells
     * 
     * @return Qt key code for revealing the current cell
     */
    int getKeyReleaseCell() const;
    
    /**
     * @brief Get the key code for flag operations
     * 
     * @return Qt key code for placing/removing flags on the current cell
     */
    int getKeyFlagCell() const;
    
    /**
     * @brief Get the key code for centering actions
     * 
     * @return Qt key code for chord clicking (revealing neighbors)
     */
    int getKeyCenterCell() const;

    /**
     * @brief Set custom key bindings for the controller
     * 
     * Configures the controller with custom key codes for all actions.
     * The keys list should contain exactly 7 elements in the order:
     * [left, down, right, up, release, flag, center]
     * 
     * @param keys List of Qt key codes for the controller actions
     */
    void setKeys(QList<int> keys);

    /**
     * @brief Check if the controller is currently active
     * 
     * @return true if the controller is active and handling events
     */
    bool isActive() const;

    /**
     * @brief Refresh the controller state and position
     * 
     * Updates the controller with new grid coordinates and ensures
     * the visual cursor is properly positioned and displayed.
     * 
     * @param x New X coordinate for the cursor
     * @param y New Y coordinate for the cursor
     * @param gui Pointer to the main GUI for updates
     */
    void refresh(uchar x, uchar y, LibreMinesGui* gui);
    
    /**
     * @brief Deactivate the keyboard controller
     * 
     * Disables keyboard navigation and removes the visual cursor
     * from the game grid.
     * 
     * @param gui Pointer to the main GUI for cleanup
     */
    void deactivate(LibreMinesGui* gui);

private:
    /**
     * @brief Activate the keyboard controller
     * 
     * Enables keyboard navigation and initializes the visual cursor
     * at the center of the game grid.
     * 
     * @param gui Pointer to the main GUI for initialization
     */
    void activate(LibreMinesGui* gui);

    /**
     * @brief Set the current cursor position
     * 
     * Updates the cursor position and refreshes the visual highlighting
     * on the game grid.
     * 
     * @param x X coordinate for the cursor
     * @param y Y coordinate for the cursor
     * @param gui Pointer to the main GUI for updates
     */
    void setCurrentCell(const uchar x, const uchar y, LibreMinesGui* gui);
    
    /**
     * @brief Remove the visual cursor from the grid
     * 
     * Clears the cursor highlighting from the current cell.
     * 
     * @param gui Pointer to the main GUI for updates
     */
    void unsetCurrentCell(LibreMinesGui* gui);
    
    /**
     * @brief Move the cursor left
     * 
     * @param gui Pointer to the main GUI for updates
     */
    void moveLeft(LibreMinesGui* gui);
    
    /**
     * @brief Move the cursor right
     * 
     * @param gui Pointer to the main GUI for updates
     */
    void moveRight(LibreMinesGui* gui);
    
    /**
     * @brief Move the cursor down
     * 
     * @param gui Pointer to the main GUI for updates
     */
    void moveDown(LibreMinesGui* gui);
    
    /**
     * @brief Move the cursor up
     * 
     * @param gui Pointer to the main GUI for updates
     */
    void moveUp(LibreMinesGui* gui);
    
    /**
     * @brief Reveal the cell at the current cursor position
     * 
     * @param gui Pointer to the main GUI for action processing
     */
    void releaseCenterCell(LibreMinesGui* gui);
    
    /**
     * @brief Toggle flag state of the cell at cursor position
     * 
     * @param gui Pointer to the main GUI for action processing
     */
    void flagCenterCell(LibreMinesGui* gui);
    
    /**
     * @brief Perform chord click at the current cursor position
     * 
     * Attempts to reveal neighboring cells if the current cell is
     * already revealed and has the correct number of flags around it.
     * 
     * @param gui Pointer to the main GUI for action processing
     */
    void centerCurrentCell(LibreMinesGui* gui);

    bool ctrlPressed;   ///< Whether Ctrl key is currently pressed
    bool active;        ///< Whether the controller is active
    uchar currentX;     ///< Current cursor X coordinate
    uchar currentY;     ///< Current cursor Y coordinate

    int keyLeft;        ///< Key code for left movement
    int keyDown;        ///< Key code for down movement  
    int keyRight;       ///< Key code for right movement
    int keyUp;          ///< Key code for up movement
    int keyReleaseCell; ///< Key code for cell revealing
    int keyFlagCell;    ///< Key code for flag operations
    int keyCenterCell;  ///< Key code for chord clicking

    bool keysVaild;     ///< Whether the current key configuration is valid
};

#endif // KEYBOARDCONTROLLER_H
