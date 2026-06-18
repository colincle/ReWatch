#include "SearchResults.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"
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
    : QWidget(parent), appStorage(storage)
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	setAttribute(Qt::WA_StyledBackground, true);

	setupLayout();
}

void SearchResults::setupLayout()
{
	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(20, 20, 20, 20);

	spinner = new Spinner(Palette::accent, 8, this);
	spinner->setFixedSize(48, 48);
	spinner->hide();

	m_layout->addWidget(spinner, 0, Qt::AlignCenter);

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

	m_layout->addWidget(scrollArea);
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

	connect(
	    omdbSearch,
	    &OmdbSearch::searchFinished,
	    this,
	    [this, omdbSearch]()
	    {
		    if(omdbSearch != currentSearch)
		    {
			    omdbSearch->deleteLater();
			    return;
		    }

		    currentSearch = nullptr;
		    spinner->hide();
		    onSearchFinished(omdbSearch);
	    }
	);

	omdbSearch->search();
}

void SearchResults::onSearchFinished(OmdbSearch *omdbSearch)
{
	const Results &r = omdbSearch->getResults();

	if(r.errorType == SearchErrorType::AuthInvalid ||
	   r.errorType == SearchErrorType::Network)
	{
		clearResultsLayout();
		scrollArea->hide();
		clearExtraLayoutWidgets();

		emit searchError(
		    r.errorType == SearchErrorType::AuthInvalid ? API_KEY_ERROR_MESSAGE
		                                                : SEARCH_NETWORK_ERROR_MESSAGE
		);

		omdbSearch->deleteLater();
		return;
	}

	if(r.errorType == SearchErrorType::NotFound)
	{
		setFullPageState(AssetsPaths::noMoviesFound);
		omdbSearch->deleteLater();
		return;
	}

	int row = 0;
	int col = 0;

	for(const ResultTitle &title : r.titles)
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

	QTimer::singleShot(
	    0,
	    this,
	    [this]()
	    {
		    for(ElidedLabel *label : resultsContainer->findChildren<ElidedLabel *>())
		    {
			    label->refreshElision();
		    }
	    }
	);
}

namespace
{

QLabel *makePosterLabel(const ResultTitle &title)
{
	auto *poster = new QLabel;
	poster->setFixedSize(100, 150);
	poster->setStyleSheet("border: none; background: transparent;");
	poster->setPixmap(title.posterImage.scaled(
	    poster->size(),
	    Qt::KeepAspectRatio,
	    Qt::SmoothTransformation
	));
	return poster;
}

QLabel *makeTitleLabel(const ResultTitle &title)
{
	QFont font;
	font.setPixelSize(24);
	font.setBold(true);

	auto *label = new ElidedLabel(title.title);
	label->setFont(font);
	label->setStyleSheet(
	    QStringLiteral("color: %1; border: none; background: transparent;")
	        .arg(Palette::textPrimary)
	);
	label->setFixedHeight(QFontMetrics(font).height());
	return label;
}

QLabel *makeYearLabel(const ResultTitle &title)
{
	QFont font;
	font.setPixelSize(15);
	font.setBold(true);

	auto *label = new QLabel(title.year);
	label->setFont(font);
	label->setStyleSheet(
	    QStringLiteral("color: %1; border: none; background: transparent;")
	        .arg(Palette::textSecondary)
	);
	label->setFixedHeight(QFontMetrics(font).height());
	return label;
}

QLabel *makePlotLabel(const ResultTitle &title)
{
	const bool hasPlot = !title.plot.isEmpty() && title.plot != "N/A";

	auto *label = new ElidedLabel(hasPlot ? title.plot : QString(), 0);
	label->setStyleSheet(
	    QStringLiteral(
	        "color: %1; font-size: 12px; border: none; background: transparent;"
	    )
	        .arg(Palette::textSecondary)
	);
	label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	return label;
}

} // namespace

