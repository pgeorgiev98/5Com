#include "bytereceivetimesdialog.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QTime>
#include <QFontDatabase>
#include <QHeaderView>
#include <QScrollBar>

ByteReceiveTimesDialog::ByteReceiveTimesDialog(int height, QWidget *parent)
	: QDialog(parent)
	, m_table(new QTableWidget)
	, m_rowHeight(20)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(m_table);

	m_table->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	QFontMetrics fm = m_table->fontMetrics();
	m_table->setAlternatingRowColors(true);
	m_table->setColumnCount(5);
	m_table->setColumnWidth(0, fm.width("  Time (ms)  "));
	m_table->setColumnWidth(1, fm.width(" Dec "));
	m_table->setColumnWidth(2, fm.width(" Hex "));
	m_table->setColumnWidth(3, fm.width(" 00000000 "));
	m_table->setColumnWidth(4, fm.width(" Char "));
	m_table->setHorizontalHeaderLabels({"Time (ms)", "Dec", "Hex", "Bin", "Char"});
	m_table->verticalHeader()->hide();
	m_rowHeight = fm.height();

	setMinimumWidth(m_table->horizontalHeader()->length());
	setFixedWidth(m_table->horizontalHeader()->length() + 50);
	setFixedHeight(height);
}

int ByteReceiveTimesDialog::bytesCount() const
{
	return m_bytes.count();
}

const QVector<ByteReceiveTimesDialog::Byte> &ByteReceiveTimesDialog::bytes() const
{
	return m_bytes;
}

void ByteReceiveTimesDialog::removeFromBegining(int bytes)
{
	for(int i = 0; i < bytes; ++i)
		m_table->removeRow(0);
}

void ByteReceiveTimesDialog::insertData(const QByteArray &data)
{
	if (m_bytes.isEmpty())
		m_startTime = QTime::currentTime();

	QTime receiveTime = QTime::currentTime();
	int ms = m_startTime.msecsTo(receiveTime);
	int row = m_table->rowCount();
	for (int i = 0; i < data.count(); ++i)
		m_table->insertRow(m_table->rowCount());

	bool atBottom = m_table->verticalScrollBar()->value() == m_table->verticalScrollBar()->maximum();

	for (const unsigned char byte : data) {
		m_bytes.append(Byte {ms, byte});

		QString ch(byte);
		if (byte < ' ' || byte > '~')
			ch = "�";

		QString bin = QString::number(byte, 2);
		while (bin.length() < 8)
			bin.push_front('0');

		QString hex = QString::number(byte, 16);
		while (bin.length() < 2)
			bin.push_front('0');

		int col = 0;
		for (QString label : {QString::number(ms),
			 QString::number(int(byte), 10),
			 QString::number(int(byte), 16),
			 bin, ch}) {
			QTableWidgetItem *item = new QTableWidgetItem(label);
			item->setTextAlignment(Qt::AlignRight);
			m_table->setItem(row, col++, item);
		}
		m_table->setRowHeight(row, m_rowHeight);

		++row;
	}

	if (atBottom)
		m_table->scrollToBottom();
}

void ByteReceiveTimesDialog::clear()
{
	m_table->setRowCount(0);
	m_bytes.clear();
}
