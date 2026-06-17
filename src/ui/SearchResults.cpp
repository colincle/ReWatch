#include "SearchResults.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"
#include "ElidedLabel.hpp"
#include "ErrorMessages.hpp"
#include "IconButton.hpp"
#include "OmdbSearch.hpp"
#include "Spinner.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

SearchResults::SearchResults(AppStorage &storage, QWidget *parent)
	: QWidget(parent)
	, appStorage(storage)
{
	setStyleSheet("background-color: " COLOR_BG_PRIMARY ";");
	setAttribute(Qt::WA_StyledBackground, true);

	setupLayout();
}

void SearchResults::setupLayout()
{
	layout = new QVBoxLayout(this);
	layout->setContentsMargins(20, 20, 20, 20);

	spinner = new Spinner(COLOR_ACCENT, 8, this);
	spinner->setFixedSize(48, 48);
	spinner->hide();

	layout->addWidget(spinner, 0, Qt::AlignCenter);

	resultsContainer = new QWidget;
	resultsLayout = new QGridLayout(resultsContainer);
	resultsLayout->setContentsMargins(0, 0, 0, 0);
	resultsLayout->setSpacing(12);
	resultsLayout->setColumnStretch(0, 1);
	resultsLayout->setColumnStretch(1, 1);
	resultsLayout->setAlignment(Qt::AlignTop);

	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidget(resultsContainer);
	scrollArea->hide();

	layout->addWidget(scrollArea);
}

void SearchResults::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	for(ElidedLabel *label : resultsContainer->findChildren<ElidedLabel *>())
	{
		label->refreshElision();
	}
}

void SearchResults::search(QString query)
{
	spinner->show();
	scrollArea->hide();
	clearExtraLayoutWidgets();
	scrollArea->verticalScrollBar()->setValue(0);
	clearResultsLayout();

	if(currentSearch)
	{
		disconnect(currentSearch, nullptr, this, nullptr);
		currentSearch->deleteLater();
		currentSearch = nullptr;
	}

	auto *omdbSearch = new OmdbSearch(appStorage, query, appStorage.getKey(), this);
	currentSearch = omdbSearch;

	connect(omdbSearch, &OmdbSearch::searchFinished, this, [this, omdbSearch]()
	{
		if(omdbSearch != currentSearch)
		{
			omdbSearch->deleteLater();
			return;
		}

		currentSearch = nullptr;
		spinner->hide();
		onSearchFinished(omdbSearch);
	});

	omdbSearch->search();
}

void SearchResults::onSearchFinished(OmdbSearch *omdbSearch)
{
	const results &r = omdbSearch->getResults();

	if(r.errorType == SearchErrorType::AuthInvalid || r.errorType == SearchErrorType::Network)
	{
		clearResultsLayout();
		scrollArea->hide();
		clearExtraLayoutWidgets();

		emit searchError(r.errorType == SearchErrorType::AuthInvalid
		                 ? API_KEY_ERROR_MESSAGE
		                 : "Couldn't reach OMDb — check your internet connection.");

		omdbSearch->deleteLater();
		return;
	}

	if(r.errorType == SearchErrorType::NotFound)
	{
		setFullPageState(NO_MOVIES_FOUND);
		omdbSearch->deleteLater();
		return;
	}

	int row = 0;
	int col = 0;

	for(const resultTitle &title : r.titles)
	{
		resultsLayout->addWidget(makeResultRow(title), row, col);

		if(++col >= 2)
		{
			col = 0;
			++row;
		}
	}

	scrollArea->show();
	omdbSearch->deleteLater();

	QTimer::singleShot(0, this, [this]()
	{
		for(ElidedLabel *label : resultsContainer->findChildren<ElidedLabel *>())
		{
			label->refreshElision();
		}
	});
}

QWidget *SearchResults::makeResultRow(const resultTitle &title)
{
	auto *row = new QWidget;
	row->setStyleSheet(
	    "background-color: " COLOR_BG_SECONDARY ";"
	    "border: 1px solid " COLOR_BORDER ";"
	    "border-radius: 10px;"
	);

	row->setFixedHeight(174);

	auto *rowLayout = new QHBoxLayout(row);
	rowLayout->setContentsMargins(12, 12, 12, 12);
	rowLayout->setSpacing(20);

	rowLayout->addWidget(makePosterLabel(title));
	rowLayout->addWidget(makeTitleInfo(title), 1);
	rowLayout->addWidget(appStorage.contains(title.imdbId)
	                     ? makeDoneButton(title, row)
	                     : makeAddButton(title, row));

	return row;
}

QLabel *SearchResults::makePosterLabel(const resultTitle &title)
{
	auto *poster = new QLabel;
	poster->setFixedSize(100, 150);
	poster->setStyleSheet("border: none; background: transparent;");
	poster->setPixmap(title.posterImage.scaled(
	                      poster->size(),
	                      Qt::KeepAspectRatio,
	                      Qt::SmoothTransformation));
	return poster;
}

