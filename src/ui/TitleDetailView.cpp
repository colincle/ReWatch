#include "TitleDetailView.hpp"
#include "AssetsPaths.hpp"
#include "FlowWidget.hpp"
#include "Palette.hpp"
#include "StreamingPlatformButton.hpp"
#include "TextButton.hpp"

#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

static constexpr int    MARGIN = 24;
static constexpr double POSTER_RATIO = 0.66;
static constexpr int    POSTER_RADIUS = 10;

TitleDetailView::TitleDetailView(AppStorage &appStorage, QWidget *parent)
    : QWidget(parent), appStorage(appStorage)
{
	setupUi();
	connect(
	    &appStorage,
	    &AppStorage::streamingPlatformsChanged,
	    this,
	    [this]()
	    {
		    if(!currentTitle.imdbId.isEmpty())
			    populateInfo(currentTitle);
	    }
	);
}

void TitleDetailView::refreshStyle()
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	topBar->setStyleSheet(
	    QStringLiteral("background-color: %1;").arg(Palette::bgPrimary)
	);
	if(backBtn)
		backBtn->updateColors(Palette::accent, Palette::surface);
	if(deleteBtn)
		deleteBtn->updateColors(Palette::error, Palette::surface);
	if(toWatchBtn)
		toWatchBtn->updateColors(Palette::error, Palette::surface);
	if(watchedBtn)
		watchedBtn->updateColors(Palette::success, Palette::surface);
	if(!currentTitle.imdbId.isEmpty())
		populateInfo(currentTitle);
}

void TitleDetailView::setTitle(const Title &title)
{
	currentTitle = title;
	currentPoster = title.posterImage;

	updatePosterSize();
	updateWatchButtons();
	populateInfo(title);
}

void TitleDetailView::setupUi()
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	setAttribute(Qt::WA_StyledBackground, true);

	auto *outerLayout = new QVBoxLayout(this);
	outerLayout->setContentsMargins(0, 0, 0, 0);
	outerLayout->setSpacing(0);
	outerLayout->addWidget(buildTopBar());
	outerLayout->addWidget(buildContentRow(), 1);
}

QWidget *TitleDetailView::buildTopBar()
{
	topBar = new QWidget;
	topBar->setStyleSheet(
	    QStringLiteral("background-color: %1;").arg(Palette::bgPrimary)
	);

	auto *layout = new QHBoxLayout(topBar);
	layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
	layout->setSpacing(8);

	backBtn = new IconButton(
	    AssetsPaths::crossIcon,
	    32,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	connect(backBtn, &QPushButton::clicked, this, &TitleDetailView::backRequested);

	toWatchBtn = new IconTextButton(
	    AssetsPaths::boxNotCheckedIcon,
	    "To watch",
	    36,
	    Palette::error,
	    Palette::surface,
	    true,
	    true,
	    this
	);
	watchedBtn = new IconTextButton(
	    AssetsPaths::boxCheckedIcon,
	    "Watched",
	    36,
	    Palette::success,
	    Palette::surface,
	    true,
	    true,
	    this
	);
	deleteBtn = new IconButton(
	    AssetsPaths::deleteIcon,
	    32,
	    Palette::error,
	    Palette::surface,
	    this
	);

	connect(toWatchBtn, &QPushButton::clicked, this, &TitleDetailView::onWatchToggled);
	connect(watchedBtn, &QPushButton::clicked, this, &TitleDetailView::onWatchToggled);
	connect(deleteBtn, &QPushButton::clicked, this, &TitleDetailView::onDeleteClicked);

	layout->addWidget(backBtn);
	layout->addStretch();
	layout->addWidget(toWatchBtn);
	layout->addWidget(watchedBtn);
	layout->addWidget(deleteBtn);

	return topBar;
}

QWidget *TitleDetailView::buildContentRow()
{
	auto *contentRow = new QWidget;
	contentRow->setStyleSheet("background: transparent;");

	auto *layout = new QHBoxLayout(contentRow);
	layout->setContentsMargins(MARGIN, 0, MARGIN, 0);
	layout->setSpacing(MARGIN);

	posterLabel = new QLabel;
	posterLabel->setAlignment(Qt::AlignCenter);
	posterLabel->setStyleSheet("background: transparent;");
	layout->addWidget(posterLabel, 0, Qt::AlignTop);

	infoContainer = new QWidget;
	infoContainer->setStyleSheet("background: transparent;");
	layout->addWidget(infoContainer, 1);

	return contentRow;
}

void TitleDetailView::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	updatePosterSize();
}

