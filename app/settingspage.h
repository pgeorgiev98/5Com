#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QDialog>

class QCheckBox;

class SettingsPage : public QDialog
{
	Q_OBJECT
public:
	explicit SettingsPage(QWidget *parent = nullptr);

private slots:
	void load();
	void save();

private:
	QCheckBox *m_includePtsDirectory;
};

#endif // SETTINGSPAGE_H
