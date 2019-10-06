#include "autoupdatedialog.h"
#include "common.h"

#include <QtGui/private/qzipreader_p.h>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QBuffer>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QMessageBox>

AutoUpdateDialog::AutoUpdateDialog(QWidget *parent)
	: QDialog(parent)
	, m_manager(new QNetworkAccessManager(this))
	, m_file(new QFile(this))
	, m_label(new QLabel)
	, m_progressBar(new QProgressBar)
{
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(m_label, 0, Qt::AlignHCenter);
	layout->addWidget(m_progressBar);

	connect(m_manager, &QNetworkAccessManager::finished, this, &AutoUpdateDialog::onFinished);
}

void AutoUpdateDialog::startDownloading(const QString &urlString)
{
	m_label->setText("Downloading...");
	m_progressBar->setRange(0, 0);
	QUrl url(urlString);
	QNetworkRequest request(url);
	QNetworkReply *reply = m_manager->get(request);
	connect(reply, &QNetworkReply::downloadProgress, this, &AutoUpdateDialog::onProgress);
}

void AutoUpdateDialog::onFinished(QNetworkReply *reply)
{
	if (reply->error()) {
		m_label->setText("Failed: " + reply->errorString());
		qCritical() << "ERROR:" << reply->errorString();
		return;
	}

	// Check for redirection
	QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (!redirectionTarget.isNull()) {
		QUrl redirectUrl = reply->url().resolved(redirectionTarget.toUrl());
		startDownloading(redirectUrl.toString());
		reply->deleteLater();
		return;
	}

	saveFile(reply);

	reply->deleteLater();
}

void AutoUpdateDialog::onProgress(qint64 current, qint64 total)
{
	if (m_progressBar->maximum() != total)
		m_progressBar->setMaximum(int(total));
	m_progressBar->setValue(int(current));
}

void AutoUpdateDialog::saveFile(QNetworkReply *reply)
{
	m_label->setText("Extracting...");
	repaint();

	QByteArray data = reply->readAll();
	QBuffer buffer(&data);

	QZipReader reader(&buffer);
	QString dir = QDir::tempPath();
	if (!reader.extractAll(dir)) {
		m_label->setText("Extracting failed: " + QString::number(reader.status()));
		return;
	}

	m_label->setText("Swapping bianries...");
	repaint();

	QFileInfo appInfo = QFileInfo(QCoreApplication::applicationFilePath());
	if (!appInfo.fileName().endsWith(".exe")) {
		m_label->setText("Unexpected error: executable is not an .exe file");
		return;
	}
	QString fileNameWithoutExtension = appInfo.fileName();
	fileNameWithoutExtension.remove(fileNameWithoutExtension.size() - 4, 4);
	QString oldAppPath(appInfo.path() + "/" + fileNameWithoutExtension + "_old.exe");
	QFileInfo newAppInfo(dir + "/5Com.exe");

	QFile oldAppFile(oldAppPath);
	if (oldAppFile.exists()) {
		if (!oldAppFile.remove()) {
			m_label->setText("Failed to remove backup file: " + oldAppFile.errorString());
			return;
		}
	}

	QFile appFile(appInfo.filePath());
	if (!appFile.rename(oldAppPath)) {
		m_label->setText("Failed to move current application: " + appFile.errorString());
		return;
	}

	QFile newAppFile(newAppInfo.filePath());
	if (!newAppFile.rename(appInfo.filePath())) {
		m_label->setText("Failed to move new application: " + newAppFile.errorString());
		appFile.rename(appInfo.filePath());
		return;
	}

	m_label->setText("Done");

	int b = QMessageBox::question(this, "Update",
								   "5Com was updated. You must restart the program to use the newer version.",
								   "Restart later", "Restart now");
	if (b == 1) {
		if (!QProcess::startDetached("\"" + appInfo.filePath() + "\""))
			QMessageBox::critical(this, "", "Failed to launch new application", "Cancel");
		else
			QCoreApplication::exit(0);
	}

	accept();
}
