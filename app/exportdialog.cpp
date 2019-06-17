#include "exportdialog.h"
#include "line.h"
#include "plaintextview.h"
#include "hexview.h"
#include "config.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

ExportDialog::ExportDialog(const QByteArray &rawData,
						   PlainTextView *plaintTextView,
						   HexView *hexView,
						   QWidget *parent)
	: QDialog(parent)
	, m_rawData(rawData)
	, m_plainTextView(plaintTextView)
	, m_hexView(hexView)
	, m_rawDataButton(new QRadioButton("Binary file"))
	, m_plainTextButton(new QRadioButton("Plain text file"))
	, m_hexButton(new QRadioButton("Formatted hex file"))
{
	QPushButton *exportButton = new QPushButton("Export");
	m_rawDataButton->setChecked(true);

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("Export as a"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Horizontal));
	layout->addWidget(m_rawDataButton);
	layout->addWidget(m_plainTextButton);
	layout->addWidget(m_hexButton);
	layout->addWidget(exportButton);

	connect(exportButton, &QPushButton::clicked, this, &ExportDialog::exportData);
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
	static const char *anyTypeFilter = "Any type (*)";

	Config c;
	QStringList filters;

#ifdef Q_OS_WIN
	if (m_plainTextButton->isChecked())
		filters << plainTextFilter;
	else if (m_hexButton->isChecked())
		filters << plainTextFilter;
#endif

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

		if (filter == binaryFilter && !path.endsWith(".bin")) {
			path.append(".bin");
			if (!confirmReplaceFile(this, path))
				continue;
		} else if (filter == plainTextFilter && !path.endsWith(".txt")) {
			path.append(".txt");
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

	if (file.write(data) < 0) {
		QMessageBox::critical(this, "Export error", "Failed to write to file: " + file.errorString());
		reject();
		return;
	}

	accept();
}
