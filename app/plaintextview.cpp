#include "plaintextview.h"
#include "common.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QShortcut>
#include <QGuiApplication>

#define MINIMUM_WIDTH 80
#define MINIMUM_HEIGHT 80

inline qreal textWidth(const QFontMetricsF &fm, const QString &text)
{
	return fm.averageCharWidth() * text.size();
}

PlainTextView::PlainTextView(QWidget *parent)
	: QWidget(parent)
	, m_font(getFixedFont())
	, m_fm(m_font)
	, m_width(MINIMUM_WIDTH)
	, m_height(MINIMUM_HEIGHT)
	, m_padding(qRound(m_fm.averageCharWidth()))
	, m_rows({Row()})
{
	QPalette pal = palette();
	m_backgroundColor = pal.base().color();
	m_textColor = pal.text().color();
	m_standardHexCodeColor = Qt::blue;
	m_nonStandardHexCodeColor = Qt::red;
	m_hoverTextColor = pal.link().color();
	m_selectionColor = pal.highlight().color();
	m_selectedTextColor = pal.highlightedText().color();

	pal.setColor(QPalette::Window, m_backgroundColor);
	setAutoFillBackground(true);
	setPalette(pal);
	setMinimumWidth(m_width);
	setMinimumHeight(m_height);
	setMouseTracking(true);

	QShortcut *copyShortcut = new QShortcut(QKeySequence::Copy, this);
	QShortcut *selectAllShortcut = new QShortcut(QKeySequence::SelectAll, this);

	connect(copyShortcut, &QShortcut::activated, this, &PlainTextView::copySelection);
	connect(selectAllShortcut, &QShortcut::activated, this, &PlainTextView::selectAll);
}

QString PlainTextView::toPlainText() const
{
	QString text;
	for (const auto &row : m_rows) {
		if (row.elements.isEmpty())
			continue;

		for (const auto &el : row.elements)
			text.append(el.str);
		text.append('\n');
	}
	return text;
}

QPoint PlainTextView::getByteCoordinates(int index) const
{
	const int rowHeight = qRound(m_fm.height());
	qreal x = m_padding;
	int y = int(m_padding);

	for (const Row &r : m_rows) {
		for (const Element &el : r.elements) {
			int elementByteCount =
					el.type == Element::PlainText ? el.str.size() : 1;
			if (index >= el.rawStartIndex &&
					index < el.rawStartIndex + elementByteCount)
				return QPoint(int(x + m_fm.averageCharWidth() * (index - el.rawStartIndex)), y);

			x += textWidth(m_fm, el.str) + 1;
		}
		x = m_padding;
		y += rowHeight;
	}

	return QPoint(0, 0);
}

std::optional<ByteSelection> PlainTextView::selection() const
{
	return m_selection;
}

void PlainTextView::setData(const QByteArray &data)
{
	m_width = minimumWidth();
	m_height = minimumHeight();
	clear();
	insertData(data);
}

void PlainTextView::insertData(const QByteArray &data)
{
	const int start = m_data.size();
	m_data.append(data);

	int oldRowCount = m_rows.size();

	int lastType = m_rows.last().elements.isEmpty() ? -1 : m_rows.last().elements.last().type;
	for (int i = start; i < m_data.size(); ++i) {
		unsigned char b = static_cast<unsigned char>(m_data[i]);

		if (b != '\n' && m_data.size() > i - 1 && m_data[i - 1] == '\r')
			m_rows.append(Row());

		const auto &info = byteInfos[b];
		if (info.type != 0 || info.type != lastType) {
			m_rows.last().elements.append({i, Element::Type(info.type), info.str});
			lastType = info.type;
		} else {
			m_rows.last().elements.last().str.append(info.str);
		}

		if (b == '\n')
			m_rows.append(Row());
	}

	for (int i = oldRowCount - 1; i < m_rows.size(); ++i)
		m_rows[i].width = calculateRowWidth(m_rows[i]);
	recalculateSize(oldRowCount - 1, width());
}

