#include "sendsequencewindow.h"
#include "serialport.h"
#include "line.h"
#include "config.h"
#include "common.h"

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
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

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
	, m_itemMenu(new QMenu)
{
	m_timer->setSingleShot(true);
	m_operationsLayout->setAlignment(Qt::AlignTop);
	m_operationsLayout->setColumnStretch(2, 1);
	m_sendIndefinitely->setChecked(false);
	m_sequencesCount->setRange(1, INT_MAX);
	m_sequencesCount->setValue(1);

	m_sequencesDirectory = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/sequences";
	if (!QDir(m_sequencesDirectory).exists())
		if (!QDir().mkpath(m_sequencesDirectory))
			m_sequencesDirectory = "";

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

	{
		QFrame *loadSaveOptions = new QFrame;
		loadSaveOptions->setFrameShape(QFrame::Shape::Panel);
		QHBoxLayout *l = new QHBoxLayout;
		loadSaveOptions->setLayout(l);

		QToolButton *load = new QToolButton;
		load->setText("Load");
		load->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
		QToolButton *save = new QToolButton;
		save->setText("Save");

		m_recents = new QMenu;
		load->setMenu(m_recents);
		Config c;
		QStringList recentsList;
		for (const QString &path : c.recentSequences()) {
			if (QFile(path).exists()) {
				recentsList.append(path);
				m_recents->addAction(path);
			}
		}
		if (recentsList.size() != c.recentSequences().size())
			c.setRecentSequences(recentsList);

		l->addStretch(1);
		l->addWidget(load);
		l->addStretch(1);
		l->addWidget(save);
		l->addStretch(1);

		layout->addWidget(loadSaveOptions);

		connect(save, &QPushButton::clicked, this, &SendSequenceWindow::saveSequence);
		connect(load, &QPushButton::clicked, this, static_cast<void(SendSequenceWindow::*)()>(&SendSequenceWindow::loadSequence));
		connect(m_recents, &QMenu::triggered, [this](QAction *action) {
			loadSequence(action->text());
		});
	}

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

	QMenu *menu = new QMenu(this);
	QAction *sendAction = new QAction("Send");
	QAction *waitAction = new QAction("Wait");
	QAction *changeDtr = new QAction("Change DTR");
	QAction *changeRts = new QAction("Change RTS");
	menu->addAction(sendAction);
	menu->addAction(waitAction);
	menu->addAction(changeDtr);
	menu->addAction(changeRts);
	addNewButton->setMenu(menu);
	connect(menu, &QMenu::triggered, [this, sendAction, waitAction, changeDtr, changeRts](QAction *action) {
		if (action == sendAction)
			addOperation(OperationType::Send);
		else if (action == waitAction)
			addOperation(OperationType::Wait);
		else if (action == changeDtr)
			addOperation(OperationType::ChangeDTR);
		else if (action == changeRts)
			addOperation(OperationType::ChangeRTS);
	});
	connect(addNewButton, &QPushButton::clicked, [addNewButton]() {
		addNewButton->showMenu();
	});

	QAction *removeAction = new QAction("Remove");
	QMenu *addBefore = new QMenu("Add before this");
	QMenu *addAfter = new QMenu("Add after this");
	QAction *addSendBefore = new QAction("Send");
	QAction *addWaitBefore = new QAction("Wait");
	QAction *addChangeDtrBefore = new QAction("Change DTR");
	QAction *addChangeRtsBefore = new QAction("Change RTS");
	QAction *addSendAfter = new QAction("Send");
	QAction *addWaitAfter = new QAction("Wait");
	QAction *addChangeDtrAfter = new QAction("Change DTR");
	QAction *addChangeRtsAfter = new QAction("Change RTS");
	QAction *moveUp = new QAction("Move up");
	QAction *moveDown = new QAction("Move down");

	addBefore->addAction(addSendBefore);
	addBefore->addAction(addWaitBefore);
	addBefore->addAction(addChangeDtrBefore);
	addBefore->addAction(addChangeRtsBefore);
	addAfter->addAction(addSendAfter);
	addAfter->addAction(addWaitAfter);
	addAfter->addAction(addChangeDtrAfter);
	addAfter->addAction(addChangeRtsAfter);

	m_itemMenu->addAction(removeAction);
	m_itemMenu->addSeparator();
	m_itemMenu->addMenu(addBefore);
	m_itemMenu->addMenu(addAfter);
	m_itemMenu->addSeparator();
	m_itemMenu->addAction(moveUp);
	m_itemMenu->addAction(moveDown);
	connect(m_itemMenu, &QMenu::aboutToShow, [moveUp, moveDown, this]() {
		moveUp->setEnabled(m_itemMenuIndex != 0);
		moveDown->setEnabled(m_itemMenuIndex != m_operations.size() - 1);
	});
	connect(m_itemMenu, &QMenu::triggered, [=](QAction *action) {
		int i = m_itemMenuIndex;
		if (action == removeAction)
			removeOperation(i, true);
		else if (action == addSendBefore)
			addOperation(OperationType::Send, i);
		else if (action == addWaitBefore)
			addOperation(OperationType::Wait, i);
		else if (action == addChangeDtrBefore)
			addOperation(OperationType::ChangeDTR, i);
		else if (action == addChangeRtsBefore)
			addOperation(OperationType::ChangeRTS, i);
		else if (action == addSendAfter)
			addOperation(OperationType::Send, i + 1);
		else if (action == addWaitAfter)
			addOperation(OperationType::Wait, i + 1);
		else if (action == addChangeDtrAfter)
			addOperation(OperationType::ChangeDTR, i + 1);
		else if (action == addChangeRtsAfter)
			addOperation(OperationType::ChangeRTS, i + 1);
		else if (action == moveUp)
			moveOperation(i, i - 1);
		else if (action == moveDown)
			moveOperation(i, i + 1);
	});

	connect(m_sendButton, &QPushButton::clicked, this, &SendSequenceWindow::onSendClicked);
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

void SendSequenceWindow::moveOperation(int before, int after)
{
	for (int i : {before, after}) {
		const Operation &o = m_operations[i];
		m_operationsLayout->removeWidget(o.label);
		m_operationsLayout->removeWidget(o.input);
		m_operationsLayout->removeWidget(o.actionButton);
	}

	for (QPair<int, int> p : {QPair<int, int>{before, after}, {after, before}}) {
		const Operation &o = m_operations[p.first];
		int loc = p.second;
		m_operationsLayout->addWidget(o.label, loc, 0);
		m_operationsLayout->addWidget(o.input, loc, 1, 1, o.inputSpan);
		m_operationsLayout->addWidget(o.actionButton, loc, 3);
	}

	qSwap(m_operations[before], m_operations[after]);
}

void SendSequenceWindow::addOperation(SendSequenceWindow::OperationType type, int row)
{
	for (int i = m_operations.size() - 1; i >= row; --i) {
		m_operationsLayout->removeWidget(m_operations[i].label);
		m_operationsLayout->removeWidget(m_operations[i].input);
		m_operationsLayout->removeWidget(m_operations[i].actionButton);
		m_operationsLayout->addWidget(m_operations[i].label, i + 1, 0);
		m_operationsLayout->addWidget(m_operations[i].input, i + 1, 1, 1, m_operations[i].inputSpan);
		m_operationsLayout->addWidget(m_operations[i].actionButton, i + 1, 3);
	}

	Operation op(type, 1);
	if (type == OperationType::Send) {
		m_operationsLayout->addWidget(op.label = new QLabel("Send: "), row, 0);
		QLineEdit *input = new QLineEdit;
		m_operationsLayout->addWidget(input, row, 1, 1, 2);
		input->setFocus();
		op.input = input;
		op.inputSpan = 2;
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
	actionButton->setMenu(m_itemMenu);
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
		m_operationsLayout->addWidget(m_operations[j + 1].input, j, 1, 1, m_operations[j + 1].inputSpan);
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
	m_itemMenuIndex = i;

	QToolButton *button = static_cast<QToolButton *>(sender());
	button->showMenu();
}

void SendSequenceWindow::cancelSequence()
{
	if (m_currentOperation > 0 && m_currentOperation <= m_operations.count())
		m_operationsLayout->itemAtPosition(m_currentOperation - 1, 0)->widget()->setStyleSheet("color: green");

	m_timer->stop();
	m_currentOperation = -1;
	m_sendButton->setText("Send");
}

static const char *operationStrings[] = {
	"Send", "Wait", "ChangeDTR", "ChangeRTS",
};

static const char *pinOptionStrings[] = {
	"1", "0", "toggle",
};

void SendSequenceWindow::saveSequence()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(m_sequencesDirectory);
	const QString seqNameFilter = "Sequence files (*.seq)";
	dialog.setNameFilters({seqNameFilter, "All files (*)"});

	if (!dialog.exec())
		return;

	QString filePath = dialog.selectedFiles().first();
	if (filePath.isEmpty())
		return;

	if (dialog.selectedNameFilter() == seqNameFilter && !filePath.endsWith(".seq"))
		filePath.append(".seq");

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "Save sequence", QString("Failed to open file %1: %2").arg(filePath).arg(file.errorString()));
		return;
	}

	QJsonArray rootObj;
	for (const auto &op : m_operations) {
		QJsonObject obj;
		auto type = op.type;
		obj.insert("type", operationStrings[int(type)]);
		switch(type) {
		case OperationType::Send:
			obj.insert("value", static_cast<QLineEdit *>(op.input)->text());
			break;
		case OperationType::Wait:
			obj.insert("value", static_cast<QSpinBox *>(op.input)->value());
			break;
		case OperationType::ChangeDTR:
		case OperationType::ChangeRTS:
			obj.insert("value", pinOptionStrings[static_cast<QComboBox *>(op.input)->currentIndex()]);
			break;
		}
		rootObj.append(obj);
	}
	QJsonDocument document(rootObj);
	if (file.write(document.toJson()) == -1) {
		QMessageBox::critical(this, "Save sequence", QString("Failed to write to file %1: %2").arg(filePath).arg(file.errorString()));
		return;
	}

	addRecent(filePath);
}

