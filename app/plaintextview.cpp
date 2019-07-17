#include "plaintextview.h"

#include <QPainter>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>

static QColor backgroundColor("#ffffff");
static QColor textColor("#000000");
static QColor standardHexCodeColor(Qt::blue);
static QColor nonStandardHexCodeColor(Qt::red);
static QColor hoverTextColor("#ff0000");
static QColor selectionColor("#0000ff");
static QColor selectedTextColor("#000000");

PlainTextView::PlainTextView(QWidget *parent)
	: QWidget(parent)
	, m_font(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont))
	, m_fm(m_font)
	, m_width(80)
	, m_height(80)
	, m_padding(m_fm.averageCharWidth())
	, m_rows({Row()})
{
	QPalette pal = palette();
	backgroundColor = pal.base().color();
	textColor = pal.text().color();
	hoverTextColor = pal.link().color();
	selectionColor = pal.highlight().color();
	selectedTextColor = pal.highlightedText().color();

	pal.setColor(QPalette::Background, backgroundColor);
	setAutoFillBackground(true);
	setPalette(pal);
	setMinimumWidth(m_width);
	setMinimumHeight(m_height);
	setMouseTracking(true);
}

QString PlainTextView::toPlainText() const
{
	// TODO
	return m_data;
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
	const int oldRowsCount = m_rows.size();
	m_data.append(data);

	int maxX = m_width;
	int lastType = m_rows.last().elements.isEmpty() ? -1 : m_rows.last().elements.last().type;
	for (int i = start; i < m_data.size(); ++i) {
		unsigned char b = static_cast<unsigned char>(m_data[i]);

		const auto &info = byteInfos[b];
		if (info.type != 0 || info.type != lastType) {
			m_rows.last().elements.append({Element::Type(info.type), info.str});
			lastType = info.type;
		} else {
			m_rows.last().elements.last().str.append(info.str);
		}

		if (b == '\n')
			m_rows.append(Row());
	}

	for (int i = oldRowsCount - 1; i < m_rows.size(); ++i) {
		const Row &row = m_rows[i];
		int x = 2 * m_padding;
		for (const Element &element : row.elements)
			x += m_fm.horizontalAdvance(element.str) + 1;
		if (x > maxX)
			maxX = x;
	}

	if (maxX != m_width)
		m_width = maxX;

	m_height = (m_rows.size() + 1) * m_fm.height();
	resize(m_width, m_height);
	repaint();
}

void PlainTextView::clear()
{
	m_data.clear();
	m_rows.clear();
	repaint();
}

void PlainTextView::setColorSpecialCharacters(bool colorSpecialCharacters)
{
	Q_UNUSED(colorSpecialCharacters)
}


void PlainTextView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QFont underlinedFont = m_font;
	underlinedFont.setUnderline(true);

	const int rowHeight = m_fm.height();

	QRect rect = event->rect();
	const int startRow = qBound(0, (rect.y() - m_fm.ascent() - m_padding) / rowHeight - 2, m_rows.size() - 1);
	const int endRow = qBound(0, (rect.y() + rect.height() - m_fm.ascent() - m_padding) / rowHeight + 2, m_rows.size());

	static QColor colors[3] = {
		textColor,
		standardHexCodeColor,
		nonStandardHexCodeColor,
	};

	for (int rowIndex = startRow; rowIndex < endRow; ++rowIndex) {
		const Row &row = m_rows[rowIndex];
		int x = m_padding;
		int y = m_fm.ascent() + m_padding + rowIndex * rowHeight;
		for (int elementIndex = 0; elementIndex < row.elements.size(); ++elementIndex) {
			const Element &element = row.elements[elementIndex];
			int width = m_fm.horizontalAdvance(element.str);
			QRect rect(x, y - m_fm.ascent(), width, rowHeight);

			ElementId elId(rowIndex, elementIndex, 0);
			if (elId.row >= m_selection.first.row && elId.row <= m_selection.second.row &&
				(elId.row > m_selection.first.row || elId.element >= m_selection.first.element) &&
				(elId.row < m_selection.second.row || elId.element <= m_selection.second.element)) {
				int firstIndex = 0, secondIndex = element.str.size();
				if (element.type == Element::Type::PlainText) {
					firstIndex = m_selection.first.row == rowIndex && m_selection.first.element == elementIndex ? m_selection.first.index : 0;
					secondIndex = m_selection.second.row == rowIndex && m_selection.second.element == elementIndex ? m_selection.second.index : element.str.size();
				}

				QRect selection;

				selection.setX(x + m_fm.horizontalAdvance(element.str.left(firstIndex)) - 1);
				selection.setY(y - m_fm.ascent());

				selection.setWidth(m_fm.horizontalAdvance(element.str.left(secondIndex).right(secondIndex - firstIndex)) + 2);
				selection.setHeight(rowHeight);

				painter.fillRect(selection, selectionColor);
			}

			painter.setFont(rect.contains(m_mousePos) && element.type != Element::Type::PlainText ? underlinedFont : m_font);
			painter.setPen(colors[element.type]);
			painter.drawText(x, y, element.str);
			x += width + 1;
		}
	}
}

