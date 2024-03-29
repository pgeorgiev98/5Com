#include "hexview.h"
#include "common.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QtGlobal>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>

#define cellX(x) ((x) * m_cellSize + ((x) + 1 + ((x) / 8)) * m_cellPadding)
#define textX(x) (cellX(m_bytesPerLine + 1) + x * (m_characterWidth + 5))

static const char hexTable[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
								  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_font(getFixedFont())
	, m_fontMetrics(m_font)
#if QT_VERSION >= 0x050B00
	, m_characterWidth(m_fontMetrics.horizontalAdvance(' '))
#else
	, m_characterWidth(m_fontMetrics.width(' '))
#endif
	, m_cellSize(m_fontMetrics.height())
	, m_cellPadding(m_characterWidth)
	, m_bytesPerLine(16)
	, m_hoveredIndex(-1)
	, m_selectionStart(-1)
	, m_selectionEnd(-1)
	, m_selection(Selection::None)
	, m_selecting(false)
{
	QPalette pal = palette();
	m_backgroundColor = pal.base().color();
	m_textColor = pal.text().color();
	m_hoverTextColor = pal.link().color();
	m_selectedColor = pal.highlight().color();
	m_selectedTextColor = pal.highlightedText().color();

	pal.setColor(QPalette::Window, m_backgroundColor);
	setAutoFillBackground(true);
	setPalette(pal);
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);
	setMinimumHeight(80);
	setMouseTracking(true);
}

QString HexView::toPlainText() const
{
	QString s;
	if (m_data.isEmpty())
		return s;

	QString byte = "FF ";
	int i = 0;
	while (i < m_data.size()) {
		for (int x = 0; i < m_data.size() && x < m_bytesPerLine; ++x, ++i) {
			unsigned char b = static_cast<unsigned char>(m_data[i]);
			byte[0] = hexTable[(b >> 4) & 0xF];
			byte[1] = hexTable[(b >> 0) & 0xF];
			s.append(byte);
		}
		s[s.size() - 1] = '\n';
	}
	return s;
}

QPoint HexView::getByteCoordinates(int index) const
{
	QPoint p;
	p.setX(cellX(index % 16));
	p.setY((index / 16) * (m_cellSize + m_cellPadding) + m_cellSize - m_fontMetrics.ascent());
	return p;
}

std::optional<ByteSelection> HexView::selection() const
{
	if (m_selection == Selection::None)
		return std::optional<ByteSelection>();
	int s = m_selectionStart, e = m_selectionEnd;
	if (s > e)
		qSwap(s, e);
	return ByteSelection(s, e - s + 1);
}

void HexView::clear()
{
	m_data.clear();
	m_selection = Selection::None;
	m_selectionStart = m_selectionEnd = -1;

	int rows = m_data.size() / m_bytesPerLine + (m_data.size() % m_bytesPerLine > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);

	repaint();
}

void HexView::setData(const QByteArray &data)
{
	m_data.clear();
	m_selection = Selection::None;
	m_selectionStart = m_selectionEnd = -1;
	insertData(data);
}

void HexView::insertData(const QByteArray &data)
{
	m_data.append(data);

	int rows = m_data.size() / m_bytesPerLine + (m_data.size() % m_bytesPerLine > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);

	repaint();
}

void HexView::trimData(int visibleBytesCount)
{
	int bytesToRemove = m_data.size() - visibleBytesCount;
	m_data.remove(0, bytesToRemove);

	m_selectionStart = qMax(m_selectionStart - bytesToRemove, 0);
	m_selectionEnd = qMax(m_selectionEnd - bytesToRemove, 0);
	if (m_selectionStart == m_selectionEnd)
		m_selection = Selection::None;

	int oldHeight = height();

	int rows = m_data.size() / m_bytesPerLine + (m_data.size() % m_bytesPerLine > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);

	emit mustScrollUp(oldHeight - height());

	repaint();
}

void HexView::setBytesPerLine(int bytesPerLine)
{
	m_bytesPerLine = bytesPerLine;

	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);
	int rows = m_data.size() / m_bytesPerLine + (m_data.size() % m_bytesPerLine > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);
	repaint();
}

void HexView::highlight(ByteSelection selection)
{
	m_selection = Selection::Cells;
	m_selectionStart = selection.begin;
	m_selectionEnd = selection.begin + selection.count - 1;

	repaint();
}

void HexView::selectNone()
{
	m_selectionStart = 0;
	m_selectionEnd = 0;
	m_selection = Selection::None;
	m_selecting = false;

	repaint();
}

