#include "plaintextview.h"

#include <QPainter>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPaintEvent>

static QColor backgroundColor("#ffffff");
static QColor textColor("#000000");
static QColor standardHexCodeColor(Qt::blue);
static QColor nonStandardHexCodeColor(Qt::red);
static QColor hoverTextColor("#ff0000");
static QColor selectedColor("#0000ff");
static QColor selectedTextColor("#000000");

PlainTextView::PlainTextView(QWidget *parent)
	: QWidget(parent)
	, m_font(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont))
	, m_fm(m_font)
	, m_rows({Row()})
{
	QPalette pal = palette();
	backgroundColor = pal.base().color();
	textColor = pal.text().color();
	hoverTextColor = pal.link().color();
	selectedColor = pal.highlight().color();
	selectedTextColor = pal.highlightedText().color();

	pal.setColor(QPalette::Background, backgroundColor);
	setAutoFillBackground(true);
	setPalette(pal);
	setMinimumWidth(80);
	setMinimumHeight(80);
	setMouseTracking(true);
}

QString PlainTextView::toPlainText() const
{
	// TODO
	return m_data;
}


void PlainTextView::setData(const QByteArray &data)
{
	clear();
	insertData(data);
}

void PlainTextView::insertData(const QByteArray &data)
{
	int start = m_data.size();
	m_data.append(data);

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
	painter.setFont(m_font);

	const int rowHeight = m_fm.height();
	const int padding = m_fm.averageCharWidth();

	QRect rect = event->rect();
	const int startRow = qBound(0, (rect.y() - m_fm.ascent() - padding) / rowHeight - 2, m_rows.size() - 1);
	const int endRow = qBound(0, (rect.y() + rect.height() - m_fm.ascent() - padding) / rowHeight + 2, m_rows.size());

	static QColor colors[3] = {
		textColor,
		standardHexCodeColor,
		nonStandardHexCodeColor,
	};

	int y = m_fm.ascent() + padding;
	for (int rowIndex = startRow; rowIndex < endRow; ++rowIndex) {
		const Row &row = m_rows[rowIndex];
		int x = padding;
		for (const Element &element : row.elements) {
			painter.setPen(colors[element.type]);
			painter.drawText(x, y, element.str);
			x += m_fm.horizontalAdvance(element.str) + 1;
		}

		y += rowHeight;
	}
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