void PlainTextView::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = event->pos();
	m_mousePos = pos;

	if (m_mousePressPos.has_value()) {
		if ((m_mousePressPos.value() - pos).manhattanLength() > 3) {
			m_selection = getSelectedElements();
			repaint();
		}
	}
}

void PlainTextView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		QPoint pos = event->pos();
		m_mousePressPos = pos;
		m_mousePos = pos;
		m_pressedElement = getElementAtPos(pos);
		m_selection = std::pair<ElementId, ElementId>();
		repaint();
	}
}

void PlainTextView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		QPoint pos = event->pos();
		m_mousePos = pos;

		auto elIdOp = getElementAtPos(pos);
		if (elIdOp.has_value()) {
			ElementId elId = elIdOp.value();
			if (m_pressedElement.has_value()) {
				const auto &el = m_pressedElement.value();
				if (el.row == elId.row && el.element == elId.element) {
					const Element &element = m_rows[elId.row].elements[elId.element];
					if (element.type != Element::Type::PlainText) {
						QToolTip::showText(cursor().pos(), element.str, this);
					}
				}
			}
		}

		m_mousePressPos.reset();
	}
}

void PlainTextView::leaveEvent(QEvent *)
{
	m_mousePos = QPoint();
	repaint();
}

std::optional<PlainTextView::ElementId> PlainTextView::getElementAtPos(QPoint pos)
{
	int row = (pos.y() - m_padding) / m_fm.height();
	if (row < 0 || row >= m_rows.size())
		return std::optional<ElementId>();

	const Row &r = m_rows[row];

	int x = m_padding;
	for (int e = 0; e < r.elements.size(); ++e) {
		const Element &element = r.elements[e];
		int width = m_fm.horizontalAdvance(element.str);
		if (pos.x() >= x && pos.x() <= x + width) {
			int index;
			index = (pos.x() - x) / m_fm.averageCharWidth();
			return ElementId(row, e, index);
		} else if (x + width > pos.x()) {
			break;
		}
		x += width + 1;
	}
	return std::optional<ElementId>();
}

std::pair<PlainTextView::ElementId, PlainTextView::ElementId> PlainTextView::getSelectedElements()
{
	ElementId begin(0, 0, 0), end(INT_MAX, INT_MAX, 0);

	int beginRow = (m_mousePressPos.value().y() - m_padding) / m_fm.height();
	begin.row = beginRow;

	int endRow = (m_mousePos.y() - m_padding) / m_fm.height();
	end.row = endRow;

	if (m_pressedElement.has_value())
		begin = m_pressedElement.value();

	auto endElementOp = getElementAtPos(m_mousePos);
	if (endElementOp.has_value())
		end = endElementOp.value();

	if (begin > end)
		std::swap(begin, end);

	return std::pair<ElementId, ElementId>(begin, end);
}


