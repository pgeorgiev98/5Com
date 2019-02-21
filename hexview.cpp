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
	int scrollValue = m_plainTextEdit->verticalScrollBar()->value();
	bool atBottom = scrollValue == m_plainTextEdit->verticalScrollBar()->maximum();
	const int lineLenght = 48 + 18 + 1;
	int lines = m_text.size() / lineLenght;
	m_plainTextEdit->insertPlainText(data);
	for (int i = 0; i < data.size(); ++i) {
		if (m_bytes % 16 == 0) {
			++lines;
			for (int i = 0; i < lineLenght - 1; ++i)
				m_text.append(' ');
			m_text.append('\n');
		}

		int hexOffset = (lines - 1) * lineLenght;
		hexOffset += (m_bytes % 16) * 3;
		if (m_bytes % 16 >= 8)
			++hexOffset;

		auto toHex = [](int n) -> char {
			if (n < 10)
				return char(n + '0');
			else
				return char(n - 10 + 'A');
		};

		unsigned char b = (unsigned char)data[i];
		m_text[hexOffset++] = toHex((b >> 4) & 0xF);
		m_text[hexOffset++] = toHex(b & 0xF);

		int charOffset = (lines - 1) * lineLenght;
		charOffset += 48 + 2;
		charOffset += (m_bytes % 16);
		if (b >= 32 && b <= 126)
			m_text[charOffset] = b;
		else
			m_text[charOffset] = '.';

		++m_bytes;
	}
	m_plainTextEdit->setPlainText(m_text);

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
