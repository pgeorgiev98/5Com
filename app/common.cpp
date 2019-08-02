#include "common.h"
#include "config.h"
#include <QFontDatabase>

static int builtInFontId = -1;

QFont getFixedFont()
{
	Config c;
	QFont font;
	if (c.useBuildInFixedFont()) {
		font = QFont(QFontDatabase::applicationFontFamilies(builtInFontId).at(0));
	} else if (c.useSystemFixedFont()) {
		font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
	} else {
		QString family = c.fixedFontName();
		if (family.isEmpty())
			font = QFont(QFontDatabase::applicationFontFamilies(builtInFontId).at(0));
		else
			font = QFont(c.fixedFontName());
	}

	int pointSize = c.fixedFontSize();
	font.setPointSize(pointSize);

	return font;
}

void loadBuiltInFont()
{
	builtInFontId = QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
	Config c;
	int pointSize = c.fixedFontSize();
	if (pointSize == -1) {
		QFont font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
		pointSize = font.pointSize();
		c.setFixedFontSize(pointSize);
	}
}
