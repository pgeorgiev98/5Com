#include "common.h"
#include "config.h"
#include <QFontDatabase>

static int builtInFontId = -1;

QFont getFixedFont()
{
	Config c;
	Config::Font &f = c.font;
	QFont font;
	if (f.useBuildInFixedFont()) {
		font = QFont(QFontDatabase::applicationFontFamilies(builtInFontId).at(0));
	} else if (f.useSystemFixedFont()) {
		font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
	} else {
		QString family = f.fixedFontName();
		if (family.isEmpty())
			font = QFont(QFontDatabase::applicationFontFamilies(builtInFontId).at(0));
		else
			font = QFont(f.fixedFontName());
	}

	font.setPointSize(f.fixedFontSize());

	return font;
}

void loadBuiltInFont()
{
	builtInFontId = QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
}

int getDefaultFixedFontSize()
{
	return QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont).pointSize();
}

int textWidth(const QFontMetrics &fm, const QString &text)
{
#if QT_VERSION >= 0x050B00
	   return fm.horizontalAdvance(text);
#else
	   return fm.width(text);
#endif
}
