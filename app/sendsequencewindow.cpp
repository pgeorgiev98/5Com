#include "sendsequencewindow.h"
#include "serialport.h"
#include "line.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QMessageBox>
#include <QCloseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QComboBox>

SendSequenceWindow::SendSequenceWindow(SerialPort *port, QWidget *parent)
	: QDialog(parent)
	, m_port(port)
	, m_operationsLayout(new QGridLayout)
	, m_operationsScrollArea(new QScrollArea)
	, m_sendIndefinitely(new QCheckBox("Send indefinitely"))
	, m_sequencesCount(new QSpinBox)
	, m_sendButton(new QPushButton("Send"))
	, m_currentOperation(-1)
	, m_timer(new QTimer(this))
{
	m_timer->setSingleShot(true);
	m_operationsLayout->setAlignment(Qt::AlignTop);
	m_sendIndefinitely->setChecked(false);
	m_sequencesCount->setRange(1, INT_MAX);
	m_sequencesCount->setValue(1);

	QToolButton *addNewButton = new QToolButton;
	QToolButton *clearOperationsButton = new QToolButton;
	addNewButton->setText("+");
	clearOperationsButton->setText("Clear");

	setMinimumWidth(400);
	setMinimumHeight(400);
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	{
		QHBoxLayout *hbox = new QHBoxLayout;
		addNewButton->setText("+");
		clearOperationsButton->setText("Clear");
		hbox->addWidget(clearOperationsButton);
		hbox->addStretch(1);
		hbox->addWidget(addNewButton);

		QVBoxLayout *vbox = new QVBoxLayout;

		vbox->addLayout(m_operationsLayout, 1);
		vbox->addLayout(hbox);

		QWidget *operationsWidget = new QWidget;
		operationsWidget->setLayout(vbox);
		m_operationsScrollArea->setWidget(operationsWidget);
		m_operationsScrollArea->setWidgetResizable(true);
	}

	layout->addWidget(new QLabel("Send sequence"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Horizontal));
	layout->addWidget(m_operationsScrollArea);
	layout->addSpacing(16);
	{
		QFrame *sendSettings = new QFrame;
		sendSettings->setFrameShape(QFrame::Shape::Panel);
		QVBoxLayout *l = new QVBoxLayout;
		sendSettings->setLayout(l);

		l->addWidget(m_sendIndefinitely);
		{
			QHBoxLayout *hbox = new QHBoxLayout;
			hbox->addWidget(new QLabel("Number of sequences to send: "));
			hbox->addWidget(m_sequencesCount);
			l->addLayout(hbox);
		}

		layout->addWidget(sendSettings);
	}

	{
		QHBoxLayout *hbox = new QHBoxLayout;
		hbox->addStretch(1);
		hbox->addWidget(m_sendButton);
		hbox->addStretch(1);
		layout->addLayout(hbox);
	}

	connect(m_sendButton, &QPushButton::clicked, this, &SendSequenceWindow::onSendClicked);
	connect(addNewButton, &QPushButton::clicked, [this, addNewButton]() {
		QMenu menu(addNewButton);
		QAction sendAction("Send");
		QAction waitAction("Wait");
		QAction changeDtr("Change DTR");
		QAction changeRts("Change RTS");
		menu.addAction(&sendAction);
		menu.addAction(&waitAction);
		menu.addAction(&changeDtr);
		menu.addAction(&changeRts);
		QPoint pos = addNewButton->pos();
		pos.setX(pos.x() + addNewButton->width());
		menu.popup(m_operationsScrollArea->viewport()->mapToGlobal(pos));
		QAction *action = menu.exec();
		if (action == &sendAction)
			addOperation(OperationType::Send);
		else if (action == &waitAction)
			addOperation(OperationType::Wait);
		else if (action == &changeDtr)
			addOperation(OperationType::ChangeDTR);
		else if (action == &changeRts)
			addOperation(OperationType::ChangeRTS);
	});
	connect(clearOperationsButton, &QPushButton::clicked, this, &SendSequenceWindow::clearOperations);
	connect(m_sendIndefinitely, &QCheckBox::stateChanged, m_sequencesCount, &QWidget::setDisabled);
	m_sequencesCount->setDisabled(m_sendIndefinitely->isChecked());
	m_sendButton->setEnabled(false);
	connect(this, &SendSequenceWindow::operationsCountChanged, [this](int count) {
		m_sendButton->setEnabled(count > 0);
	});
	connect(m_timer, &QTimer::timeout, this, &SendSequenceWindow::executeNextOperation);
	connect(m_port, &SerialPort::errorOccurred, this, &SendSequenceWindow::cancelSequence);
	connect(m_port, &SerialPort::closed, this, &SendSequenceWindow::cancelSequence);
}

void SendSequenceWindow::onSendClicked()
{
	if (m_currentOperation == -1) {
		if (!m_port->isOpen())
			if (!m_port->open())
				return;

		m_currentOperation = 0;
		m_sendButton->setText("Cancel");

		executeNextOperation();
	} else {
		cancelSequence();
	}
}

