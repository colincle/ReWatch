#include "AddBar.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"
#include "IconButton.hpp"

#include <QHBoxLayout>
#include <QShowEvent>

static constexpr int BUTTON_HEIGHT = 40;

AddBar::AddBar(QWidget *parent) : QWidget(parent)
{
	setStyleSheet(QStringLiteral("background-color: %1; border-bottom: 1px solid %2;")
	                  .arg(Palette::bgSecondary, Palette::border));
	setAttribute(Qt::WA_StyledBackground, true);

	setupLayout();
	connectSignals();
}

void AddBar::setupLayout()
{
	auto *layout = new QHBoxLayout(this);
	layout->setContentsMargins(20, 0, 20, 0);
	layout->setSpacing(10);

	searchBar = new SearchBar(this);

	returnButton = new IconButton(
	    AssetsPaths::crossIcon,
	    BUTTON_HEIGHT,
	    Palette::accent,
	    Palette::surface,
	    this
	);

	layout->addWidget(searchBar, 1);
	layout->addWidget(returnButton);
}

void AddBar::connectSignals()
{
	connect(returnButton, &QPushButton::clicked, this, &AddBar::onReturnClicked);
	connect(searchBar, &QLineEdit::returnPressed, this, &AddBar::onSearchCommitted);
	connect(
	    searchBar,
	    &SearchBar::escapePressed,
	    this,
	    [this]()
	    {
		    searchBar->clear();
		    emit requestNormalMode();
	    }
	);
}

void AddBar::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	searchBar->setFocus();
}

void AddBar::onReturnClicked()
{
	emit requestNormalMode();
}

void AddBar::onSearchCommitted()
{
	QString query = searchBar->text().trimmed();

	if(!query.isEmpty())
	{
		emit searchRequested(query);
	}
}
