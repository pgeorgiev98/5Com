#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class PlainTextView;
class HexView;
class QRadioButton;

class ExportDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ExportDialog(const QByteArray &rawData,
						  PlainTextView *plaintTextView,
						  HexView *hexView,
						  QWidget *parent = nullptr);

private slots:
	void exportData();

private:
	QByteArray m_rawData;
	PlainTextView *m_plainTextView;
	HexView *m_hexView;
	QRadioButton *m_rawDataButton;
	QRadioButton *m_plainTextButton;
	QRadioButton *m_hexButton;
};

#endif // EXPORTDIALOG_H
