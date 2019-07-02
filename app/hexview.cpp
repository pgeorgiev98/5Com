#include "hexview.h"
#include <QPainter>
#include <QFontDatabase>
#include <QFont>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QtGlobal>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>

static QColor backgroundColor("#ffffff");
static QColor textColor("#000000");
static QColor hoverTextColor("#ff0000");
static QColor selectedColor("#0000ff");
static QColor selectedTextColor("#000000");

#define cellX(x) (x * m_cellSize + (x + 1 + (x > 7)) * m_cellPadding)
#define textX(x) (cellX(17) + x * m_characterWidth)

static const char hexTable[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
								  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_font(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont))
	, m_fontMetrics(m_font)
#if QT_VERSION >= 0x050B00
	, m_characterWidth(m_fontMetrics.horizontalAdvance(' '))
#else
	, m_characterWidth(m_fontMetrics.width(' '))
#endif
	, m_cellSize(m_fontMetrics.height())
	, m_cellPadding(m_characterWidth)
	, m_hoveredIndex(-1)
	, m_selectionStart(-1)
	, m_selectionEnd(-1)
	, m_selection(Selection::None)
	, m_selecting(false)
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
	setFixedWidth(textX(16) + m_cellPadding);
	setMinimumHeight(80);
	setMouseTracking(true);
}

QString HexView::toPlainText() const
{
	return QString();
}

void HexView::clear()
{
	m_data.clear();

	int rows = m_data.size() / 16 + (m_data.size() % 16 > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);
}

void HexView::setData(const QByteArray &data)
{
	m_data.clear();
	insertData(data);
}

void HexView::insertData(const QByteArray &data)
{
	m_data.append(data);

	int rows = m_data.size() / 16 + (m_data.size() % 16 > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);

	repaint();
}

void HexView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	painter.setFont(m_font);

	const int startY = qMax(0, event->rect().y() / (m_cellSize + m_cellPadding) - 1);
	const int endY = qMin(m_data.size(), event->rect().bottom() / (m_cellSize + m_cellPadding) + 1);

	int selectionStart = -1;
	int selectionEnd = -1;
	if (m_selection == Selection::Cells || m_selection == Selection::Text) {
		selectionStart = qMin(m_selectionStart, m_selectionEnd);
		selectionEnd = qMax(m_selectionStart, m_selectionEnd) + 1;
	} else if (m_selection == Selection::CellRows || m_selection == Selection::TextRows) {
		if (m_selectionStart < m_selectionEnd) {
			selectionStart = m_selectionStart;
			selectionEnd = m_selectionEnd + 1;
		} else {
			selectionStart = m_selectionEnd - 15;
			selectionEnd = m_selectionStart + 16;
		}
	}

	QString cellText = "FF";
	QString ch = "a";
	int i = startY * 16;
	for (int y = startY; i < m_data.size() && y < endY; ++y) {
		for (int x = 0; i < m_data.size() && x < 16; ++x, ++i) {
			unsigned char byte = static_cast<unsigned char>(m_data[i]);
			cellText[0] = hexTable[(byte >> 4) & 0xF];
			cellText[1] = hexTable[(byte >> 0) & 0xF];
			ch[0] = (byte >= 32 && byte <= 126) ? char(byte) : '.';

			int yCoord = y * (m_cellSize + m_cellPadding) + m_cellSize;

			QPoint cellCoord;
			cellCoord.setX(cellX(x));
			cellCoord.setY(yCoord);

			QPoint textCoord;
			textCoord.setX(textX(x));
			textCoord.setY(yCoord);

			bool inSelection = (i >= selectionStart && i < selectionEnd);

			if (i == m_hoveredIndex || inSelection) {
				if (inSelection)
					painter.setBrush(selectedColor);
				else
					painter.setBrush(backgroundColor);
				painter.setPen(selectedColor);

				painter.drawRect(cellCoord.x() - m_cellPadding / 2,
								 cellCoord.y() - m_fontMetrics.ascent() - m_cellPadding / 2,
								 m_characterWidth * 2 + m_cellPadding,
								 m_cellSize + m_cellPadding);

				painter.drawRect(textCoord.x(),
								 textCoord.y() - m_fontMetrics.ascent(),
								 m_characterWidth,
								 m_fontMetrics.height());
			}

			if (m_hoveredIndex == i)
				painter.setPen(hoverTextColor);
			else
				painter.setPen(textColor);

			painter.drawText(cellCoord, cellText);
			painter.drawText(textCoord, ch);
		}
	}
}

