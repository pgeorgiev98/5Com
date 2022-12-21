#include "bytereceivetimesdialog.h"
#include "common.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QTime>
#include <QHeaderView>
#include <QScrollBar>

inline int textWidth(const QFontMetrics &fm, const QString &text)
{
#if QT_VERSION >= 0x050B00
	return fm.horizontalAdvance(text);
#else
	return fm.width(text);
#endif
}

ByteReceiveTimesDialog::ByteReceiveTimesDialog(int height, QWidget *parent)
	: QDialog(parent)
	, m_table(new QTableWidget)
	, m_rowHeight(20)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(m_table);

	setFont(getFixedFont());

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

void ByteReceiveTimesDialog::setFont(QFont font)
{
	m_table->setFont(font);
	QFontMetrics fm = m_table->fontMetrics();
	m_table->setAlternatingRowColors(true);
	m_table->setColumnCount(5);
	m_table->setColumnWidth(0, textWidth(fm, "  Time (ms)  "));
	m_table->setColumnWidth(1, textWidth(fm, " Dec "));
	m_table->setColumnWidth(2, textWidth(fm, " Hex "));
	m_table->setColumnWidth(3, textWidth(fm, " 00000000 "));
	m_table->setColumnWidth(4, textWidth(fm, " Char "));
	m_table->setHorizontalHeaderLabels({"Time (ms)", "Dec", "Hex", "Bin", "Char"});
	m_table->verticalHeader()->hide();
	m_rowHeight = fm.height();
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

	for (char c : data) {
		unsigned char byte = static_cast<unsigned char>(c);
		m_bytes.append(Byte {ms, byte});

		const auto labels = {
			QString::number(ms),
			QString::number(int(byte), 10),
			QString::number(int(byte), 16).rightJustified(2, '0'),
			QString::number(int(byte), 2).rightJustified(8, '0'),
			byte < ' ' || byte > '~' ? QString("\uFFFD") : QString(byte),
		};

		int col = 0;
		for (const auto& label : labels) {
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
