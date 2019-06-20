#ifndef INPUTFIELD_H
#define INPUTFIELD_H

#include <QComboBox>

class InputField : public QComboBox
{
	Q_OBJECT
public:
	explicit InputField(QWidget *parent = nullptr);

public slots:
	void onInputEntered();

protected:
	void keyPressEvent(QKeyEvent *) override;

private:
	QString m_searchPrefix;
	bool m_searching;
};

#endif // INPUTFIELD_H