QWidget *SearchResults::makeResultRow(const ResultTitle &title)
{
	auto *row = new QWidget;
	row->setStyleSheet(QStringLiteral(
	                       "background-color: %1;"
	                       "border: 1px solid %2;"
	                       "border-radius: 10px;"
	)
	                       .arg(Palette::bgSecondary, Palette::border));

	row->setFixedHeight(174);

	auto *rowLayout = new QHBoxLayout(row);
	rowLayout->setContentsMargins(12, 12, 12, 12);
	rowLayout->setSpacing(20);

	rowLayout->addWidget(makePosterLabel(title));
	rowLayout->addWidget(makeTitleInfo(title), 1);
	rowLayout->addWidget(
	    appStorage.contains(title.imdbId) ? makeDoneButton(title, row)
	                                      : makeAddButton(title, row)
	);

	return row;
}

QWidget *SearchResults::makeTitleInfo(const ResultTitle &title)
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

IconButton *SearchResults::makeDoneButton(const ResultTitle &title, QWidget *row)
{
	auto *doneButton = new IconButton(
	    AssetsPaths::addedIcon,
	    40,
	    Palette::success,
	    Palette::surface,
	    row
	);

	connect(
	    doneButton,
	    &QPushButton::clicked,
	    this,
	    [this, title, doneButton, row]()
	    {
		    appStorage.deleteTitle(title.imdbId);
		    auto *addButton = makeAddButton(title, row);
		    qobject_cast<QHBoxLayout *>(row->layout())
		        ->replaceWidget(doneButton, addButton);
		    doneButton->deleteLater();
	    }
	);

	return doneButton;
}

IconButton *SearchResults::makeAddButton(const ResultTitle &title, QWidget *row)
{
	auto *addButton =
	    new IconButton(AssetsPaths::addIcon, 40, Palette::accent, Palette::surface, row);

	connect(
	    addButton,
	    &QPushButton::clicked,
	    this,
	    [this, title, addButton, row]() { onAddClicked(title, addButton, row); }
	);

	return addButton;
}

void SearchResults::restoreRowButton(
    QWidget *row, Spinner *rowSpinner, IconButton *oldAddButton, IconButton *replacement
)
{
	qobject_cast<QHBoxLayout *>(row->layout())->replaceWidget(rowSpinner, replacement);
	rowSpinner->deleteLater();
	oldAddButton->deleteLater();
}

void SearchResults::onAddClicked(
    const ResultTitle &title, IconButton *addButton, QWidget *row
)
{
	auto *rowSpinner = new Spinner(Palette::accent, 6, row);
	rowSpinner->setFixedSize(40, 40);

	auto *rowLayout = qobject_cast<QHBoxLayout *>(row->layout());
	rowLayout->replaceWidget(addButton, rowSpinner);
	addButton->hide();

	auto *fetch = new OmdbSearch(appStorage, appStorage.getKey(), this);

	connect(
	    fetch,
	    &OmdbSearch::titleFetched,
	    this,
	    [this, title, rowSpinner, addButton, row, fetch]()
	    {
		    restoreRowButton(row, rowSpinner, addButton, makeDoneButton(title, row));
		    fetch->deleteLater();
	    }
	);

	connect(
	    fetch,
	    &OmdbSearch::titleFetchFailed,
	    this,
	    [this, title, rowSpinner, addButton, row, fetch]()
	    {
		    restoreRowButton(row, rowSpinner, addButton, makeAddButton(title, row));
		    fetch->deleteLater();
	    }
	);

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

	m_layout->addWidget(label, 1);
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
	for(int i = m_layout->count() - 1; i >= 0; --i)
	{
		QLayoutItem *item = m_layout->itemAt(i);

		if(item->widget() && item->widget() != spinner && item->widget() != scrollArea)
		{
			item->widget()->deleteLater();
			delete m_layout->takeAt(i);
		}
	}
}