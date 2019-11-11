#include "favoriteslistobject.h"
#include "config.h"

static FavoritesListObject *favoritesListObjectInstance = nullptr;

FavoritesListObject::FavoritesListObject(QObject *parent)
	: QObject(parent)
{
	Q_ASSERT(favoritesListObjectInstance == nullptr);
	favoritesListObjectInstance = this;
	m_favorites = Config().favoriteInputs();
	for (const QString &s : m_favorites)
		emit favoriteAdded(s);
}

FavoritesListObject *FavoritesListObject::instance()
{
	if (favoritesListObjectInstance == nullptr)
		new FavoritesListObject();
	return favoritesListObjectInstance;
}

const QStringList &FavoritesListObject::favorites() const
{
	return m_favorites;
}

bool FavoritesListObject::isFavorite(const QString &value) const
{
	return m_favorites.contains(value);
}

void FavoritesListObject::addFavorite(const QString &value)
{
	Q_ASSERT(!isFavorite(value));
	m_favorites.append(value);
	Config().setFavoriteInputs(m_favorites);
	emit favoriteAdded(value);
}

void FavoritesListObject::removeFavorite(const QString &value)
{
	Q_ASSERT(isFavorite(value));
	m_favorites.removeOne(value);
	Config().setFavoriteInputs(m_favorites);
	emit favoriteRemoved(value);
}
