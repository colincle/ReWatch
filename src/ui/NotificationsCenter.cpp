// knownNotificationIds deduplicates rows so re-opening the panel or calling
// onNotificationsAdded twice never shows the same title twice.
#include "NotificationsCenter.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QUrl>
#include <QWidgetAction>

static QString notifTypeLabel(NotificationType type)
{
	switch(type)
	{
	case NotificationType::NewSeason:
		return QStringLiteral("New Season");
	case NotificationType::NewEpisode:
		return QStringLiteral("New Episode");
	case NotificationType::MovieRelease:
		return QStringLiteral("Now available");
	}
	return {};
}

static constexpr int NOTIFICATIONS_WIDTH = 280;
static constexpr int NOTIFICATION_POSTER_WIDTH = 48;
static constexpr int NOTIFICATION_POSTER_HEIGHT = 72;
static constexpr int POPUP_Y_OFFSET = 4;

static QString notificationsMenuStyleSheet()
{
	return QStringLiteral(
	           "QMenu {"
	           "    background-color: %1;"
	           "    border: 1px solid %2;"
	           "    border-radius: 8px;"
	           "    padding: 0px;"
	           "}"
	           "QMenu::item {"
	           "    border: none;"
	           "    margin: 0px;"
	           "    padding: 0px;"
	           "    background: transparent;"
	           "}"
	)
	    .arg(Palette::bgSecondary, Palette::border);
}

NotificationsCenter::NotificationsCenter(AppStorage &appStorage, QObject *parent)
    : QObject(parent), appStorage(appStorage)
{
	notificationSound.setSource(QUrl::fromLocalFile(AssetsPaths::notificationSound));
	notificationSound.setVolume(1.0);

	setupMenu();

	for(const Notification &notif : appStorage.getNotifications())
	{
		addNotificationRow(notif);
	}

	connect(
	    &appStorage,
	    &AppStorage::notificationsAdded,
	    this,
	    &NotificationsCenter::onNotificationsAdded,
	    Qt::QueuedConnection
	);
}

void NotificationsCenter::setupMenu()
{
	notificationsContainer = new QWidget;
	notificationsContainer->setFixedWidth(NOTIFICATIONS_WIDTH);
	notificationsContainer->setStyleSheet("background: transparent;");

	notificationsLayout = new QVBoxLayout(notificationsContainer);
	notificationsLayout->setContentsMargins(4, 4, 4, 4);
	notificationsLayout->setSpacing(4);

	noNotificationsLabel = new QLabel("No notifications", notificationsContainer);
	noNotificationsLabel->setStyleSheet(
	    QStringLiteral(
	        "color: %1; font-size: 14px; border: none; background: transparent;"
	    )
	        .arg(Palette::textSecondary)
	);
	notificationsLayout->addWidget(noNotificationsLabel);

	notificationsScrollArea = new QScrollArea;
	notificationsScrollArea->setWidget(notificationsContainer);
	notificationsScrollArea->setWidgetResizable(true);
	notificationsScrollArea->setFrameShape(QFrame::NoFrame);
	notificationsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	notificationsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	notificationsScrollArea->setFixedWidth(NOTIFICATIONS_WIDTH);
	notificationsScrollArea->setStyleSheet(
	    "QScrollArea { background: transparent; border: none; }"
	    "QScrollBar:vertical { width: 0px; }"
	);
	notificationsScrollArea->viewport()->setStyleSheet(
	    "background: transparent; border: none;"
	);

	notificationsMenu = new QMenu;
	notificationsMenu->setStyleSheet(notificationsMenuStyleSheet());

	auto *scrollAction = new QWidgetAction(notificationsMenu);
	scrollAction->setDefaultWidget(notificationsScrollArea);
	notificationsMenu->addAction(scrollAction);
}

const Title *NotificationsCenter::findTitleForNotification(const QString &imdbId) const
{
	auto guard = appStorage.lock();
	for(const Title &t : appStorage.getTitles(guard))
	{
		if(t.imdbId == imdbId)
			return &t;
	}

	return nullptr;
}

