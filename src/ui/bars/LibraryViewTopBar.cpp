#include "LibraryViewTopBar.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"

#include <QHBoxLayout>
#include <QKeyEvent>

static constexpr int SEARCH_INPUT_WIDTH = 220;

LibraryViewTopBar::LibraryViewTopBar(QWidget *parent)
	: QWidget(parent)
{
	setupLayout();
	connectSignals();
}

void LibraryViewTopBar::setupLayout()
{
	auto *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	showAllButton = new TextButton("All", 40, this);
	showToWatchButton = new TextButton("To watch", 40, this);
	showAllButton->toggleActive();

	searchInput = new QLineEdit(this);
	searchInput->setPlaceholderText(" Search...");
	searchInput->setClearButtonEnabled(true);
	searchInput->setFrame(false);
	searchInput->setFixedHeight(40);
	searchInput->installEventFilter(this);
	searchInput->setStyleSheet(
	    "QLineEdit {"
	    "    background-color: " COLOR_SURFACE ";"
	    "    color: " COLOR_TEXT_SECONDARY ";"
	    "    border: 1px solid " COLOR_BORDER ";"
	    "    border-radius: 10px;"
	    "    padding-left: 12px;"
	    "    padding-right: 28px;"
	    "    selection-background-color: " COLOR_ACCENT_LIGHT ";"
	    "    selection-color: white;"
	    "}"
	);
	searchInput->hide();

	searchButton = new IconButton(SEARCH_ICON, 40, COLOR_ACCENT, COLOR_SURFACE, this);
	closeButton = new IconButton(CROSS_ICON, 40, COLOR_ACCENT, COLOR_SURFACE, this);
	zoomInButton = new IconButton(ZOOM_IN_ICON, 40, COLOR_ACCENT, COLOR_SURFACE, this);
	zoomOutButton = new IconButton(ZOOM_OUT_ICON, 40, COLOR_ACCENT, COLOR_SURFACE, this);
	zoomInButton->setAutoRepeat(true);
	zoomInButton->setAutoRepeatDelay(200);
	zoomInButton->setAutoRepeatInterval(100);
	zoomOutButton->setAutoRepeat(true);
	zoomOutButton->setAutoRepeatDelay(200);
	zoomOutButton->setAutoRepeatInterval(100);
	closeButton->hide();

	layout->addWidget(showAllButton);
	layout->addWidget(showToWatchButton);
	layout->addSpacing(50);
	layout->addWidget(zoomOutButton);
	layout->addWidget(zoomInButton);
	layout->addStretch();
	layout->addWidget(searchInput);
	layout->addWidget(searchButton);
	layout->addWidget(closeButton);
}

void LibraryViewTopBar::connectSignals()
{
	connect(showAllButton, &QPushButton::clicked, this, &LibraryViewTopBar::onShowAllClicked);
	connect(showToWatchButton, &QPushButton::clicked, this, &LibraryViewTopBar::onShowToWatchClicked);
	connect(searchButton, &QPushButton::clicked, this, &LibraryViewTopBar::openSearch);
	connect(closeButton, &QPushButton::clicked, this, &LibraryViewTopBar::closeSearch);
	connect(searchInput, &QLineEdit::returnPressed, this, &LibraryViewTopBar::onSearchCommitted);
	connect(searchInput, &QLineEdit::textChanged, this, &LibraryViewTopBar::onSearchTextChanged);
	connect(zoomInButton, &QPushButton::clicked, this, [this]() { emit zoomRequested(10); });
	connect(zoomOutButton, &QPushButton::clicked, this, [this]() { emit zoomRequested(-10); });
}

void LibraryViewTopBar::onShowAllClicked()
{
	if(showAllButton->isActive())
	{
		return;
	}

	showAllButton->toggleActive();
	showToWatchButton->toggleActive();
	emit filterChanged(ViewFilter::All);
}

void LibraryViewTopBar::onShowToWatchClicked()
{
	if(showToWatchButton->isActive())
	{
		return;
	}

	showAllButton->toggleActive();
	showToWatchButton->toggleActive();
	emit filterChanged(ViewFilter::ToWatch);
}

void LibraryViewTopBar::onSearchCommitted()
{
	emit searchRequested(searchInput->text().trimmed());
}

void LibraryViewTopBar::onSearchTextChanged(const QString &text)
{
	if(text.isEmpty())
	{
		emit searchRequested("");
	}
}

void LibraryViewTopBar::openSearch()
{
	searchInput->setFixedWidth(SEARCH_INPUT_WIDTH);
	searchInput->show();
	searchInput->setFocus();
	searchButton->hide();
	closeButton->show();
}

void LibraryViewTopBar::closeSearch()
{
	searchInput->clear();
	searchInput->hide();
	emit searchRequested("");
	closeButton->hide();
	searchButton->show();
}

bool LibraryViewTopBar::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == searchInput && event->type() == QEvent::KeyPress)
	{
		auto *keyEvent = static_cast<QKeyEvent *>(event);

		if(keyEvent->key() == Qt::Key_Escape)
		{
			closeSearch();
			return true;
		}
	}

	return QWidget::eventFilter(obj, event);
}