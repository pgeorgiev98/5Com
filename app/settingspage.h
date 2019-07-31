#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QDialog>

class QCheckBox;
class QSpinBox;

class SettingsPage : public QDialog
{
	Q_OBJECT
public:
	explicit SettingsPage(QWidget *parent = nullptr);

signals:
	void settingsChanged();

private slots:
	void load();
	void save();
	void restoreDefaults();

private:
	QCheckBox *m_includePtsDirectory;
	QCheckBox *m_checkForUpdatesOnStartup;
	QSpinBox *m_readBufferLimitKiB;
	QSpinBox *m_inputHistoryLength;
	QCheckBox *m_clearInputOnSend;
	QCheckBox *m_rememberLastUsedPort;
	QCheckBox *m_rememberInputHistory;
	QSpinBox *m_hexViewBytesPerLine;
};

#endif // SETTINGSPAGE_H