void HexView::mouseMoveEvent(QMouseEvent *event)
{
	int hoverCellIndex = getHoverCell(event->pos());
	int hoverTextIndex = getHoverText(event->pos());

	int newIndex = qMax(hoverCellIndex, hoverTextIndex);
	if (newIndex != m_hoveredIndex) {
		m_hoveredIndex = newIndex;

		if (m_hoveredIndex != -1) {
			if (m_selecting) {
				if (newIndex == hoverCellIndex && (m_selection == Selection::Cells || m_selection == Selection::CellRows)) {
					if (m_selection == Selection::Cells)
						m_selectionEnd = m_hoveredIndex;
					else if (m_selection == Selection::CellRows)
						m_selectionEnd = 16 * (m_hoveredIndex / 16) + 15;
			} else if (newIndex == hoverTextIndex && (m_selection == Selection::Text || m_selection == Selection::TextRows)) {
					if (m_selection == Selection::Text)
						m_selectionEnd = m_hoveredIndex;
					else if (m_selection == Selection::TextRows)
						m_selectionEnd = 16 * (m_hoveredIndex / 16) + 15;
				}
			}
		}

		repaint();
	}
}

void HexView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		int selectionStart = -1;
		int selectionEnd = -1;
		if (m_selection == Selection::Cells || m_selection == Selection::Text) {
			selectionStart = qMin(m_selectionStart, m_selectionEnd);
			selectionEnd = qMax(m_selectionStart, m_selectionEnd) + 1;
		} else if (m_selection == Selection::CellRows || m_selection == Selection::TextRows) {
			if (m_selectionStart < m_selectionEnd) {
				selectionStart = m_selectionStart;
				selectionEnd = m_selectionEnd + 1;
			} else {
				selectionStart = m_selectionEnd - 15;
				selectionEnd = m_selectionStart + 16;
			}
		}
		if (selectionEnd > m_data.size())
			selectionEnd = m_data.size();

		QMenu menu(this);
		QAction copyTextAction("Copy text");
		QAction copyHexAction("Copy hex");
		menu.addAction(&copyTextAction);
		menu.addAction(&copyHexAction);
		menu.popup(event->globalPos());
		bool copyEnabled = (selectionStart < selectionEnd);
		copyTextAction.setEnabled(copyEnabled);
		copyHexAction.setEnabled(copyEnabled);
		QAction *a = menu.exec();

		QClipboard *clipboard = QGuiApplication::clipboard();

		if (a == &copyTextAction) {
			QString s;
			for (int i = selectionStart; i < selectionEnd; ++i) {
				char b = m_data[i];
				s.append((b >= 32 && b <= 126) ? b : '.');
			}
			clipboard->setText(s);
		} else if (a == &copyHexAction) {
			QString cell = "00 ";
			QString s;
			for (int i = selectionStart; i < selectionEnd; ++i) {
				unsigned char byte = static_cast<unsigned char>(m_data[i]);
				cell[0] = hexTable[(byte >> 4) & 0xF];
				cell[1] = hexTable[(byte >> 0) & 0xF];
				s.append(cell);
			}
			s.remove(s.size() - 1, 1);
			clipboard->setText(s);
		}
		return;
	}
	int hoverCellIndex = getHoverCell(event->pos());
	int hoverTextIndex = getHoverText(event->pos());
	int newIndex = qMax(hoverCellIndex, hoverTextIndex);
	m_selectionStart = newIndex;
	m_selectionEnd = m_selectionStart;
	if (newIndex == -1)
		m_selection = Selection::None;
	else if (newIndex == hoverCellIndex)
		m_selection = Selection::Cells;
	else if (newIndex == hoverTextIndex)
		m_selection = Selection::Text;
	m_selecting = (newIndex != -1);
	repaint();
}

void HexView::mouseReleaseEvent(QMouseEvent *)
{
	m_selecting = false;
}

void HexView::mouseDoubleClickEvent(QMouseEvent *event)
{
	int hoverCellIndex = getHoverCell(event->pos());
	int hoverTextIndex = getHoverText(event->pos());
	int newIndex = qMax(hoverCellIndex, hoverTextIndex);
	if (newIndex != -1) {
		m_selectionStart = 16 * (newIndex / 16);
		m_selectionEnd = m_selectionStart + 15;
		if (newIndex == hoverCellIndex)
			m_selection = Selection::CellRows;
		else
			m_selection = Selection::TextRows;
		m_selecting = true;
		repaint();
	}
}

void HexView::leaveEvent(QEvent *)
{
	m_hoveredIndex = -1;
	repaint();
}

int HexView::getHoverCell(const QPoint &mousePos) const
{
	int x = mousePos.x();
	int y = mousePos.y();

	if (x > 9 * m_cellPadding + 8 * m_cellSize)
		x -= m_cellPadding;

	x -= m_cellPadding / 2;
	y -= m_cellPadding / 2;

	int xi = -1;
	int yi = -1;

	if (x >= 0 && x < 16 * (m_cellPadding + m_cellSize))
		xi = x / (m_cellPadding + m_cellSize);

	if (y >= 0 && y < m_data.size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return xi + 16 * yi;

	return -1;
}

int HexView::getHoverText(const QPoint &mousePos) const
{
	int x = mousePos.x();
	int y = mousePos.y();

	x -= cellX(17);

	int xi = -1, yi = -2;
	if (x >= 0 && x < m_characterWidth * 16)
		xi = x / m_characterWidth;
	if (y >= 0 && y < m_data.size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return xi + 16 * yi;

	return -1;
}
