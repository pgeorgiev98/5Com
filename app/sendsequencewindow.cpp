#include "sendsequencewindow.h"
#include "serialport.h"
#include "line.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QPushButton>
#include <QToolButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QMessageBox>
#include <QCloseEvent>

SendSequenceWindow::SendSequenceWindow(SerialPort *port, QWidget *parent)
	: QDialog(parent)
	, m_port(port)
	, m_operationsLayout(new QGridLayout)
	, m_addnewButton(new QToolButton)
	, m_clearOperationsButton(new QToolButton)
	, m_currentOperation(-1)
	, m_timer(new QTimer(this))
{
	m_addnewButton->setText("+");
	m_clearOperationsButton->setText("Clear");
	setMinimumWidth(400);
	QPushButton *sendButton = new QPushButton("Send");
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("Send sequence"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Horizontal));
	layout->addLayout(m_operationsLayout);

	{
		QHBoxLayout *hbox = new QHBoxLayout;
		hbox->addStretch(1);
		hbox->addWidget(sendButton);
		hbox->addStretch(1);
		layout->addLayout(hbox);
	}

	m_operationsLayout->addWidget(m_addnewButton, 0, 3, Qt::AlignRight);
	m_operationsLayout->addWidget(m_clearOperationsButton, 0, 0, Qt::AlignLeft);

	connect(sendButton, &QPushButton::clicked, this, &SendSequenceWindow::onSendClicked);
	connect(m_addnewButton, &QPushButton::clicked, [this]() {
		QMenu menu(m_addnewButton);
		QAction sendAction("Send");
		QAction waitAction("Wait");
		menu.addAction(&sendAction);
		menu.addAction(&waitAction);
		QPoint pos = m_addnewButton->pos();
		pos.setX(pos.x() + m_addnewButton->width());
		menu.popup(QWidget::mapToGlobal(pos));
		QAction *action = menu.exec();
		if (action == &sendAction)
			addOperation(OperationType::Send);
		else if (action == &waitAction)
			addOperation(OperationType::Wait);
	});
	connect(m_clearOperationsButton, &QPushButton::clicked, this, &SendSequenceWindow::clearOperations);
	sendButton->setEnabled(false);
	connect(this, &SendSequenceWindow::operationsCountChanged, [sendButton](int count) {
		sendButton->setEnabled(count > 0);
	});
	connect(m_timer, &QTimer::timeout, this, &SendSequenceWindow::executeNextOperation);
}

void SendSequenceWindow::onSendClicked()
{
	if (!m_port->isOpen())
		if (!m_port->open())
			return;

	for (int i = 0; i < m_operations.size(); ++i)
		m_operationsLayout->itemAtPosition(i, 0)->widget()->setStyleSheet("");

	m_currentOperation = 0;
	executeNextOperation();
}

void SendSequenceWindow::addOperation(SendSequenceWindow::OperationType type)
{
	m_operationsLayout->removeWidget(m_addnewButton);
	m_operationsLayout->removeWidget(m_clearOperationsButton);
	m_operationsLayout->addWidget(m_addnewButton, m_operations.size() + 1, 3, Qt::AlignRight);
	m_operationsLayout->addWidget(m_clearOperationsButton, m_operations.size() + 1, 0, Qt::AlignLeft);
	int row = m_operations.size();
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
	}
	m_operations.append(op);

	emit operationsCountChanged(m_operations.count());
}

void SendSequenceWindow::clearOperations()
{
	while (!m_operations.isEmpty())
		removeOperation(m_operations.size() - 1);
	QTimer::singleShot(0, this, &QWidget::adjustSize);
}

void SendSequenceWindow::removeOperation(int i, bool adjustSize)
{
	m_operationsLayout->removeWidget(m_operations[i].label);
	m_operationsLayout->removeWidget(m_operations[i].input);
	m_operations[i].label->deleteLater();
	m_operations[i].input->deleteLater();

	m_operationsLayout->removeWidget(m_addnewButton);
	m_operationsLayout->removeWidget(m_clearOperationsButton);
	m_operationsLayout->addWidget(m_addnewButton, i, 3, Qt::AlignRight);
	m_operationsLayout->addWidget(m_clearOperationsButton, i, 0, Qt::AlignLeft);

	m_operations.removeAt(i);

	emit operationsCountChanged(m_operations.size());

	if (adjustSize)
		QTimer::singleShot(0, this, &QWidget::adjustSize);
}

void SendSequenceWindow::executeNextOperation()
{
	if (m_currentOperation == m_operations.size()) {
		m_currentOperation = -1;
		return;
	}

	if (m_currentOperation == -1)
		return;

	int i = m_currentOperation++;

	m_operationsLayout->itemAtPosition(i, 0)->widget()->setStyleSheet("color: green");

	Operation op = m_operations[i];
	if (op.type == OperationType::Send) {
		QLineEdit *input = static_cast<QLineEdit *>(m_operationsLayout->itemAtPosition(i, 1)->widget());
		m_port->writeFormattedData(input->text());
		executeNextOperation();
	} else if (op.type == OperationType::Wait) {
		QSpinBox *input = static_cast<QSpinBox *>(m_operationsLayout->itemAtPosition(i, 1)->widget());
		m_timer->start(input->value());
	}
}

void SendSequenceWindow::reject()
{
	if (m_currentOperation != -1) {
		auto b = QMessageBox::question(this, "Sequence",
									   "Sequence is still running. Cancel it?");
		if (b != QMessageBox::StandardButton::Yes)
			return;

		m_currentOperation = -1;
		m_timer->stop();
	}
	QDialog::reject();
}
