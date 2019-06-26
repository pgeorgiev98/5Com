#ifndef AUTOUPDATEDIALOG_H
#define AUTOUPDATEDIALOG_H

#include <QDialog>

class QNetworkReply;
class QNetworkAccessManager;
class QLabel;
class QProgressBar;
class QFile;

class AutoUpdateDialog : public QDialog
{
	Q_OBJECT
public:
	explicit AutoUpdateDialog(QWidget *parent = nullptr);

public slots:
	void startDownloading(const QString &url);
	void saveFile(QNetworkReply *reply);

private slots:
	void onFinished(QNetworkReply *reply);
	void onProgress(qint64 current, qint64 total);

private:
	QNetworkAccessManager *m_manager;
	QFile *m_file;
	QLabel *m_label;
	QProgressBar *m_progressBar;
};

#endif // AUTOUPDATEDIALOG_H
