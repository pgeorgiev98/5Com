#include "plaintextview.h"
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QScrollBar>

PlainTextView::PlainTextView(QWidget *parent)
	: QWidget(parent)
	, m_edit(new QPlainTextEdit)
	, m_lastChar('\0')
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);
	layout->addWidget(m_edit);

	m_edit->setReadOnly(true);
	m_edit->setUndoRedoEnabled(false);
	m_edit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
	m_edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void PlainTextView::setData(const QByteArray &data)
{
	int scrollValue = m_edit->verticalScrollBar()->value();
	bool atBottom = scrollValue == m_edit->verticalScrollBar()->maximum();

	clear();
	insertData(data);

	if (atBottom)
		m_edit->verticalScrollBar()->setValue(m_edit->verticalScrollBar()->maximum());
	else
		m_edit->verticalScrollBar()->setValue(scrollValue);
}

void PlainTextView::insertData(const QByteArray &data)
{
	int scrollValue = m_edit->verticalScrollBar()->value();
	bool atBottom = scrollValue == m_edit->verticalScrollBar()->maximum();

	QString tmp;
	for (const unsigned char c : data) {
		if (m_lastChar == '\r')
			if (c != '\n')
				tmp.append('\n');

		const char *replace = nullptr;
		if (c == '\r') replace = "<CR>";
		else if (c == '\n') replace = "<LF>\n";
		else if (c == '\0') replace = "<NUL>";
		else if (c == '\x01') replace = "<SOH>";
		else if (c == '\x02') replace = "<STX>";
		else if (c == '\x03') replace = "<ETX>";
		else if (c == '\x04') replace = "<EOT>";
		else if (c == '\x05') replace = "<ENQ>";
		else if (c == '\x06') replace = "<ACK>";
		else if (c == '\x07') replace = "<BEL>";
		else if (c == '\x08') replace = "<BS>";
		else if (c == '\x0B') replace = "<VT>";
		else if (c == '\x1B') replace = "<ESC>";
		else if (c == '\x7F') replace = "<DEL>";

		if (replace) {
			tmp.append(replace);
		} else if (c > 0x7f) {
			static const char hex[16] = {'0', '1', '2', '3', '4', '5',
										'6', '7', '8', '9', 'A', 'B',
										'C', 'D', 'E', 'F'};
			tmp.append('<');
			tmp.append(hex[(c >> 4) & 0x0f]);
			tmp.append(hex[c & 0x0f]);
			tmp.append('>');
		} else {
			tmp.append(c);
		}

		m_lastChar = c;
	}

	QTextCursor cursor = m_edit->textCursor();
	cursor.movePosition(QTextCursor::MoveOperation::End);
	cursor.insertText(tmp);

	if (atBottom)
		m_edit->verticalScrollBar()->setValue(m_edit->verticalScrollBar()->maximum());
	else
		m_edit->verticalScrollBar()->setValue(scrollValue);
}

void PlainTextView::clear()
{
	m_edit->clear();
	m_lastChar = '\0';
}

QString PlainTextView::toPlainText() const
{
	return m_edit->toPlainText();
}
