#include "changelogdialog.h"
#include <QFile>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QFontDatabase>

ChangelogDialog::ChangelogDialog(QWidget *parent)
	: QDialog(parent)
{
	QFile file(":/CHANGELOG.txt");
	bool opened = file.open(QIODevice::ReadOnly);
	Q_ASSERT(opened);
	QString changelog = file.readAll();

	QFont font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
	QFontMetrics fontMetrics = QFontMetrics(font);
	int textWidth = fontMetrics.size(0, changelog).width();
	textWidth += fontMetrics.size(0, "        ").width();

	QPlainTextEdit *view = new QPlainTextEdit;
	view->setReadOnly(true);
	view->setPlainText(changelog);
	view->setWordWrapMode(QTextOption::WrapMode::NoWrap);
	view->setMinimumWidth(textWidth);
	view->setFixedHeight(parent->height());
	view->setFont(font);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(view);
	setLayout(layout);
}
