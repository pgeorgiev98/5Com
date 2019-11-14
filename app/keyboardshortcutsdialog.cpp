#include "keyboardshortcutsdialog.h"
#include "keysequenceinputwidget.h"
#include "config.h"
#include "common.h"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

KeyboardShortcutsDialog::KeyboardShortcutsDialog(QWidget *parent)
	: QDialog(parent)
	, m_treeWidget(new QTreeWidget)
{
	m_treeWidget->setRootIsDecorated(false);
	m_treeWidget->setHeaderLabels({"Name", "Shortcut"});
	m_treeWidget->setColumnWidth(0, textWidth(QFontMetrics(m_treeWidget->font()), "A very long name for a shortcut"));
	m_treeWidget->setColumnWidth(1, textWidth(QFontMetrics(m_treeWidget->font()), "CTRL+SHIFT+Whatever"));
	m_treeWidget->setMinimumWidth(int(1.1 * (m_treeWidget->columnWidth(0) + m_treeWidget->columnWidth(1))));

	QPushButton *ok = new QPushButton("Ok");
	QPushButton *cancel = new QPushButton("Cancel");

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	layout->addWidget(m_treeWidget);

	QHBoxLayout *l = new QHBoxLayout;
	layout->addLayout(l);
	l->addStretch();
	l->addWidget(ok);
	l->addStretch();
	l->addWidget(cancel);
	l->addStretch();

	Config c;
	Config::Shortcuts &s = c.shortcuts;

	addShortcut("Export", s.exportShortcut(), &Config::Shortcuts::setExportShortcut);
	addShortcut("Clear screen", s.clearScreenShortcut(), &Config::Shortcuts::setClearScreenShortcut);
	addShortcut("Write file", s.writeFileShortcut(), &Config::Shortcuts::setWriteFileShortcut);
	addShortcut("Quit", s.quitShortcut(), &Config::Shortcuts::setQuitShortcut);

	addShortcut("Focus input field", s.focusInputShortcut(), &Config::Shortcuts::setFocusInputShortcut);
	addShortcut("Clear input field", s.clearInputFieldShortcut(), &Config::Shortcuts::setClearInputFieldShortcut);
	addShortcut("Open plain text view", s.openPlainTextViewShortcut(), &Config::Shortcuts::setOpenPlainTextViewShortcut);
	addShortcut("Open hex view", s.openHexViewShortcut(), &Config::Shortcuts::setOpenHexViewShortcut);

	connect(ok, &QPushButton::clicked, [this]() {saveShortcuts(); accept();});
	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

void KeyboardShortcutsDialog::addShortcut(const QString &name, const QString &sequence, void (Config::Shortcuts::*setter)(const QString &))
{
	auto item = new QTreeWidgetItem({name});
	auto widget = new KeySequenceInputWidget(sequence);
	m_treeWidget->addTopLevelItem(item);
	m_treeWidget->setItemWidget(item, 1, widget);

	m_shortcuts.append(Shortcut{name, QKeySequence::fromString(sequence), setter, widget});
}

void KeyboardShortcutsDialog::saveShortcuts()
{
	Config c;
	for (auto s : m_shortcuts) {
		if (s.sequence != s.widget->sequence())
			(c.shortcuts.*s.setter)(s.widget->sequence().toString());
	}
}
