#include "exportdialog.h"
#include "line.h"
#include "plaintextview.h"
#include "hexview.h"
#include "bytereceivetimesdialog.h"
#include "config.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>

#include <QStandardPaths>
#include <QTextStream>

ExportDialog::ExportDialog(const QByteArray &rawData,
						   PlainTextView *plaintTextView,
						   HexView *hexView,
						   ByteReceiveTimesDialog *byteReceiveTimesDialog,
						   QWidget *parent)
	: QDialog(parent)
	, m_rawData(rawData)
	, m_plainTextView(plaintTextView)
	, m_hexView(hexView)
	, m_byteReceiveTimesDialog(byteReceiveTimesDialog)

	, m_exportWithCRLFEndings(new QCheckBox("Export with CR-LF line endings"))
	, m_rawDataButton(new QRadioButton("Binary file"))
	, m_plainTextButton(new QRadioButton("Plain text file"))
	, m_hexButton(new QRadioButton("Formatted hex file"))
	, m_byteReceiveTimesButton(new QRadioButton("Byte receive times table"))
{
	Config c;
	QPushButton *exportButton = new QPushButton("Export");
	m_rawDataButton->setChecked(true);
	m_exportWithCRLFEndings->setChecked(c.exportWithCRLFEndings());

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(16);
	setLayout(layout);

	QGroupBox *exportTypeBox = new QGroupBox("Export type");
	QVBoxLayout *exportTypesLayout = new QVBoxLayout;
	exportTypeBox->setLayout(exportTypesLayout);
	exportTypesLayout->addWidget(m_rawDataButton);
	exportTypesLayout->addWidget(m_plainTextButton);
	exportTypesLayout->addWidget(m_hexButton);
	exportTypesLayout->addWidget(m_byteReceiveTimesButton);

	layout->addWidget(exportTypeBox);
	layout->addWidget(m_exportWithCRLFEndings);
	layout->addWidget(exportButton);

	connect(exportButton, &QPushButton::clicked, this, &ExportDialog::exportData);

	m_exportWithCRLFEndings->setDisabled(m_rawDataButton->isChecked());
	connect(m_rawDataButton, &QRadioButton::toggled, m_exportWithCRLFEndings, &QWidget::setDisabled);
}

static bool confirmReplaceFile(ExportDialog *t, const QString &path)
{
	QFile file(path);
	if (!file.exists())
		return true;
	int b = QMessageBox::question(t, "Export", path + " already exists\nDo you want to replace it?");
	return b == QMessageBox::StandardButton::Yes;
}

void ExportDialog::exportData()
{
	static const char *binaryFilter = "Binary (*.bin)";
	static const char *plainTextFilter = "Plain text (*.txt)";
	static const char *csvFilter = "CSV file (*.csv)";
	static const char *anyTypeFilter = "Any type (*)";

	Config c;
	QStringList filters;

#ifdef Q_OS_WIN
	if (m_plainTextButton->isChecked())
		filters << plainTextFilter;
	else if (m_hexButton->isChecked())
		filters << plainTextFilter;
#endif

	if (m_byteReceiveTimesButton->isChecked())
		filters << csvFilter;

	filters << anyTypeFilter;

	QString path;
	for (;;) {
		QString dirName = c.lastExportDirectory();
		if (dirName.isEmpty() || !QDir(dirName).exists())
			dirName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
		QFileDialog dialog(this, "Export", dirName, filters.join(";;")); // TODO: Remember directory
		dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
		if (!dialog.exec() || dialog.selectedFiles().isEmpty()) {
			reject();
			return;
		}

		path = dialog.selectedFiles().first();
		QString filter = dialog.selectedNameFilter();

		if (filter == binaryFilter && !path.toLower().endsWith(".bin")) {
			path.append(".bin");
			if (!confirmReplaceFile(this, path))
				continue;
		} else if (filter == plainTextFilter && !path.toLower().endsWith(".txt")) {
			path.append(".txt");
			if (!confirmReplaceFile(this, path))
				continue;
		} else if (filter == csvFilter && !path.toLower().endsWith(".csv")) {
			path.append(".csv");
			if (!confirmReplaceFile(this, path))
				continue;
		}

		c.setLastExportDirectory(dialog.directory().path());
		break;
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "Export error", "Failed to open file \"" +
							  path + "\": " + file.errorString());
		reject();
		return;
	}

	QByteArray data;

	if (m_rawDataButton->isChecked())
		data = m_rawData;
	else if (m_plainTextButton->isChecked())
		data = m_plainTextView->toPlainText().toLatin1();
	else if (m_hexButton->isChecked())
		data = m_hexView->toPlainText().toLatin1();
	else if (m_byteReceiveTimesButton->isChecked())
		data = byteReceiveTimesTable();

	if (!m_rawDataButton->isChecked() &&
			m_exportWithCRLFEndings->isChecked())
		data.replace('\n', "\r\n");
	c.setExportWithCRLFEndings(m_exportWithCRLFEndings->isChecked());

	if (file.write(data) < 0) {
		QMessageBox::critical(this, "Export error", "Failed to write to file: " + file.errorString());
		reject();
		return;
	}

	accept();
}

QByteArray ExportDialog::byteReceiveTimesTable() const
{
	const QChar sep = ',';
	QString str;
	QTextStream out(&str);
	const auto &bytes = m_byteReceiveTimesDialog->bytes();

	out << "Time (ms)" << sep
		<< "Decimal" << sep
		<< "Hex" << sep
		<< "Binary" << sep
		<< "Character" << endl;
	for (const auto &byte : bytes) {
		unsigned char b = byte.value;
		QString ch(b);
		if (b < ' ' || b > '~')
			ch.clear();

		QString bin = QString::number(b, 2);
		while (bin.length() < 8)
			bin.push_front('0');

		QString hex = QString::number(b, 16);
		while (bin.length() < 2)
			bin.push_front('0');
		out << byte.ms << sep
			<< QString::number(b) << sep
			<< bin << sep
			<< hex << sep
			<< ch << endl;
	}

	return str.toLatin1();
}
