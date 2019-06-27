#ifndef SENDSEQUENCEWINDOW_H
#define SENDSEQUENCEWINDOW_H

#include <QDialog>
#include <QVector>

class SerialPort;

class QGridLayout;
class QToolButton;
class QCheckBox;
class QSpinBox;

class SendSequenceWindow : public QDialog
{
	Q_OBJECT
	enum class OperationType { Send, Wait };
public:
	explicit SendSequenceWindow(SerialPort *port, QWidget *parent = nullptr);

signals:
	void operationsCountChanged(int count);

private slots:
	void onSendClicked();
	void addOperation(OperationType);
	void addOperation(OperationType, int);
	void clearOperations();
	void removeOperation(int i, bool adjustSize = false);
	void executeNextOperation();
	void onActionButtonClicked();
	void cancelSequence();

protected:
	void reject() override;

private:
	struct Operation {
		QWidget *label, *input, *actionButton;
		OperationType type;
	};
	QVector<Operation> m_operations;
	SerialPort *m_port;
	QGridLayout *m_operationsLayout;
	QToolButton *m_addnewButton;
	QToolButton *m_clearOperationsButton;
	QCheckBox *m_sendIndefinitely;
	QSpinBox *m_sequencesCount;
	QPushButton *m_sendButton;
	int m_currentOperation;
	QTimer *m_timer;
};

#endif // SENDSEQUENCEWINDOW_H