void HexView::setFont(QFont font)
{
	m_font = font;
	m_fontMetrics = QFontMetrics(m_font);
	m_characterWidth = m_fontMetrics.averageCharWidth();
	m_cellSize = m_fontMetrics.height();
	m_cellPadding = m_characterWidth;
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);

	int rows = m_data.size() / m_bytesPerLine + (m_data.size() % m_bytesPerLine > 0);
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
			selectionStart = m_selectionEnd - m_bytesPerLine + 1;
			selectionEnd = m_selectionStart + m_bytesPerLine;
		}
	}

	QString cellText = "FF";
	QString ch = "a";
	int i = startY * m_bytesPerLine;
	for (int y = startY; i < m_data.size() && y < endY; ++y) {
		for (int x = 0; i < m_data.size() && x < m_bytesPerLine; ++x, ++i) {
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
					painter.setBrush(m_selectedColor);
				else
					painter.setBrush(m_backgroundColor);
				if (i == m_hoveredIndex)
					painter.setPen(m_hoverTextColor);
				else
					painter.setPen(m_selectedColor);

				painter.drawRect(cellCoord.x() - m_cellPadding / 2,
								 cellCoord.y() - m_fontMetrics.ascent() - m_cellPadding / 2,
								 m_characterWidth * 2 + m_cellPadding,
								 m_cellSize + m_cellPadding - 1);

				painter.drawRect(textCoord.x() - 2,
								 textCoord.y() - m_fontMetrics.ascent() - 2,
								 m_characterWidth + 4,
								 m_fontMetrics.height() + 4);
			}

			if (m_hoveredIndex == i)
				painter.setPen(m_hoverTextColor);
			else
				painter.setPen(m_textColor);

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
						m_selectionEnd = m_bytesPerLine * (m_hoveredIndex / m_bytesPerLine) + m_bytesPerLine - 1;
			} else if (newIndex == hoverTextIndex && (m_selection == Selection::Text || m_selection == Selection::TextRows)) {
					if (m_selection == Selection::Text)
						m_selectionEnd = m_hoveredIndex;
					else if (m_selection == Selection::TextRows)
						m_selectionEnd = m_bytesPerLine * (m_hoveredIndex / m_bytesPerLine) + m_bytesPerLine - 1;
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
				selectionStart = m_selectionEnd - m_bytesPerLine + 1;
				selectionEnd = m_selectionStart + m_bytesPerLine;
			}
		}
		if (selectionEnd > m_data.size())
			selectionEnd = m_data.size();

		QMenu menu(this);
		QAction copyTextAction("Copy text");
		QAction copyHexAction("Copy hex");
		QAction selectAllAction("Select All");
		QAction selectNoneAction("Select None");
		QAction highlightInTextViewAction("Highlight in Text View");

		menu.addAction(&copyTextAction);
		menu.addAction(&copyHexAction);
		menu.addSeparator();
		menu.addAction(&selectAllAction);
		menu.addAction(&selectNoneAction);
		menu.addSeparator();
		menu.addAction(&highlightInTextViewAction);

		menu.popup(event->globalPos());

		bool hasSelection = (selectionStart < selectionEnd);
		copyTextAction.setEnabled(hasSelection);
		copyHexAction.setEnabled(hasSelection);
		selectAllAction.setEnabled(!m_data.isEmpty());
		selectNoneAction.setEnabled(hasSelection);
		highlightInTextViewAction.setEnabled(hasSelection);

		QAction *a = menu.exec();

		if (a == &copyTextAction) {
			QString s;
			for (int i = selectionStart; i < selectionEnd; ++i) {
				char b = m_data[i];
				s.append((b >= 32 && b <= 126) ? b : '.');
			}
			QClipboard *clipboard = QGuiApplication::clipboard();
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
			QClipboard *clipboard = QGuiApplication::clipboard();
			clipboard->setText(s);
		} else if (a == &selectAllAction) {
			m_selectionStart = 0;
			m_selectionEnd = m_data.size() - 1;
			m_selection = Selection::Cells;
			m_selecting = false;
			repaint();
		} else if (a == &selectNoneAction) {
			selectNone();
		} else if (a == &highlightInTextViewAction) {
			emit highlightInTextView(ByteSelection(m_selectionStart, m_selectionEnd - m_selectionStart + 1));
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
		m_selectionStart = m_bytesPerLine * (newIndex / m_bytesPerLine);
		m_selectionEnd = m_selectionStart + m_bytesPerLine - 1;
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

	{
		int vx = x;
		while (vx > 8 * m_cellPadding + 8 * m_cellSize) {
			x -= m_cellPadding;
			vx -= m_cellPadding;
			vx -= 8 * m_cellPadding + 8 * m_cellSize;
		}
	}

	x -= m_cellPadding / 2;
	y -= m_cellPadding / 2;

	int xi = -1;
	int yi = -1;

	if (x >= 0 && x < m_bytesPerLine * (m_cellPadding + m_cellSize))
		xi = x / (m_cellPadding + m_cellSize);

	if (y >= 0 && y < m_data.size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return xi + m_bytesPerLine * yi;

	return -1;
}

int HexView::getHoverText(const QPoint &mousePos) const
{
	int x = mousePos.x();
	int y = mousePos.y();

	x -= cellX(m_bytesPerLine + 1);

	int xi = -1, yi = -2;
	if (x >= 0 && x < (m_characterWidth + 5) * m_bytesPerLine)
		xi = x / (m_characterWidth + 5);
	if (y >= 0 && y < m_data.size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return xi + m_bytesPerLine * yi;

	return -1;
}