void PlainTextView::trimData(int visibleBytesCount)
{
	Q_ASSERT(visibleBytesCount <= m_data.size());
	int previousRowCount = m_rows.size();
	int firstByteIndex = m_data.size() - visibleBytesCount;

	int firstRowIndex = -1, firstElementIndex = -1;
	for (int r = 0; r < m_rows.size(); ++r) {
		const Row &row = m_rows[r];
		for (int el = 0; el < row.elements.size(); ++el) {
			const Element &element = row.elements[el];
			if (element.rawStartIndex == firstByteIndex) {
				firstRowIndex = r;
				firstElementIndex = el;
				r = m_rows.size();
				break;
			} else if (element.rawStartIndex > firstByteIndex) {
				if (el > 0) {
					firstRowIndex = r;
					firstElementIndex = el - 1;
				} else {
					Q_ASSERT(r > 0);
					firstRowIndex = r - 1;
					firstElementIndex = m_rows[r - 1].elements.size() - 1;
				}
				r = m_rows.size();
				break;
			}
		}
	}

	// Delete the unneeded rows and elements
	m_rows.remove(0, firstRowIndex);
	m_rows.first().elements.remove(0, firstElementIndex);

	// Trim some data from the first element
	Element &el = m_rows.first().elements.first();
	for (int i = el.rawStartIndex; i < firstByteIndex; ++i) {
		unsigned char byte = static_cast<unsigned char>(m_data[i]);
		ByteInfo info = byteInfos[byte];
		el.str.remove(0, info.str.size());
	}
	if (el.str.isEmpty()) {
		m_rows.first().elements.remove(0);
		if (m_rows.first().elements.isEmpty())
			m_rows.remove(0);
	}

	// Update some element variables
	for (Row &r : m_rows)
		for (Element &e : r.elements)
			e.rawStartIndex -= firstByteIndex;
	el.rawStartIndex = 0;
	m_pressedByteIndex -= firstByteIndex;

	// Update the selection
	if (m_selection) {
		int begin = m_selection->begin - firstByteIndex;
		if (begin >= 0) {
			m_selection->begin = begin;
		} else {
			m_selection->begin = 0;
			m_selection->count += begin;
			if (m_selection->count <= 0)
				m_selection.reset();
		}
	}

	// Delete the raw bytes
	m_data.remove(0, firstByteIndex);

	// Recalculate the widget size
	m_rows.first().width = calculateRowWidth(m_rows.first());
	recalculateSize(0, MINIMUM_WIDTH);

	// Scrollbar fix
	int rowsRemoved = previousRowCount - m_rows.size();
	emit mustScrollUp(qRound(rowsRemoved * m_fm.height()));
}

void PlainTextView::recalculateSize(int startRow, int minimumWidth)
{
	int width = minimumWidth;
	for (int i = startRow; i < m_rows.size(); ++i)
		if (m_rows[i].width > width)
			width = m_rows[i].width;

	m_width = width;
	m_height = qMax(MINIMUM_HEIGHT, qRound((m_rows.size() + 1) * m_fm.height()));
	if (m_width != this->width() || m_height != this->height())
		resize(m_width, m_height);
	update();
}

int PlainTextView::calculateRowWidth(const Row &row)
{
	qreal x = 2 * m_padding;
	for (const Element &element : row.elements)
		x += textWidth(m_fm, element.str) + 1;
	return qRound(x);
}

void PlainTextView::clear()
{
	m_data.clear();
	m_rows = {Row()};
	m_selection.reset();
	recalculateSize(0, MINIMUM_WIDTH);
}

void PlainTextView::copySelection()
{
	if (!m_selection.has_value())
		return;

	QString text;
	for (int i = m_selection->begin; i < m_selection->begin + m_selection->count; ++i) {
		unsigned char b = static_cast<unsigned char>(m_data[i]);
		text.append(byteInfos[b].str);
		if ((b == '\n') || (b == '\r' && i + 1 < m_selection->begin + m_selection->count && m_data[i + 1] != '\n'))
			text.append('\n');
	}
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(text);
}

void PlainTextView::selectAll()
{
	if (m_data.isEmpty())
		m_selection.reset();
	else
		m_selection = ByteSelection(0, m_data.size());

	update();
}

void PlainTextView::selectNone()
{
	m_selection.reset();
	update();
}

void PlainTextView::highlight(ByteSelection selection)
{
	m_selection = selection;
	update();
}

