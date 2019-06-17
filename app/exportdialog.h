#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class PlainTextView;
class HexView;
class ByteReceiveTimesDialog;
class QRadioButton;

class ExportDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ExportDialog(const QByteArray &rawData,
						  PlainTextView *plaintTextView,
						  HexView *hexView,
						  ByteReceiveTimesDialog *byteReceiveTimesDialog,
						  QWidget *parent = nullptr);

private slots:
	void exportData();

private:
	QByteArray m_rawData;
	PlainTextView *m_plainTextView;
	HexView *m_hexView;
	ByteReceiveTimesDialog *m_byteReceiveTimesDialog;
	QRadioButton *m_rawDataButton;
	QRadioButton *m_plainTextButton;
	QRadioButton *m_hexButton;
	QRadioButton *m_byteReceiveTimesButton;

	QByteArray byteReceiveTimesTable() const;
};

#endif // EXPORTDIALOG_H
