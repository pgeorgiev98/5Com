#include "changelogdialog.h"
#include "common.h"
#include <QFile>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QRegularExpression>

ChangelogDialog::ChangelogDialog(QWidget *parent)
	: QDialog(parent)
{
	QString changelog = changelogHtml();
	QFont font = getFixedFont();
	QFontMetrics fontMetrics(font);
	int textWidth = fontMetrics.size(0, QString(" ").repeated(100)).width();

	QTextBrowser *view = new QTextBrowser;
	view->setReadOnly(true);
	view->setHtml(changelog);
	view->setWordWrapMode(QTextOption::WrapMode::NoWrap);
	view->setMinimumWidth(textWidth);
	view->setMinimumHeight(fontMetrics.height() * 30);
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

	QString listItemText;

	auto commitListItem = [&listItemText, &html]() {
		if (!listItemText.isEmpty()) {
			html.append(QString("<li>%1</li>").arg(listItemText));
			listItemText.clear();
		}
	};

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
				if (!listItemText.isEmpty())
					commitListItem();
				inRelease = false;
				html.append("</ul>");
			}
		} else {
			QString l = line.trimmed();
			if (l.startsWith('-')) {
				l.remove(0, 1);
				commitListItem();
				listItemText.append(l.trimmed().toHtmlEscaped());
			} else if (!listItemText.isEmpty()) {
				listItemText.append("<br/>");
				listItemText.append(l.trimmed().toHtmlEscaped());
			}
		}
	}
	commitListItem();
	return html;
}