void SendSequenceWindow::addOperation(SendSequenceWindow::OperationType type)
{
	addOperation(type, m_operations.size());
	// Scroll to bottom; this looks pretty badly hacked
	QTimer::singleShot(1, [this]() {
		auto sb = m_operationsScrollArea->verticalScrollBar();
		sb->setValue(sb->maximum());
	});
}

void SendSequenceWindow::addOperation(SendSequenceWindow::OperationType type, int row)
{
	for (int i = m_operations.size() - 1; i >= row; --i) {
		m_operationsLayout->removeWidget(m_operations[i].label);
		m_operationsLayout->removeWidget(m_operations[i].input);
		m_operationsLayout->removeWidget(m_operations[i].actionButton);
		m_operationsLayout->addWidget(m_operations[i].label, i + 1, 0);
		m_operationsLayout->addWidget(m_operations[i].input, i + 1, 1);
		m_operationsLayout->addWidget(m_operations[i].actionButton, i + 1, 3);
	}

	Operation op;
	op.type = type;
	if (type == OperationType::Send) {
		m_operationsLayout->addWidget(op.label = new QLabel("Send: "), row, 0);
		QLineEdit *input = new QLineEdit;
		m_operationsLayout->addWidget(input, row, 1);
		input->setFocus();
		op.input = input;
	} else if (type == OperationType::Wait) {
		m_operationsLayout->addWidget(op.label = new QLabel("Wait: "), row, 0);
		QSpinBox *input = new QSpinBox;
		input->setRange(0, INT_MAX);
		input->setSuffix("ms");
		m_operationsLayout->addWidget(input, row, 1, Qt::AlignLeft);
		input->setFocus();
		input->selectAll();
		op.input = input;
	} else if (type == OperationType::ChangeDTR) {
		m_operationsLayout->addWidget(op.label = new QLabel("Change DTR: "), row, 0);
		QComboBox *input = new QComboBox;
		input->addItems({"True", "False", "Toggle"});
		m_operationsLayout->addWidget(input, row, 1, Qt::AlignLeft);
		input->setFocus();
		op.input = input;
	} else if (type == OperationType::ChangeRTS) {
		m_operationsLayout->addWidget(op.label = new QLabel("Change RTS: "), row, 0);
		QComboBox *input = new QComboBox;
		input->addItems({"True", "False", "Toggle"});
		m_operationsLayout->addWidget(input, row, 1, Qt::AlignLeft);
		input->setFocus();
		op.input = input;
	}
	QToolButton *actionButton = new QToolButton;
	actionButton->setText("...");
	m_operationsLayout->addWidget(actionButton, row, 3);
	op.actionButton = actionButton;
	connect(actionButton, &QToolButton::clicked, this, &SendSequenceWindow::onActionButtonClicked);
	m_operations.insert(row, op);

	if (m_currentOperation != -1 && row <= m_currentOperation)
		++m_currentOperation;

	emit operationsCountChanged(m_operations.count());
}

void SendSequenceWindow::clearOperations()
{
	cancelSequence();
	while (!m_operations.isEmpty())
		removeOperation(m_operations.size() - 1);
	QTimer::singleShot(0, this, &QWidget::adjustSize);
}

void SendSequenceWindow::removeOperation(int i, bool adjustSize)
{
	m_operationsLayout->removeWidget(m_operations[i].label);
	m_operationsLayout->removeWidget(m_operations[i].input);
	m_operationsLayout->removeWidget(m_operations[i].actionButton);
	m_operations[i].label->deleteLater();
	m_operations[i].input->deleteLater();
	m_operations[i].actionButton->deleteLater();

	for (int j = i; j < m_operations.size() - 1; ++j) {
		m_operationsLayout->removeWidget(m_operations[j + 1].label);
		m_operationsLayout->removeWidget(m_operations[j + 1].input);
		m_operationsLayout->removeWidget(m_operations[j + 1].actionButton);
		m_operationsLayout->addWidget(m_operations[j + 1].label, j, 0);
		m_operationsLayout->addWidget(m_operations[j + 1].input, j, 1);
		m_operationsLayout->addWidget(m_operations[j + 1].actionButton, j, 3);
	}

	m_operations.removeAt(i);

	if (m_currentOperation != -1 && i <= m_currentOperation)
		--m_currentOperation;

	emit operationsCountChanged(m_operations.size());

	if (adjustSize)
		QTimer::singleShot(0, this, &QWidget::adjustSize);
}

