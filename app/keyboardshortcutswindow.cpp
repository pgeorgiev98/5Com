#include "keyboardshortcutswindow.h"

#include <QGridLayout>
#include <QLabel>

KeyboardShortcutsWindow::KeyboardShortcutsWindow(QWidget *parent)
	: QWidget(parent, Qt::Dialog)
{
	QGridLayout *layout = new QGridLayout;
	layout->setHorizontalSpacing(24);
	QString m[] = {"Exit", QKeySequence(QKeySequence::Quit).toString(),
				   "Write file to port", QKeySequence(QKeySequence::Open).toString(),
				   "Export", "Ctrl+E",
				   "Clear screen", "Ctrl+Shift+L",
				   "Focus input", "Ctrl+L",
				   "Clear input", "Ctrl+W",
				   "Open Plain Text View", "Alt+1",
				   "Open Hex View", "Alt+2",
				   "Input history search", "↑/↓"};
	for (unsigned int i = 0; i < sizeof(m) / sizeof(QString); i += 2) {
		layout->addWidget(new QLabel(m[i  ]), i / 2, 0);
		layout->addWidget(new QLabel(m[i+1]), i / 2, 1);
	}
	setLayout(layout);
}
