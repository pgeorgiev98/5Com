#ifndef KEYSEQUENCEINPUTWIDGET_H
#define KEYSEQUENCEINPUTWIDGET_H

#include <QToolButton>
#include <QKeySequence>

class KeySequenceInputWidget : public QToolButton
{
	Q_OBJECT
public:
	explicit KeySequenceInputWidget(QKeySequence sequence, QWidget *parent = nullptr);
	QKeySequence sequence() const;

signals:
	void sequenceChanged(QKeySequence sequence);

public slots:
	void stopInputting();

private slots:
	void onClicked();

protected:
	void focusOutEvent(QFocusEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;

private:
	QKeySequence m_sequence;
	bool m_inputting;
};

#endif // KEYSEQUENCEINPUTWIDGET_H