void SendSequenceWindow::loadSequence()
{
	QString path = QFileDialog::getOpenFileName(this, "Load sequence", m_sequencesDirectory, "Sequence files (*.seq);;All Files (*)");
	if (path.isEmpty())
		return;

	loadSequence(path);
}

void SendSequenceWindow::loadSequence(const QString &filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, "Load sequence", QString("Failed to load file %1: %2").arg(filePath).arg(file.errorString()));
		return;
	}

	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	QJsonArray arr = doc.array();
	struct ParseError {};


	auto operationFromString = [](const QString &str) -> OperationType
	{
		for (unsigned int i = 0; i < sizeof(operationStrings) / sizeof(operationStrings[0]); ++i)
			if (operationStrings[i] == str)
				return OperationType(i);
		throw ParseError();
	};

	auto pinOptionFromString = [](const QString &str) -> int
	{
		for (unsigned int i = 0; i < sizeof(pinOptionStrings) / sizeof(pinOptionStrings[0]); ++i)
			if (pinOptionStrings[i] == str)
				return i;
		throw ParseError();
	};

	clearOperations();
	try {
		for (QJsonValueRef v : arr) {
			if (!v.isObject())
				throw ParseError();
			QJsonObject obj = v.toObject();
			if (!obj.contains("type") || !obj.contains("value"))
				throw ParseError();
			OperationType type = operationFromString(obj["type"].toString());
			addOperation(type);
			QWidget *input = m_operations.last().input;
			auto value = obj["value"];
			switch(type) {
			case OperationType::Send:
				if (!value.isString())
					throw ParseError();
				static_cast<QLineEdit *>(input)->setText(value.toString());
				break;
			case OperationType::Wait:
				if (!value.isDouble())
					throw ParseError();
				static_cast<QSpinBox *>(input)->setValue(value.toInt());
				break;
			case OperationType::ChangeDTR:
			case OperationType::ChangeRTS:
				if (!value.isString())
					throw ParseError();
				static_cast<QComboBox *>(input)->setCurrentIndex(pinOptionFromString(value.toString()));
				break;
			}
		}
	} catch(ParseError e) {
		QMessageBox::critical(this, "Load sequence", QString("Failed to load sequence from file %1: Invalid format").arg(filePath));
		clearOperations();
		return;
	}

	addRecent(filePath);
}

void SendSequenceWindow::addRecent(const QString &path)
{
	Config c;
	QStringList list = c.recentSequences();
	if (list.contains(path))
		return;
	m_recents->addAction(path);
	list.append(path);
	c.setRecentSequences(list);
}