void NotificationsCenter::addNotificationRow(const Notification &notif)
{
	if(knownNotificationIds.contains(notif.imdbId))
	{
		return;
	}

	const Title *match = findTitleForNotification(notif.imdbId);

	if(!match)
	{
		return;
	}

	knownNotificationIds.insert(notif.imdbId);

	const QString typeLabel = notifTypeLabel(notif.type);
	const QString typeTag = QStringLiteral("<span style=\"color: %1;\">%2</span>")
	                            .arg(Palette::accentLight, typeLabel);
	const QString text = match->title + "<br>" + typeTag;

	auto *row = new QWidget;
	row->setObjectName("notificationRow");
	row->setAttribute(Qt::WA_StyledBackground, true);
	row->setAttribute(Qt::WA_Hover, true);
	row->setStyleSheet(
	    QStringLiteral(
	        "#notificationRow { background: transparent; border-radius: 6px; }"
	        "#notificationRow:hover { background-color: %1; }"
	    )
	        .arg(Palette::surface)
	);
	row->setProperty("imdbId", notif.imdbId);
	row->setProperty("notifType", static_cast<int>(notif.type));
	row->installEventFilter(this);

	auto *rowLayout = new QHBoxLayout(row);
	rowLayout->setContentsMargins(4, 4, 4, 4);
	rowLayout->setSpacing(8);

	auto *poster = new QLabel(row);
	poster->setFixedSize(NOTIFICATION_POSTER_WIDTH, NOTIFICATION_POSTER_HEIGHT);
	poster->setStyleSheet("border: none; background: transparent;");
	poster->setPixmap(match->posterImage.scaled(
	    poster->size(),
	    Qt::KeepAspectRatioByExpanding,
	    Qt::SmoothTransformation
	));

	auto *label = new QLabel(text, row);
	label->setTextFormat(Qt::RichText);
	label->setStyleSheet(
	    QStringLiteral(
	        "color: %1; font-size: 14px; border: none; background: transparent;"
	    )
	        .arg(Palette::textSecondary)
	);
	label->setWordWrap(true);

	rowLayout->addWidget(poster);
	rowLayout->addWidget(label, 1);

	notificationsLayout->insertWidget(0, row);

	noNotificationsLabel->setVisible(false);
}

void NotificationsCenter::onNotificationsAdded()
{
	for(const Notification &notif : appStorage.getNotifications())
	{
		addNotificationRow(notif);
	}

	notificationSound.play();
}

bool NotificationsCenter::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease)
	{
		const QString imdbId = obj->property("imdbId").toString();
		if(!imdbId.isEmpty())
		{
			const Title *t = findTitleForNotification(imdbId);
			if(t)
			{
				notificationsMenu->close();
				emit titleNavigationRequested(*t);
			}
		}
	}
	return QObject::eventFilter(obj, event);
}

void NotificationsCenter::refreshStyle()
{
	notificationsMenu->setStyleSheet(notificationsMenuStyleSheet());
	noNotificationsLabel->setStyleSheet(
	    QStringLiteral(
	        "color: %1; font-size: 14px; border: none; background: transparent;"
	    )
	        .arg(Palette::textSecondary)
	);
	for(QWidget *row : notificationsContainer->findChildren<QWidget *>("notificationRow"))
	{
		row->setStyleSheet(
		    QStringLiteral(
		        "#notificationRow { background: transparent; border-radius: 6px; }"
		        "#notificationRow:hover { background-color: %1; }"
		    )
		        .arg(Palette::surface)
		);
		const QString imdbId = row->property("imdbId").toString();
		const Title  *match = findTitleForNotification(imdbId);
		if(!match)
			continue;
		for(QLabel *label : row->findChildren<QLabel *>())
		{
			if(label->wordWrap())
			{
				const auto type =
				    static_cast<NotificationType>(row->property("notifType").toInt());
				const QString typeLabel = notifTypeLabel(type);
				const QString tag = QStringLiteral("<span style=\"color: %1;\">%2</span>")
				                        .arg(Palette::accentLight, typeLabel);
				label->setText(match->title + "<br>" + tag);
				label->setStyleSheet(QStringLiteral(
				                         "color: %1; font-size: 14px; border: none; "
				                         "background: transparent;"
				)
				                         .arg(Palette::textSecondary));
			}
		}
	}
}

void NotificationsCenter::popup(QWidget *anchor)
{
	const int maxHeight = anchor->window()->height() / 2;
	notificationsScrollArea->setMaximumHeight(maxHeight);

	const int    menuWidth = notificationsMenu->sizeHint().width();
	const QPoint position(anchor->width() - menuWidth, anchor->height() + POPUP_Y_OFFSET);

	notificationsMenu->popup(anchor->mapToGlobal(position));
	appStorage.removeNotifications();
}
