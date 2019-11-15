#ifndef FONTPREFERENCESPAGE_H
#define FONTPREFERENCESPAGE_H

#include <QDialog>

class QRadioButton;
class QLineEdit;
class QSpinBox;

class FontPreferencesPage : public QDialog
{
	Q_OBJECT
public:
	explicit FontPreferencesPage(QWidget *parent = nullptr);

signals:
	void preferencesChanged();

private slots:
	void load();
	bool save();
	void restoreDefaults();

private:
	QRadioButton *m_useBuiltInFixedFont;
	QRadioButton *m_useSystemFixedFont;
	QRadioButton *m_useOtherFixedFont;
	QLineEdit *m_fixedFontName;
	QSpinBox *m_fixedFontSize;
	QWidget *m_fixedFontInputWidget;
};

#endif // FONTPREFERENCESPAGE_H
