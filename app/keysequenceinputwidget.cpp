#include "keysequenceinputwidget.h"
#include <QEvent>
#include <QKeyEvent>
#include <QFontMetrics>

inline int textWidth(const QFontMetrics &fm, const QString &text)
{
#if QT_VERSION >= 0x050B00
	   return fm.horizontalAdvance(text);
#else
	   return fm.width(text);
#endif
}

KeySequenceInputWidget::KeySequenceInputWidget(QKeySequence sequence, QWidget *parent)
	: QToolButton(parent)
	, m_sequence(sequence)
	, m_inputting(false)
{
	setText(sequence.toString(QKeySequence::NativeText));
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	setMinimumWidth(textWidth(QFontMetrics(font()), "   Enter shortcut...   "));

	connect(this, &QToolButton::clicked, this, &KeySequenceInputWidget::onClicked);
}

QKeySequence KeySequenceInputWidget::sequence() const
{
	return m_sequence;
}

void KeySequenceInputWidget::stopInputting()
{
	setText(m_sequence.toString(QKeySequence::NativeText));
	setDown(false);
	m_inputting = false;
}

void KeySequenceInputWidget::onClicked()
{
	setText("Enter shortcut...");
	setDown(true);
	m_inputting = true;
}

void KeySequenceInputWidget::focusOutEvent(QFocusEvent *)
{
	if (m_inputting)
		stopInputting();
}

void KeySequenceInputWidget::keyPressEvent(QKeyEvent *event)
{
	if (m_inputting) {
		int key = event->key();
		if (key == Qt::Key_Escape) {
			stopInputting();
		} else {
			if (!event->text().isEmpty()) {
				m_sequence = key + int(event->modifiers());
				stopInputting();
				emit sequenceChanged(m_sequence);
			}
		}
	}
}
