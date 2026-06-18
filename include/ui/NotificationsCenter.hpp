#pragma once

#include "AppStorage.hpp"

#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QSet>
#include <QSoundEffect>
#include <QVBoxLayout>
#include <QWidget>

class NotificationsCenter : public QObject
{
	Q_OBJECT

  public:
	explicit NotificationsCenter(AppStorage &appStorage, QObject *parent = nullptr);

	void popup(QWidget *anchor);

  private:
	AppStorage &appStorage;

	QSoundEffect notificationSound;

	QMenu        *notificationsMenu;
	QScrollArea  *notificationsScrollArea;
	QWidget      *notificationsContainer;
	QVBoxLayout  *notificationsLayout;
	QLabel       *noNotificationsLabel;
	int           notificationRowCount = 0;
	QSet<QString> knownNotificationIds;

	void setupMenu();
	void onNotificationsAdded();

	void         addNotificationRow(const QString &imdbId);
	const Title *findTitleForNotification(const QString &imdbId) const;
};