void TitleDetailView::updatePosterSize()
{
	const int h = static_cast<int>(height() * POSTER_RATIO);
	const int w = h * 2 / 3;
	posterLabel->setFixedSize(w, h);

	QPixmap result(w, h);
	result.fill(Qt::transparent);

	QPainter p(&result);
	p.setRenderHint(QPainter::Antialiasing);

	QPainterPath clip;
	clip.addRoundedRect(QRectF(0, 0, w, h), POSTER_RADIUS, POSTER_RADIUS);
	p.setClipPath(clip);

	if(currentPoster.isNull())
	{
		p.fillRect(0, 0, w, h, QColor(Palette::surface));
	}
	else
	{
		const QPixmap px =
		    currentPoster
		        .scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		p.drawPixmap((w - px.width()) / 2, (h - px.height()) / 2, px);
	}

	posterLabel->setPixmap(result);
}

void TitleDetailView::updateWatchButtons()
{
	toWatchBtn->setVisible(!currentTitle.viewed);
	watchedBtn->setVisible(currentTitle.viewed);
}

void TitleDetailView::updateWatchState()
{
	watchedValueLabel->setText(currentTitle.viewed ? "Yes" : "No");

	if(currentTitle.lastViewed.isValid())
	{
		lastWatchedValueLabel->setText(currentTitle.lastViewed.toString("MMMM d, yyyy"));
		lastWatchedRow->show();
	}
	else
	{
		lastWatchedRow->hide();
	}
}

void TitleDetailView::onWatchToggled()
{
	appStorage.toggleViewed(currentTitle.imdbId);
	currentTitle.viewed = !currentTitle.viewed;
	if(currentTitle.viewed)
		currentTitle.lastViewed = QDate::currentDate();
	updateWatchButtons();
	updateWatchState();
}

void TitleDetailView::onDeleteClicked()
{
	appStorage.deleteTitle(currentTitle.imdbId);
	emit backRequested();
}

void TitleDetailView::populateInfo(const Title &title)
{
	watchedValueLabel = nullptr;
	lastWatchedRow = nullptr;
	lastWatchedValueLabel = nullptr;

	if(QLayout *old = infoContainer->layout())
	{
		while(QLayoutItem *item = old->takeAt(0))
		{
			delete item->widget();
			delete item;
		}
		delete old;
	}

	auto *layout = new QVBoxLayout(infoContainer);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(9);

	addHeaderSection(layout, title);
	layout->addWidget(makeSeparator());
	addMetaSection(layout, title);
	if(!title.plot.isEmpty() && title.plot != "N/A")
		layout->addWidget(makeSeparator());
	addPlotSection(layout, title);
	addWatchOnSection(layout, title);
	layout->addStretch();
}

void TitleDetailView::addHeaderSection(QVBoxLayout *layout, const Title &title)
{
	auto *titleLabel = new QLabel(title.title);
	QFont titleFont;
	titleFont.setPixelSize(26);
	titleFont.setBold(true);
	titleLabel->setFont(titleFont);
	titleLabel->setWordWrap(true);
	titleLabel->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::accent));
	layout->addWidget(titleLabel);

	const QString typeStr = title.type == "series" ? "TV Series" : "Movie";
	auto         *subLabel =
	    new QLabel(title.year.isEmpty() ? typeStr : title.year + "  -  " + typeStr);
	QFont subFont;
	subFont.setPixelSize(15);
	subLabel->setFont(subFont);
	subLabel->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::textSecondary));
	layout->addWidget(subLabel);
}

void TitleDetailView::addMetaSection(QVBoxLayout *layout, const Title &title)
{
	auto add = [&](const QString &label, const QString &value)
	{
		if(!value.isEmpty() && value != "N/A")
			layout->addWidget(makeRow(label, value));
	};

	add("Released", title.released);
	add("Director", title.director);
	add("Actors", title.actors);
	if(title.type == "series")
	{
		add("Seasons", title.totalSeasons);
		if(title.lastChecked.isValid())
			layout->addWidget(
			    makeRow("Last checked", title.lastChecked.toString("MMMM d, yyyy"))
			);
	}
	if(title.rank > 0)
		layout->addWidget(makeRow("Rank", QString::number(title.rank)));
	auto *watchedRow = makeRow("Watched", title.viewed ? "Yes" : "No");
	watchedValueLabel = watchedRow->findChildren<QLabel *>().at(1);
	layout->addWidget(watchedRow);

	lastWatchedRow = makeRow(
	    "Last watched",
	    title.lastViewed.isValid() ? title.lastViewed.toString("MMMM d, yyyy") : ""
	);
	lastWatchedValueLabel = lastWatchedRow->findChildren<QLabel *>().at(1);
	lastWatchedRow->setVisible(title.lastViewed.isValid());
	layout->addWidget(lastWatchedRow);
}