void PlainTextView::setFont(QFont font)
{
	m_font = font;
	m_fm = QFontMetricsF(m_font);
	m_padding = qRound(m_fm.averageCharWidth());
	for (Row &r : m_rows)
		r.width = calculateRowWidth(r);
	recalculateSize(0, MINIMUM_WIDTH);
}

void PlainTextView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QFont underlinedFont = m_font;
	underlinedFont.setUnderline(true);

	const int rowHeight = qRound(m_fm.height());

	QRect rect = event->rect();
	const int startRow = qBound(0, int((rect.y() - m_fm.ascent() - m_padding) / rowHeight - 2), m_rows.size() - 1);
	const int endRow = qBound(0, int((rect.y() + rect.height() - m_fm.ascent() - m_padding) / rowHeight + 2), m_rows.size());

	static QColor colors[3] = {
		m_textColor,
		m_standardHexCodeColor,
		m_nonStandardHexCodeColor,
	};

	ByteSelection selection = m_selection.value_or(ByteSelection(0, 0));

	for (int rowIndex = startRow; rowIndex < endRow; ++rowIndex) {
		const Row &row = m_rows[rowIndex];
		qreal x = m_padding;
		int y = int(m_fm.ascent() + m_padding + rowIndex * rowHeight);
		for (int elementIndex = 0; elementIndex < row.elements.size() && x <= rect.right(); ++elementIndex) {
			const Element &element = row.elements[elementIndex];
			qreal width = textWidth(m_fm, element.str);
			if (x + width + 1 >= rect.left()) {
				int elementBytesCount = element.type == Element::PlainText ? element.str.size() : 1;
				if (element.rawStartIndex < selection.begin + selection.count &&
						element.rawStartIndex + elementBytesCount > selection.begin) {
					int firstIndex = qMax(0, selection.begin - element.rawStartIndex);
					int selectionLength = element.type == Element::Type::PlainText ?
								qMin(selection.begin + selection.count - element.rawStartIndex - firstIndex, element.str.size() - firstIndex) :
								element.str.size();

					QRectF selection;

					selection.setX(x + textWidth(m_fm, element.str.left(firstIndex)));
					selection.setY(y - m_fm.ascent());

					selection.setWidth(textWidth(m_fm, element.str.mid(firstIndex, selectionLength)));
					selection.setHeight(rowHeight);

					if (firstIndex + selectionLength == element.str.size())
						selection.setWidth(selection.width() + 1);

					painter.fillRect(selection, m_selectionColor);
				}

				QRectF rect(x, y - m_fm.ascent(), width, rowHeight);
				painter.setFont(rect.contains(m_mousePos) && element.type != Element::Type::PlainText ? underlinedFont : m_font);
				painter.setPen(colors[element.type]);
				painter.drawText(qRound(x), y, element.str);
			}
			x += width + 1;
		}
	}
}

void PlainTextView::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	m_mousePos = pos;

	auto cursorShape = Qt::CursorShape::ArrowCursor;

	if (m_mousePressPos.has_value() && (m_mousePressPos.value() - pos).manhattanLength() > 3) {
		m_selection = getSelectedBytes();
		cursorShape = Qt::CursorShape::IBeamCursor;
	} else {
		int byteIndex = getByteIndexAtPos(pos);
		if (byteIndex != -1) {
			unsigned char byte = static_cast<unsigned char>(m_data[byteIndex]);
			if (byteInfos[byte].type == Element::Type::PlainText)
				cursorShape = Qt::CursorShape::IBeamCursor;
			else
				cursorShape = Qt::CursorShape::PointingHandCursor;
		}
	}

	setCursor(cursorShape);

	update();
}

