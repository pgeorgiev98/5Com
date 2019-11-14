#ifndef COMMON_H
#define COMMON_H

#include <QFont>
#include <QFontMetrics>
#include <QString>

#define APPLICATION_NAME "5Com"
#define VERSION "0.4.1"

#define SOURCE_CODE_URL "https://github.com/pgeorgiev98/5Com"
#define RELEASES_URL "https://github.com/pgeorgiev98/5Com/releases"
#define LATEST_RELEASE_URL "https://api.github.com/repos/pgeorgiev98/5Com/releases/latest"

#include <QFont>
QFont getFixedFont();
void loadBuiltInFont();
int textWidth(const QFontMetrics &fm, const QString &text);

#endif // COMMON_H
