#ifndef FAVORITESLISTOBJECT_H
#define FAVORITESLISTOBJECT_H

#include <QObject>
#include <QString>
#include <QStringList>

class FavoritesListObject : public QObject
{
	Q_OBJECT
public:
	static FavoritesListObject *instance();

public slots:
	const QStringList &favorites() const;
	bool isFavorite(const QString &value) const;
	void addFavorite(const QString &value);
	void removeFavorite(const QString &value);

signals:
	void favoriteAdded(QString value);
	void favoriteRemoved(QString value);

private:
	FavoritesListObject(QObject *parent = nullptr);
	QStringList m_favorites;
};

#endif // FAVORITESLISTOBJECT_H
