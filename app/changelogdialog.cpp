#include "changelogdialog.h"
#include <QFile>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QRegularExpression>

ChangelogDialog::ChangelogDialog(QWidget *parent)
	: QDialog(parent)
{
	QString changelog = changelogHtml();
	QFont font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
	QFontMetrics fontMetrics(font);
	int textWidth = int(fontMetrics.size(0, "The quick brown fox jumps over the lazy dog.").width() * 1.5);

	QTextBrowser *view = new QTextBrowser;
	view->setReadOnly(true);
	view->setHtml(changelog);
	view->setWordWrapMode(QTextOption::WrapMode::NoWrap);
	view->setMinimumWidth(textWidth);
	view->setFixedHeight(parent->height());
	view->setFont(font);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(view);
	setLayout(layout);
}

QString ChangelogDialog::changelogHtml() const
{
	QFile file(":/CHANGELOG.txt");
	bool opened = file.open(QIODevice::ReadOnly);
	Q_ASSERT(opened);
	QString text = file.readAll();

	QString html;
	bool inRelease = false;

	QStringList lines = text.split('\n');
	for (const QString &line : lines) {
		if (!inRelease) {
			QString version = QRegularExpression("v\\d+\\.\\d+.\\d+$").match(line).captured(0);
			QString date = QRegularExpression("\\* *... ... \\d\\d \\d\\d\\d\\d").match(line).captured(0).remove(0, 1).trimmed();
			html.append(QString("<h3>%1 - %2</h3>").arg(version).arg(date));
			html.append("<ul>");
			inRelease = true;
		} else if (line.trimmed().isEmpty()) {
			if (inRelease) {
				inRelease = false;
				html.append("</ul>");
			}
		} else {
			QString l = line.trimmed();
			if (l.startsWith('-')) {
				l.remove(0, 1);
				html.append(QString("<li>%1</li>").arg(l.trimmed().toHtmlEscaped()));
			}
		}
	}
	return html;
}
