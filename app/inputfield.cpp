#include "inputfield.h"
#include "config.h"
#include <QKeyEvent>

InputField::InputField(QWidget *parent)
	: QComboBox(parent)
{
	setEditable(true);
	setDuplicatesEnabled(true);
	setInsertPolicy(QComboBox::InsertAtTop);
	setCompleter(nullptr);
}

void InputField::onInputEntered()
{
	QString input = currentText();
	if (input.isEmpty())
		return;

	// Move duplicate to the top
	if (count() == 0 || itemText(0) != input)
		insertItem(0, input);
	for (int i = count(); i > 0; --i) {
		if (itemText(i) == input) {
			removeItem(i);
			break;
		}
	}

	// Limit history length
	Config c;
	int historyLength = c.inputHistoryLength();
	while (count() > historyLength)
		removeItem(count() - 1);

	if (c.clearInputOnSend()) {
		clearEditText();
		m_searching = false;
	}
}

void InputField::clearText()
{
	m_searching = false;
	m_searchPrefix.clear();
	clearEditText();
}

void InputField::keyPressEvent(QKeyEvent *event)
{
	// Scroll through the history with Up/Down
	int key = event->key();
	if (key == Qt::Key_Up || key == Qt::Key_Down) {
		int index = -1;
		QString cur = currentText();
		for (int i = 0; i < count(); ++i) {
			if (itemText(i) == cur) {
				index = i;
				break;
			}
		}
		const QString &search = m_searching ? m_searchPrefix : cur;
		if (!m_searching) {
			m_searchPrefix = cur;
			m_searching = true;
		}

		if (key == Qt::Key_Up) {
			for (int i = index + 1; i < count(); ++i) {
				if (itemText(i).startsWith(search)) {
					setCurrentText(itemText(i));
					return;
				}
			}
		} else if (key == Qt::Key_Down) {
			int matchIndex = -1;
			for (int i = index - 1; i >= 0; --i) {
				if (itemText(i).startsWith(search)) {
					matchIndex = i;
					setCurrentText(itemText(i));
					return;
				}
			}
			if (matchIndex == -1)
				setCurrentText(m_searchPrefix);
		}
	} else {
		if (key != Qt::Key_Enter)
			m_searching = false;
		QComboBox::keyPressEvent(event);
	}
}
