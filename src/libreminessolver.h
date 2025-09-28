#ifndef LIBREMINESSOLVER_H
#define LIBREMINESSOLVER_H

#include "libreminesgameengine.h"

#include <QThread>
#include <QMutex>

class LibreMinesSolver : public QThread
{
    Q_OBJECT
public:

    /**
     * @brief Enumeration of possible solver states during execution
     * 
     * Represents the current lifecycle state of the solver thread,
     * from initialization through completion or termination.
     * 
     * @since v3.0.0
     */
    enum class SolverState : quint8 {
        NotStarted  = 0,  ///< Solver has been created but not yet started
        Running     = 1,  ///< Solver is currently executing and generating boards
        Finished    = 2,  ///< Solver has completed successfully and found a solvable board
        Interrupted = 3,  ///< Solver was interrupted before completion
        Failed      = 4   ///< Solver encountered an error or was terminated
    };

    enum class SolverCommand : quint8 {
        CmdNone        = 0,  ///< No command issued
        CmdInterrupt   = 1   ///< Command to interrupt the solver
    };

    explicit LibreMinesSolver(QObject *parent = nullptr);

    uchar getRows() const;
    void setRows(uchar newRows);

    uchar getCols() const;
    void setCols(uchar newCols);

    uchar getSafeX() const;
    void setSafeX(uchar newSafeX);

    uchar getSafeY() const;
    void setSafeY(uchar newSafeY);

    short getNMines() const;
    void setNMines(short newNMines);

    SolverState getSolverState() const;
    
    void interrupt();

    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine> > getBoard() const;

protected:
    void run() override;

private:
    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> board;
    uchar rows;
    uchar cols;
    short nMines;

    uchar safeX;
    uchar safeY;
    
    SolverState state;          ///< Current state of the solver thread
    mutable QMutex stateMutex;        ///< Mutex for thread-safe state access

    SolverCommand cmd;  ///< Current command for controlling solver execution
    mutable QMutex cmdMutex;   ///< Mutex for thread-safe command access

signals:
};

#endif // LIBREMINESSOLVER_H
