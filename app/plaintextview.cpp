#include "plaintextview.h"
#include <QTextBrowser>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QToolTip>

PlainTextView::PlainTextView(QWidget *parent)
	: QWidget(parent)
	, m_edit(new QTextBrowser)
	, m_lastChar('\0')
	, m_colorSpecialCharacters(false)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);
	layout->addWidget(m_edit);

	m_edit->setUndoRedoEnabled(false);
	m_edit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
	m_edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_edit->setOpenLinks(false);
	m_edit->setOpenExternalLinks(false);

	m_defaultTextColor = m_edit->currentCharFormat().foreground().color();


	connect(m_edit, &QTextBrowser::anchorClicked, [this](QUrl link) {
		QString s = link.toString();
		if (s.size() <= 4)
			return;
		s.remove(s.size() - 4, 4);
		unsigned char c = static_cast<unsigned char>(s.toInt());

		QString charInfo = "Dec: " + QString::number(int(c)).rightJustified(3, ' ') + ", "
						   "Hex: " + QString::number(int(c), 16).rightJustified(2, '0') + ", "
						   "Oct: " + QString::number(int(c), 8).rightJustified(3, ' ') + ", "
						   "Bin: " + QString::number(int(c), 2).rightJustified(8, '0');

		QToolTip::showText(cursor().pos(), charInfo, this);
	});
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

	QTextCursor cursor = m_edit->textCursor();
	cursor.movePosition(QTextCursor::End);
	m_edit->setTextCursor(cursor);
	QTextCharFormat tcf = m_edit->currentCharFormat();
	tcf.setForeground(QBrush(m_defaultTextColor));
	m_edit->setCurrentCharFormat(tcf);

	QString tmp;
	for (const unsigned char c : data) {
		if (m_lastChar == '\r')
			if (c != '\n')
				tmp.append('\n');

		const char *replace = nullptr;
		if (c == '\r') replace = "&lt;CR&gt;";
		else if (c == '\n') replace = "&lt;LF&gt;";
		else if (c == '\0') replace = "&lt;NUL&gt;";
		else if (c == '\x01') replace = "&lt;SOH&gt;";
		else if (c == '\x02') replace = "&lt;STX&gt;";
		else if (c == '\x03') replace = "&lt;ETX&gt;";
		else if (c == '\x04') replace = "&lt;EOT&gt;";
		else if (c == '\x05') replace = "&lt;ENQ&gt;";
		else if (c == '\x06') replace = "&lt;ACK&gt;";
		else if (c == '\x07') replace = "&lt;BEL&gt;";
		else if (c == '\x08') replace = "&lt;BS&gt;";
		else if (c == '\x0B') replace = "&lt;VT&gt;";
		else if (c == '\x1B') replace = "&lt;ESC&gt;";
		else if (c == '\x7F') replace = "&lt;DEL&gt;";

		if (replace || c > 0x7f || c < 0x20) {
			if (!tmp.isEmpty()) {
				m_edit->insertHtml(tmp.toHtmlEscaped());
				tmp.clear();
			}
			if (replace) {
				if (m_colorSpecialCharacters)
					tmp.append("<a href=\"" + QString::number(int(c)) + ".com\" style=\"color: blue\">");
				tmp.append(replace);
				if (m_colorSpecialCharacters)
					tmp.append("</a>");
				if (c == '\n')
					tmp.append("<br>");
				m_edit->insertHtml(tmp);
				tmp.clear();
			} else if (c > 0x7f || c < 0x20) {
				static const char hex[16] = {'0', '1', '2', '3', '4', '5',
											 '6', '7', '8', '9', 'A', 'B',
											 'C', 'D', 'E', 'F'};
				if (m_colorSpecialCharacters)
					tmp.append("<a href=\"" + QString::number(int(c)) + ".com\" style=\"color: red\">");
				tmp.append("&lt;");
				tmp.append(hex[(c >> 4) & 0x0f]);
				tmp.append(hex[c & 0x0f]);
				tmp.append("&gt;");
				if (m_colorSpecialCharacters)
					tmp.append("</a>");
				m_edit->insertHtml(tmp);
				tmp.clear();
			}
		} else {
			tmp.append(c);
		}

		m_lastChar = c;
	}

	if (!tmp.isEmpty())
		m_edit->insertHtml(tmp.toHtmlEscaped());

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

void PlainTextView::setColorSpecialCharacters(bool colorSpecialCharaters)
{
	m_colorSpecialCharacters = colorSpecialCharaters;
}

QString PlainTextView::toPlainText() const
{
	return m_edit->toPlainText();
}
