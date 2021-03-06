#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include <QDialog>

class ChangelogDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ChangelogDialog(QWidget *parent = nullptr);
	QString changelogHtml() const;
};

#endif // CHANGELOGDIALOG_H