QWidget *SearchResults::makeTitleInfo(const resultTitle &title)
{
	auto *container = new QWidget;
	container->setStyleSheet("background: transparent; border: none;");
	container->setFixedHeight(150);

	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(makeTitleLabel(title));
	layout->addSpacing(4);
	layout->addWidget(makeYearLabel(title));
	layout->addSpacing(10);
	layout->addWidget(makePlotLabel(title), 1);

	return container;
}

QLabel *SearchResults::makeTitleLabel(const resultTitle &title)
{
	QFont font;
	font.setPixelSize(24);
	font.setBold(true);

	auto *label = new ElidedLabel(title.title);
	label->setFont(font);
	label->setStyleSheet(
	    "color: " COLOR_TEXT_PRIMARY ";"
	    "border: none;"
	    "background: transparent;"
	);
	label->setFixedHeight(QFontMetrics(font).height());
	return label;
}

QLabel *SearchResults::makeYearLabel(const resultTitle &title)
{
	QFont font;
	font.setPixelSize(15);
	font.setBold(true);

	auto *label = new QLabel(title.year);
	label->setFont(font);
	label->setStyleSheet(
	    "color: " COLOR_TEXT_SECONDARY ";"
	    "border: none;"
	    "background: transparent;"
	);
	label->setFixedHeight(QFontMetrics(font).height());
	return label;
}

QLabel *SearchResults::makePlotLabel(const resultTitle &title)
{
	const bool hasPlot = !title.plot.isEmpty() && title.plot != "N/A";

	auto *label = new ElidedLabel(hasPlot ? title.plot : QString(), 0);
	label->setStyleSheet(
	    "color: " COLOR_TEXT_SECONDARY ";"
	    "font-size: 12px;"
	    "border: none;"
	    "background: transparent;"
	);
	label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	return label;
}

IconButton *SearchResults::makeDoneButton(const resultTitle &title, QWidget *row)
{
	auto *doneButton = new IconButton(ADDED_ICON, 40, COLOR_SUCCESS, COLOR_SURFACE, row);

	connect(doneButton, &QPushButton::clicked, this, [this, title, doneButton, row]()
	{
		appStorage.deleteTitle(title.imdbId);
		auto *addButton = makeAddButton(title, row);
		qobject_cast<QHBoxLayout *>(row->layout())->replaceWidget(doneButton, addButton);
		doneButton->deleteLater();
	});

	return doneButton;
}

IconButton *SearchResults::makeAddButton(const resultTitle &title, QWidget *row)
{
	auto *addButton = new IconButton(ADD_ICON, 40, COLOR_ACCENT, COLOR_SURFACE, row);

	connect(addButton, &QPushButton::clicked, this, [this, title, addButton, row]()
	{
		onAddClicked(title, addButton, row);
	});

	return addButton;
}

void SearchResults::restoreRowButton(QWidget *row, Spinner *rowSpinner, IconButton *oldAddButton,
                                     IconButton *replacement)
{
	qobject_cast<QHBoxLayout *>(row->layout())->replaceWidget(rowSpinner, replacement);
	rowSpinner->deleteLater();
	oldAddButton->deleteLater();
}

void SearchResults::onAddClicked(const resultTitle &title, IconButton *addButton, QWidget *row)
{
	auto *rowSpinner = new Spinner(COLOR_ACCENT, 6, row);
	rowSpinner->setFixedSize(40, 40);

	auto *rowLayout = qobject_cast<QHBoxLayout *>(row->layout());
	rowLayout->replaceWidget(addButton, rowSpinner);
	addButton->hide();

	auto *fetch = new OmdbSearch(appStorage, appStorage.getKey(), this);

	connect(fetch, &OmdbSearch::titleFetched, this,
	        [this, title, rowSpinner, addButton, row, fetch]()
	{
		restoreRowButton(row, rowSpinner, addButton, makeDoneButton(title, row));
		fetch->deleteLater();
	});

	connect(fetch, &OmdbSearch::titleFetchFailed, this,
	        [this, title, rowSpinner, addButton, row, fetch]()
	{
		restoreRowButton(row, rowSpinner, addButton, makeAddButton(title, row));
		fetch->deleteLater();
	});

	fetch->fetchById(title.imdbId, title.posterImage, title.posterNotFound);
}

void SearchResults::setFullPageState(const QString &imagePath)
{
	clearResultsLayout();
	scrollArea->hide();
	clearExtraLayoutWidgets();

	auto *label = new QLabel(this);
	label->setPixmap(QPixmap(imagePath));
	label->setAlignment(Qt::AlignCenter);
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	label->setMinimumSize(1, 1);
	label->setScaledContents(false);

	layout->addWidget(label, 1);
}

void SearchResults::clearResultsLayout()
{
	while(QLayoutItem *item = resultsLayout->takeAt(0))
	{
		if(item->widget())
		{
			item->widget()->deleteLater();
		}

		delete item;
	}
}

void SearchResults::clearExtraLayoutWidgets()
{
	for(int i = layout->count() - 1; i >= 0; --i)
	{
		QLayoutItem *item = layout->itemAt(i);

		if(item->widget() &&
		        item->widget() != spinner &&
		        item->widget() != scrollArea)
		{
			item->widget()->deleteLater();
			delete layout->takeAt(i);
		}
	}
}