const PlainTextView::Element PlainTextView::byteInfos[256] = {
	{1, "<NUL>"}, {1, "<SOH>"}, {1, "<STX>"}, {1, "<ETX>"},
	{1, "<EOT>"}, {1, "<ENQ>"}, {1, "<ACK>"}, {1, "<BEL>"},
	{1, "<BS>"}, {1, "<TAB>"}, {1, "<LF>"}, {1, "<VT>"},
	{2, "<0C>"}, {1, "<CR>"}, {2, "<0E>"}, {2, "<0F>"},
	{2, "<10>"}, {2, "<11>"}, {2, "<12>"}, {2, "<13>"},
	{2, "<14>"}, {2, "<15>"}, {2, "<16>"}, {2, "<17>"},
	{2, "<18>"}, {2, "<19>"}, {2, "<1A>"}, {1, "<ESC>"},
	{2, "<1C>"}, {2, "<1D>"}, {2, "<1E>"}, {2, "<1F>"},
	{0, " "}, {0, "!"}, {0, "\""}, {0, "#"},
	{0, "$"}, {0, "%"}, {0, "&"}, {0, "'"},
	{0, "<"}, {0, ">"}, {0, "*"}, {0, "+"},
	{0, ","}, {0, "-"}, {0, "."}, {0, "/"},
	{0, "0"}, {0, "1"}, {0, "2"}, {0, "3"},
	{0, "4"}, {0, "5"}, {0, "6"}, {0, "7"},
	{0, "8"}, {0, "9"}, {0, ":"}, {0, ";"},
	{0, "<"}, {0, "="}, {0, ">"}, {0, "?"},
	{0, "@"}, {0, "A"}, {0, "B"}, {0, "C"},
	{0, "D"}, {0, "E"}, {0, "F"}, {0, "G"},
	{0, "H"}, {0, "I"}, {0, "J"}, {0, "K"},
	{0, "L"}, {0, "M"}, {0, "N"}, {0, "O"},
	{0, "P"}, {0, "Q"}, {0, "R"}, {0, "S"},
	{0, "T"}, {0, "U"}, {0, "V"}, {0, "W"},
	{0, "X"}, {0, "Y"}, {0, "Z"}, {0, "<"},
	{0, "\\"}, {0, ">"}, {0, "^"}, {0, "_"},
	{0, "`"}, {0, "a"}, {0, "b"}, {0, "c"},
	{0, "d"}, {0, "e"}, {0, "f"}, {0, "g"},
	{0, "h"}, {0, "i"}, {0, "j"}, {0, "k"},
	{0, "l"}, {0, "m"}, {0, "n"}, {0, "o"},
	{0, "p"}, {0, "q"}, {0, "r"}, {0, "s"},
	{0, "t"}, {0, "u"}, {0, "v"}, {0, "w"},
	{0, "x"}, {0, "y"}, {0, "z"}, {0, "{"},
	{0, "|"}, {0, "}"}, {0, "~"},
	{1, "<DEL>"}, {2, "<80>"}, {2, "<81>"}, {2, "<82>"},
	{2, "<83>"}, {2, "<84>"}, {2, "<85>"}, {2, "<86>"},
	{2, "<87>"}, {2, "<88>"}, {2, "<89>"}, {2, "<8A>"},
	{2, "<8B>"}, {2, "<8C>"}, {2, "<8D>"}, {2, "<8E>"},
	{2, "<8F>"}, {2, "<90>"}, {2, "<91>"}, {2, "<92>"},
	{2, "<93>"}, {2, "<94>"}, {2, "<95>"}, {2, "<96>"},
	{2, "<97>"}, {2, "<98>"}, {2, "<99>"}, {2, "<9A>"},
	{2, "<9B>"}, {2, "<9C>"}, {2, "<9D>"}, {2, "<9E>"},
	{2, "<9F>"}, {2, "<A0>"}, {2, "<A1>"}, {2, "<A2>"},
	{2, "<A3>"}, {2, "<A4>"}, {2, "<A5>"}, {2, "<A6>"},
	{2, "<A7>"}, {2, "<A8>"}, {2, "<A9>"}, {2, "<AA>"},
	{2, "<AB>"}, {2, "<AC>"}, {2, "<AD>"}, {2, "<AE>"},
	{2, "<AF>"}, {2, "<B0>"}, {2, "<B1>"}, {2, "<B2>"},
	{2, "<B3>"}, {2, "<B4>"}, {2, "<B5>"}, {2, "<B6>"},
	{2, "<B7>"}, {2, "<B8>"}, {2, "<B9>"}, {2, "<BA>"},
	{2, "<BB>"}, {2, "<BC>"}, {2, "<BD>"}, {2, "<BE>"},
	{2, "<BF>"}, {2, "<C0>"}, {2, "<C1>"}, {2, "<C2>"},
	{2, "<C3>"}, {2, "<C4>"}, {2, "<C5>"}, {2, "<C6>"},
	{2, "<C7>"}, {2, "<C8>"}, {2, "<C9>"}, {2, "<CA>"},
	{2, "<CB>"}, {2, "<CC>"}, {2, "<CD>"}, {2, "<CE>"},
	{2, "<CF>"}, {2, "<D0>"}, {2, "<D1>"}, {2, "<D2>"},
	{2, "<D3>"}, {2, "<D4>"}, {2, "<D5>"}, {2, "<D6>"},
	{2, "<D7>"}, {2, "<D8>"}, {2, "<D9>"}, {2, "<DA>"},
	{2, "<DB>"}, {2, "<DC>"}, {2, "<DD>"}, {2, "<DE>"},
	{2, "<DF>"}, {2, "<E0>"}, {2, "<E1>"}, {2, "<E2>"},
	{2, "<E3>"}, {2, "<E4>"}, {2, "<E5>"}, {2, "<E6>"},
	{2, "<E7>"}, {2, "<E8>"}, {2, "<E9>"}, {2, "<EA>"},
	{2, "<EB>"}, {2, "<EC>"}, {2, "<ED>"}, {2, "<EE>"},
	{2, "<EF>"}, {2, "<F0>"}, {2, "<F1>"}, {2, "<F2>"},
	{2, "<F3>"}, {2, "<F4>"}, {2, "<F5>"}, {2, "<F6>"},
	{2, "<F7>"}, {2, "<F8>"}, {2, "<F9>"}, {2, "<FA>"},
	{2, "<FB>"}, {2, "<FC>"}, {2, "<FD>"}, {2, "<FE>"},
	{2, "<FF>"}
};
