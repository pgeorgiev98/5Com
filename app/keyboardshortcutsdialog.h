#ifndef KEYBOARDSHORTCUTSDIALOG_H
#define KEYBOARDSHORTCUTSDIALOG_H

#include "config.h"

#include <QDialog>
#include <QString>
#include <QKeySequence>
#include <QVector>

class KeySequenceInputWidget;
class QTreeWidget;

class KeyboardShortcutsDialog : public QDialog
{
	Q_OBJECT
public:
	explicit KeyboardShortcutsDialog(QWidget *parent = nullptr);

public slots:
	void saveShortcuts();
	void restoreDefaults();

private:
	struct Shortcut
	{
		QString name;
		QKeySequence sequence;
		void (Config::Shortcuts::*setter)(const QString &);
		KeySequenceInputWidget *widget;
	};

	void addShortcuts();
	void addShortcut(const QString &name, const QString &sequence, void (Config::Shortcuts::*setter)(const QString &));

	QVector<Shortcut> m_shortcuts;
	QTreeWidget *m_treeWidget;
};

#endif // KEYBOARDSHORTCUTSDIALOG_H
