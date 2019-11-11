#ifndef FAVORITESSELECTIONLIST_H
#define FAVORITESSELECTIONLIST_H

#include <QDialog>
#include <QString>

#include <optional>

class QListWidget;

class FavoritesSelectionList : public QDialog
{
	Q_OBJECT
public:
	explicit FavoritesSelectionList(QWidget *parent = nullptr);
	std::optional<QString> selectedString() const;

private slots:
	void updateList();

private:
	std::optional<QString> m_selectedString;
	QListWidget *m_list;
};

#endif // FAVORITESSELECTIONLIST_H
