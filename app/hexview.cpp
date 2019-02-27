#include "hexview.h"
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QScrollBar>

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_plainTextEdit(new QPlainTextEdit)
	, m_bytes(0)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);
	layout->addWidget(m_plainTextEdit);

	m_plainTextEdit->setReadOnly(true);
	m_plainTextEdit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
	m_plainTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void HexView::insertData(const QByteArray &data)
{
	static auto toHex = [](int n) -> char {
		if (n < 10)
			return char(n + '0');
		else
			return char(n - 10 + 'A');
	};

	int scrollValue = m_plainTextEdit->verticalScrollBar()->value();
	bool atBottom = scrollValue == m_plainTextEdit->verticalScrollBar()->maximum();

	const int lineLength = 48 + 18 + 1;
	int i = 0;
	int bytesToWrite = 16 - m_bytes % 16;
	if (bytesToWrite != 16) {
		m_plainTextEdit->moveCursor(QTextCursor::End);
		QTextCursor cursor = m_plainTextEdit->textCursor();

		int byteInLine = m_bytes % 16;

		int lineCursorPosition = byteInLine * 3;
		if (byteInLine >= 8)
			++lineCursorPosition;
		int lines = m_bytes / 16;
		cursor.setPosition(lines * lineLength + lineCursorPosition);

		for (i = 0; i < data.size() && i < bytesToWrite; ++i) {
			unsigned char byte = static_cast<unsigned char>(data[i]);

			QString byteString(2, ' ');
			byteString[1] = toHex((byte & 0x0F));
			byteString[0] = toHex(((byte >> 4) & 0x0F));
			cursor.deleteChar();
			cursor.deleteChar();
			cursor.insertText(byteString);
			cursor.setPosition(cursor.position() + (byteInLine == 7 ? 2 : 1));

			++byteInLine;
		}

		byteInLine = m_bytes % 16;
		cursor.setPosition(lines * lineLength + 48 + 2 + byteInLine);
		QString text;
		for (i = 0; i < data.size() && i < bytesToWrite; ++i) {
			unsigned char byte = static_cast<unsigned char>(data[i]);
			char normalizedByte;
			if (byte >= 32 && byte <= 126)
				normalizedByte = char(byte);
			else
				normalizedByte = '.';
			text.append(normalizedByte);
			cursor.deleteChar();
		}
		cursor.insertText(text);

		m_bytes += qMin(data.size(), bytesToWrite);
	}

	int bytesToPrint = (data.size() - i);

	if (bytesToPrint == 0)
		return;

	int linesToPrint = bytesToPrint / 16;
	if (bytesToPrint % 16 != 0)
		++linesToPrint;

	QString text(lineLength * linesToPrint, ' ');
	for (int l = 0; l < linesToPrint; ++l)
		text[lineLength * l + lineLength - 1] = '\n';

	int line = 0;
	while (i < data.size()) {
		for (int byteInLine = 0; byteInLine < 16 && i < data.size(); ++byteInLine) {
			unsigned char byte = static_cast<unsigned char>(data[i++]);

			int position1 = byteInLine * 3;
			if (byteInLine >= 8)
				++position1;

			int position2 = 48 + 2 + byteInLine;

			int lineStart = line * lineLength;
			text[lineStart + position1] = toHex(((byte >> 4) & 0x0F));
			text[lineStart + position1 + 1] = toHex((byte & 0x0F));

			char normalizedByte;
			if (byte >= 32 && byte <= 126)
				normalizedByte = char(byte);
			else
				normalizedByte = '.';
			text[lineStart + position2] = normalizedByte;
		}
		++line;
	}

	m_plainTextEdit->moveCursor(QTextCursor::End);
	m_plainTextEdit->insertPlainText(text);
	m_bytes += bytesToPrint;

	if (atBottom)
		m_plainTextEdit->verticalScrollBar()->setValue(m_plainTextEdit->verticalScrollBar()->maximum());
	else
		m_plainTextEdit->verticalScrollBar()->setValue(scrollValue);
}

QString HexView::toPlainText() const
{
	return m_plainTextEdit->toPlainText();
}

void HexView::clear()
{
	m_plainTextEdit->clear();
	m_text.clear();
	m_bytes = 0;
}
