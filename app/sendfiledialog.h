#ifndef SENDFILEDIALOG_H
#define SENDFILEDIALOG_H

#include <QDialog>

class QLabel;
class QProgressBar;

class SendFileDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SendFileDialog(qint64 bytesToSend, QWidget *parent = nullptr);

public slots:
	void onBytesSent(qint64 bytes);

private:
	qint64 m_totalBytesSent;
	qint64 m_bytesToSend;
	QLabel *m_text;
	QProgressBar *m_progressBar;
};

#endif // SENDFILEDIALOG_H