void PlainTextView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		QPoint pos = event->pos();
		m_mousePressPos = pos;
		m_mousePos = pos;
		m_pressedByteIndex = getByteIndexAtPos(pos);
		m_selection.reset();
		update();
	} else if (event->button() == Qt::RightButton) {
		QMenu menu(this);
		menu.popup(cursor().pos());

		QAction copyAction("&Copy formatted");
		QAction selectAllAction("Select All");
		QAction selectNoneAction("Select None");
		QAction highlightInHexViewAction("Highlight in Hex View");

		menu.addAction(&copyAction);
		menu.addAction(&selectAllAction);
		menu.addAction(&selectNoneAction);
		menu.addAction(&highlightInHexViewAction);

		copyAction.setEnabled(m_selection.has_value());
		selectAllAction.setDisabled(m_data.isEmpty());
		selectNoneAction.setEnabled(m_selection.has_value());
		highlightInHexViewAction.setEnabled(m_selection.has_value());

		QAction *a = menu.exec();

		if (a == &copyAction) {
			copySelection();
		} else if (a == &selectAllAction) {
			selectAll();
		} else if (a == &selectNoneAction) {
			selectNone();
		} else if (a == &highlightInHexViewAction) {
			emit highlightInHexView(selection().value());
		}
	}
}

void PlainTextView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		QPoint pos = event->pos();
		m_mousePos = pos;

		int byteIndex = getByteIndexAtPos(pos);
		if (byteIndex != -1 && byteIndex == m_pressedByteIndex) {
			int b = int(static_cast<unsigned char>(m_data[byteIndex]));
			if (byteInfos[b].type != Element::Type::PlainText) {
				QString byteInfo = QString("Dec: %1, Hex: %2, Oct: %3, Bin: %4")
						.arg(QString::number(b).rightJustified(3, ' '),
							 QString::number(b, 16).rightJustified(2, '0').toUpper(),
							 QString::number(b, 8).rightJustified(3, ' '),
							 QString::number(b, 2).rightJustified(8, '0'));
				QString text;
				if (!byteInfos[b].name.isEmpty())
					text = QString("%1 - %2\n\n").arg(byteInfos[b].str, byteInfos[b].name);
				text += byteInfo;
				QToolTip::showText(cursor().pos(), text, this);
			}
		}

		m_mousePressPos.reset();
	}
}

void PlainTextView::leaveEvent(QEvent *)
{
	m_mousePos = QPoint();
	setCursor(Qt::CursorShape::ArrowCursor);
	update();
}

int PlainTextView::getByteIndexAtPos(QPoint pos, bool selecting) const
{
	int row = int((pos.y() - m_padding) / m_fm.height());
	if (selecting) {
		if (row < 0)
			return 0;
		else if (row >= m_rows.size())
			return m_data.size();
	} else if (row < 0 || row >= m_rows.size()) {
		return -1;
	}

	const Row &r = m_rows[row];

	qreal x = m_padding;
	for (int e = 0; e < r.elements.size(); ++e) {
		const Element &element = r.elements[e];
		qreal width = textWidth(m_fm, element.str) + 1.0;
		if (pos.x() < x) {
			return element.rawStartIndex;
		} else if (pos.x() >= x && pos.x() <= x + width) {
			int index;
			if (element.type != Element::Type::PlainText) {
				if (selecting)
					index = qRound((pos.x() - x) / textWidth(m_fm, element.str));
				else
					return element.rawStartIndex;
			} else {
				index = qRound((pos.x() - x) / m_fm.averageCharWidth());
			}
			return element.rawStartIndex + index;
		} else if (x + width > pos.x()) {
			break;
		}
		x += width;
	}

	if (selecting) {
		if (r.elements.isEmpty()) {
			if (row == 0)
				return 0;
			else {
				const Element &element = m_rows[row - 1].elements.last();
				int len = element.type == Element::Type::PlainText ?
							element.str.size() : 1;
				return element.rawStartIndex + len;
			}
		} else {
			const Element &element = r.elements.last();
			int len = element.type == Element::Type::PlainText ?
						element.str.size() : 1;
			return element.rawStartIndex + len;
		}
	} else {
		return -1;
	}
}


std::optional<ByteSelection> PlainTextView::getSelectedBytes() const
{
	int begin = 0;
	if (m_pressedByteIndex != -1)
		begin = m_pressedByteIndex;

	int end = getByteIndexAtPos(m_mousePos, true);
	if (end == -1)
		return std::nullopt;

	if (begin > end)
		std::swap(begin, end);

	if (begin == end)
		return std::nullopt;

	return std::make_optional<ByteSelection>(begin, end - begin);
}


