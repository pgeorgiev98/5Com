#ifndef PREFERENCESPAGE_H
#define PREFERENCESPAGE_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QRadioButton;
class QLineEdit;

class PreferencesPage : public QDialog
{
	Q_OBJECT
public:
	explicit PreferencesPage(QWidget *parent = nullptr);

signals:
	void preferencesChanged();

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
	QCheckBox *m_saveMainWindowSize;
	QSpinBox *m_mainWindowWidth;
	QSpinBox *m_mainWindowHeight;
	QRadioButton *m_useBuiltInFixedFont;
	QRadioButton *m_useSystemFixedFont;
	QRadioButton *m_useOtherFixedFont;
	QLineEdit *m_fixedFontName;
	QSpinBox *m_fixedFontSize;
	QWidget *m_fixedFontInputWidget;
};

#endif // PREFERENCESPAGE_H