void SendSequenceWindow::executeNextOperation()
{
	if (m_currentOperation >= m_operations.size()) {
		if (m_sendIndefinitely->isChecked()) {
			m_currentOperation = 0;
		} else if (m_sequencesCount->value() > 1) {
			m_sequencesCount->setValue(m_sequencesCount->value() - 1);
			m_currentOperation = 0;
		} else {
			cancelSequence();
			return;
		}
	}

	if (m_currentOperation == -1)
		return;

	if (m_currentOperation == 0)
		for (int i = 1; i < m_operations.size(); ++i)
			m_operationsLayout->itemAtPosition(i, 0)->widget()->setStyleSheet("");

	int i = m_currentOperation++;

	if (i >= 1)
		m_operationsLayout->itemAtPosition(i - 1, 0)->widget()->setStyleSheet("color: green");

	m_operationsLayout->itemAtPosition(i, 0)->widget()->setStyleSheet("color: blue");

	Operation op = m_operations[i];
	if (op.type == OperationType::Send) {
		QLineEdit *input = static_cast<QLineEdit *>(m_operationsLayout->itemAtPosition(i, 1)->widget());
		m_port->writeFormattedData(input->text());
	} else if (op.type == OperationType::Wait) {
		QSpinBox *input = static_cast<QSpinBox *>(m_operationsLayout->itemAtPosition(i, 1)->widget());
		m_timer->start(input->value());
	} else if (op.type == OperationType::ChangeDTR) {
		QComboBox *input = static_cast<QComboBox *>(m_operationsLayout->itemAtPosition(i, 1)->widget());
		if (input->currentIndex() == 0)
			m_port->setDataTerminalReady(true);
		else if (input->currentIndex() == 1)
			m_port->setDataTerminalReady(false);
		else
			m_port->setDataTerminalReady(!m_port->isDataTerminalReady());
	} else if (op.type == OperationType::ChangeRTS) {
		QComboBox *input = static_cast<QComboBox *>(m_operationsLayout->itemAtPosition(i, 1)->widget());
		if (input->currentIndex() == 0)
			m_port->setRequestToSend(true);
		else if (input->currentIndex() == 1)
			m_port->setRequestToSend(false);
		else
			m_port->setRequestToSend(!m_port->isRequestToSend());
	}

	if (op.type != OperationType::Wait)
		m_timer->start(0);
}

void SendSequenceWindow::reject()
{
	if (m_currentOperation != -1) {
		auto b = QMessageBox::question(this, "Sequence",
									   "Sequence is still running. Cancel it?");
		if (b != QMessageBox::StandardButton::Yes)
			return;

		cancelSequence();
	}
	QDialog::reject();
}

void SendSequenceWindow::onActionButtonClicked()
{
	int i;
	QWidget *s = reinterpret_cast<QWidget *>(sender());
	for (i = 0; i < m_operations.size(); ++i)
		if (m_operations[i].actionButton == s)
			break;

	QMenu menu(s);
	QAction removeAction("Remove");
	QMenu addBefore("Add before this");
	QMenu addAfter("Add after this");
	QAction addSendBefore("Send");
	QAction addWaitBefore("Wait");
	QAction addChangeDtrBefore("Change DTR");
	QAction addChangeRtsBefore("Change RTS");
	QAction addSendAfter("Send");
	QAction addWaitAfter("Wait");
	QAction addChangeDtrAfter("Change DTR");
	QAction addChangeRtsAfter("Change RTS");

	addBefore.addAction(&addSendBefore);
	addBefore.addAction(&addWaitBefore);
	addBefore.addAction(&addChangeDtrBefore);
	addBefore.addAction(&addChangeRtsBefore);
	addAfter.addAction(&addSendAfter);
	addAfter.addAction(&addWaitAfter);
	addAfter.addAction(&addChangeDtrAfter);
	addAfter.addAction(&addChangeRtsAfter);

	menu.addAction(&removeAction);
	menu.addMenu(&addBefore);
	menu.addMenu(&addAfter);

	QPoint pos = s->pos();
	pos.setX(pos.x() + s->width());
	pos.setY(pos.y() - m_operationsScrollArea->verticalScrollBar()->value());
	menu.popup(QWidget::mapToGlobal(pos));
	QAction *action = menu.exec();
	if (action == &removeAction)
		removeOperation(i, true);
	else if (action == &addSendBefore)
		addOperation(OperationType::Send, i);
	else if (action == &addWaitBefore)
		addOperation(OperationType::Wait, i);
	else if (action == &addChangeDtrBefore)
		addOperation(OperationType::ChangeDTR, i);
	else if (action == &addChangeRtsBefore)
		addOperation(OperationType::ChangeRTS, i);
	else if (action == &addSendAfter)
		addOperation(OperationType::Send, i + 1);
	else if (action == &addWaitAfter)
		addOperation(OperationType::Wait, i + 1);
	else if (action == &addChangeDtrAfter)
		addOperation(OperationType::ChangeDTR, i + 1);
	else if (action == &addChangeRtsAfter)
		addOperation(OperationType::ChangeRTS, i + 1);
}

void SendSequenceWindow::cancelSequence()
{
	m_timer->stop();
	m_currentOperation = -1;
	m_sendButton->setText("Send");
}