const PlainTextView::ByteInfo PlainTextView::byteInfos[256] = {
	{1, "<NUL>", "Null"},
	{1, "<SOH>", "Start of heading"},
	{1, "<STX>", "Start of text"},
	{1, "<ETX>", "End of text"},
	{1, "<EOT>", "End of transmission"},
	{1, "<ENQ>", "Enquiry"},
	{1, "<ACK>", "Acknowledge"},
	{1, "<BEL>", "Bell"},
	{1,  "<BS>", "Backspace"},
	{1, "<TAB>", "Horizontal tab"},
	{1,  "<LF>", "Line feed"},
	{1,  "<VT>", "Vertical tab"},
	{2,  "<0C>", ""},
	{1,  "<CR>", "Carriage return"},

	{2,  "<0E>", ""}, {2,  "<0F>", ""}, {2,  "<10>", ""}, {2,  "<11>", ""},
	{2,  "<12>", ""}, {2,  "<13>", ""}, {2,  "<14>", ""}, {2,  "<15>", ""},
	{2,  "<16>", ""}, {2,  "<17>", ""}, {2,  "<18>", ""}, {2,  "<19>", ""},
	{2,  "<1A>", ""},

	{1, "<ESC>", "Escape"},

	{2,  "<1C>", ""}, {2,  "<1D>", ""}, {2,  "<1E>", ""}, {2,  "<1F>", ""},
	{0,     " ", ""}, {0,     "!", ""}, {0,    "\"", ""}, {0,     "#", ""},
	{0,     "$", ""}, {0,     "%", ""}, {0,     "&", ""}, {0,     "'", ""},
	{0,     "<", ""}, {0,     ">", ""}, {0,     "*", ""}, {0,     "+", ""},
	{0,     ",", ""}, {0,     "-", ""}, {0,     ".", ""}, {0,     "/", ""},
	{0,     "0", ""}, {0,     "1", ""}, {0,     "2", ""}, {0,     "3", ""},
	{0,     "4", ""}, {0,     "5", ""}, {0,     "6", ""}, {0,     "7", ""},
	{0,     "8", ""}, {0,     "9", ""}, {0,     ":", ""}, {0,     ";", ""},
	{0,     "<", ""}, {0,     "=", ""}, {0,     ">", ""}, {0,     "?", ""},
	{0,     "@", ""}, {0,     "A", ""}, {0,     "B", ""}, {0,     "C", ""},
	{0,     "D", ""}, {0,     "E", ""}, {0,     "F", ""}, {0,     "G", ""},
	{0,     "H", ""}, {0,     "I", ""}, {0,     "J", ""}, {0,     "K", ""},
	{0,     "L", ""}, {0,     "M", ""}, {0,     "N", ""}, {0,     "O", ""},
	{0,     "P", ""}, {0,     "Q", ""}, {0,     "R", ""}, {0,     "S", ""},
	{0,     "T", ""}, {0,     "U", ""}, {0,     "V", ""}, {0,     "W", ""},
	{0,     "X", ""}, {0,     "Y", ""}, {0,     "Z", ""}, {0,     "<", ""},
	{0,    "\\", ""}, {0,     ">", ""}, {0,     "^", ""}, {0,     "_", ""},
	{0,     "`", ""}, {0,     "a", ""}, {0,     "b", ""}, {0,     "c", ""},
	{0,     "d", ""}, {0,     "e", ""}, {0,     "f", ""}, {0,     "g", ""},
	{0,     "h", ""}, {0,     "i", ""}, {0,     "j", ""}, {0,     "k", ""},
	{0,     "l", ""}, {0,     "m", ""}, {0,     "n", ""}, {0,     "o", ""},
	{0,     "p", ""}, {0,     "q", ""}, {0,     "r", ""}, {0,     "s", ""},
	{0,     "t", ""}, {0,     "u", ""}, {0,     "v", ""}, {0,     "w", ""},
	{0,     "x", ""}, {0,     "y", ""}, {0,     "z", ""}, {0,     "{", ""},
	{0,     "|", ""}, {0,     "}", ""}, {0,     "~", ""},

	{1, "<DEL>", "Delete"},

	{2,  "<80>", ""}, {2,  "<81>", ""}, {2,  "<82>", ""},
	{2,  "<83>", ""}, {2,  "<84>", ""}, {2,  "<85>", ""}, {2,  "<86>", ""},
	{2,  "<87>", ""}, {2,  "<88>", ""}, {2,  "<89>", ""}, {2,  "<8A>", ""},
	{2,  "<8B>", ""}, {2,  "<8C>", ""}, {2,  "<8D>", ""}, {2,  "<8E>", ""},
	{2,  "<8F>", ""}, {2,  "<90>", ""}, {2,  "<91>", ""}, {2,  "<92>", ""},
	{2,  "<93>", ""}, {2,  "<94>", ""}, {2,  "<95>", ""}, {2,  "<96>", ""},
	{2,  "<97>", ""}, {2,  "<98>", ""}, {2,  "<99>", ""}, {2,  "<9A>", ""},
	{2,  "<9B>", ""}, {2,  "<9C>", ""}, {2,  "<9D>", ""}, {2,  "<9E>", ""},
	{2,  "<9F>", ""}, {2,  "<A0>", ""}, {2,  "<A1>", ""}, {2,  "<A2>", ""},
	{2,  "<A3>", ""}, {2,  "<A4>", ""}, {2,  "<A5>", ""}, {2,  "<A6>", ""},
	{2,  "<A7>", ""}, {2,  "<A8>", ""}, {2,  "<A9>", ""}, {2,  "<AA>", ""},
	{2,  "<AB>", ""}, {2,  "<AC>", ""}, {2,  "<AD>", ""}, {2,  "<AE>", ""},
	{2,  "<AF>", ""}, {2,  "<B0>", ""}, {2,  "<B1>", ""}, {2,  "<B2>", ""},
	{2,  "<B3>", ""}, {2,  "<B4>", ""}, {2,  "<B5>", ""}, {2,  "<B6>", ""},
	{2,  "<B7>", ""}, {2,  "<B8>", ""}, {2,  "<B9>", ""}, {2,  "<BA>", ""},
	{2,  "<BB>", ""}, {2,  "<BC>", ""}, {2,  "<BD>", ""}, {2,  "<BE>", ""},
	{2,  "<BF>", ""}, {2,  "<C0>", ""}, {2,  "<C1>", ""}, {2,  "<C2>", ""},
	{2,  "<C3>", ""}, {2,  "<C4>", ""}, {2,  "<C5>", ""}, {2,  "<C6>", ""},
	{2,  "<C7>", ""}, {2,  "<C8>", ""}, {2,  "<C9>", ""}, {2,  "<CA>", ""},
	{2,  "<CB>", ""}, {2,  "<CC>", ""}, {2,  "<CD>", ""}, {2,  "<CE>", ""},
	{2,  "<CF>", ""}, {2,  "<D0>", ""}, {2,  "<D1>", ""}, {2,  "<D2>", ""},
	{2,  "<D3>", ""}, {2,  "<D4>", ""}, {2,  "<D5>", ""}, {2,  "<D6>", ""},
	{2,  "<D7>", ""}, {2,  "<D8>", ""}, {2,  "<D9>", ""}, {2,  "<DA>", ""},
	{2,  "<DB>", ""}, {2,  "<DC>", ""}, {2,  "<DD>", ""}, {2,  "<DE>", ""},
	{2,  "<DF>", ""}, {2,  "<E0>", ""}, {2,  "<E1>", ""}, {2,  "<E2>", ""},
	{2,  "<E3>", ""}, {2,  "<E4>", ""}, {2,  "<E5>", ""}, {2,  "<E6>", ""},
	{2,  "<E7>", ""}, {2,  "<E8>", ""}, {2,  "<E9>", ""}, {2,  "<EA>", ""},
	{2,  "<EB>", ""}, {2,  "<EC>", ""}, {2,  "<ED>", ""}, {2,  "<EE>", ""},
	{2,  "<EF>", ""}, {2,  "<F0>", ""}, {2,  "<F1>", ""}, {2,  "<F2>", ""},
	{2,  "<F3>", ""}, {2,  "<F4>", ""}, {2,  "<F5>", ""}, {2,  "<F6>", ""},
	{2,  "<F7>", ""}, {2,  "<F8>", ""}, {2,  "<F9>", ""}, {2,  "<FA>", ""},
	{2,  "<FB>", ""}, {2,  "<FC>", ""}, {2,  "<FD>", ""}, {2,  "<FE>", ""},
	{2,  "<FF>", ""}
};
