#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QRadioButton;
class QLineEdit;

class SettingsPage : public QDialog
{
	Q_OBJECT
public:
	explicit SettingsPage(QWidget *parent = nullptr);

signals:
	void settingsChanged();

private slots:
	void load();
	bool save();
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
	QRadioButton *m_useBuiltInFixedFont;
	QRadioButton *m_useSystemFixedFont;
	QRadioButton *m_useOtherFixedFont;
	QLineEdit *m_fixedFontName;
	QSpinBox *m_fixedFontSize;
	QWidget *m_fixedFontInputWidget;
};

#endif // SETTINGSPAGE_H