void TitleDetailView::addPlotSection(QVBoxLayout *layout, const Title &title)
{
	if(title.plot.isEmpty() || title.plot == "N/A")
		return;

	auto *plotLabel = new QLabel(title.plot);
	plotLabel->setWordWrap(true);
	plotLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	QFont plotFont;
	plotFont.setPixelSize(14);
	plotLabel->setFont(plotFont);
	plotLabel->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::textSecondary));

	auto *plotArea = new QScrollArea;
	plotArea->setWidget(plotLabel);
	plotArea->setWidgetResizable(true);
	plotArea->setFrameShape(QFrame::NoFrame);
	plotArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plotArea->setStyleSheet(
	    "QScrollArea { background: transparent; border: none; }"
	    "QScrollArea > QWidget > QWidget { background: transparent; }"
	    "QScrollBar:vertical { width: 0px; }"
	);
	plotArea->setFixedHeight(60);
	layout->addWidget(plotArea);
}

void TitleDetailView::addWatchOnSection(QVBoxLayout *layout, const Title &title)
{
	auto *watchLabel = new QLabel("Watch on");
	QFont watchFont;
	watchFont.setPixelSize(14);
	watchFont.setBold(true);
	watchLabel->setFont(watchFont);
	watchLabel->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::accent));
	layout->addWidget(watchLabel);

	const auto &platforms = appStorage.getStreamingPlatforms();
	if(platforms.empty())
	{
		auto *emptyLabel = new QLabel(
		    "No custom streaming platforms added. You can set them up in Settings."
		);
		emptyLabel->setWordWrap(true);
		QFont emptyFont;
		emptyFont.setPixelSize(13);
		emptyLabel->setFont(emptyFont);
		emptyLabel->setStyleSheet(
		    QStringLiteral("color: %1;").arg(Palette::textSecondary)
		);
		layout->addWidget(emptyLabel);
		return;
	}

	auto *flow = new FlowWidget(8);
	flow->setStyleSheet("background: transparent;");

	for(const auto &p : platforms)
	{
		auto *btn = new StreamingPlatformButton(p, 30, Palette::accent, Palette::surface);
		connect(
		    btn,
		    &QPushButton::clicked,
		    this,
		    [p, title]()
		    {
			    const QByteArray query = QUrl::toPercentEncoding(title.title.toLower());
			    const QString    url =
			        QString(p.url).replace("movietracker", QString::fromUtf8(query));
			    QDesktopServices::openUrl(QUrl(url));
		    }
		);
		flow->addWidget(btn);
	}

	auto *tryAll = new TextButton("Try all", 30, Palette::surface, Palette::accent);
	connect(
	    tryAll,
	    &QPushButton::clicked,
	    this,
	    [platforms, title]()
	    {
		    const QByteArray query = QUrl::toPercentEncoding(title.title.toLower());
		    for(const auto &p : platforms)
		    {
			    const QString url =
			        QString(p.url).replace("movietracker", QString::fromUtf8(query));
			    QDesktopServices::openUrl(QUrl(url));
		    }
	    }
	);
	flow->addWidget(tryAll);
	layout->addWidget(flow);
}

QFrame *TitleDetailView::makeSeparator()
{
	auto *sep = new QFrame;
	sep->setFrameShape(QFrame::HLine);
	sep->setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::border));
	sep->setFixedHeight(1);
	return sep;
}

QWidget *TitleDetailView::makeRow(const QString &label, const QString &value)
{
	auto *row = new QWidget;
	row->setStyleSheet("background: transparent;");
	auto *layout = new QHBoxLayout(row);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	auto *lbl = new QLabel(label);
	QFont lblFont;
	lblFont.setPixelSize(14);
	lblFont.setBold(true);
	lbl->setFont(lblFont);
	lbl->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::textSecondary));
	lbl->setFixedWidth(110);

	auto *val = new QLabel(value);
	val->setWordWrap(true);
	QFont valFont;
	valFont.setPixelSize(14);
	val->setFont(valFont);
	val->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::textPrimary));

	layout->addWidget(lbl, 0, Qt::AlignTop);
	layout->addWidget(val, 1, Qt::AlignTop);
	return row;
}
