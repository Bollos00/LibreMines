#ifndef LIBREMINESPREFERENCESSAVER_H
#define LIBREMINESPREFERENCESSAVER_H

#include "libreminespreferencesdialog.h"

class QSpinBox;
class QCheckBox;

namespace LibreMinesPreferencesSaver
{

void vLastSessionLoadConfigurationFile(
    LibreMinesPreferencesDialog& preferences, const QDir& dirAppData,
    QSpinBox& sbCustomizedPercentageMines, QSpinBox& sbCustomizedX, QSpinBox& sbCustomizedY,
    QCheckBox& cbCustomizedMinesInPercentage
);

void vLastSessionSaveConfigurationFile(
    const LibreMinesPreferencesDialog& preferences, const QDir& dirAppData,
    const QSpinBox& sbCustomizedPercentageMines, const QSpinBox& sbCustomizedX,
    const QSpinBox& sbCustomizedY,
    const QCheckBox& cbCustomizedMinesInPercentage
);

};

#endif // LIBREMINESPREFERENCESSAVER_H
