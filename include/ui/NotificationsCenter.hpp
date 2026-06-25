// Dropdown notification panel anchored to the bell button. Adds rows for new-season and
// new-episode alerts, plays a sound, and emits titleNavigationRequested on row click.
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
	void refreshStyle();

  signals:
	void titleNavigationRequested(const Title &title);

  private:
	AppStorage &appStorage;

	QSoundEffect notificationSound;

	QMenu        *notificationsMenu;
	QScrollArea  *notificationsScrollArea;
	QWidget      *notificationsContainer;
	QVBoxLayout  *notificationsLayout;
	QLabel       *noNotificationsLabel;
	QSet<QString> knownNotificationIds;

	void setupMenu();
	void onNotificationsAdded();

	void         addNotificationRow(const Notification &notif);
	const Title *findTitleForNotification(const QString &imdbId) const;

	bool eventFilter(QObject *obj, QEvent *event) override;
};
