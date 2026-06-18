#include "IconTextButton.hpp"
#include "SvgUtils.hpp"

#include <QFont>
#include <utility>
#include <QFontMetrics>
#include <QTimer>

static constexpr int ANIMATION_MS = 180;
static constexpr int ICON_TEXT_GAP = 6;
static constexpr int RIGHT_PADDING = 12;

IconTextButton::IconTextButton(
    const QString &iconPath, const QString &text, int size, QString color1,
    QString color2, bool alwaysShowText, bool textOnLeft, QWidget *parent
)
    : HoverButton(text, parent)
    , color1(std::move(color1))
    , color2(std::move(color2))
    , alwaysShowText(alwaysShowText)
{
	const int iconSize = size / 1.5;

	normalIcon = loadColoredSvg(iconPath, this->color1, iconSize);
	hoverIcon = loadColoredSvg(iconPath, this->color2, iconSize);

	QFont f = font();
	f.setPixelSize(static_cast<int>(size * 0.4));
	setFont(f);

	displayText = text;
	iconPadding = (size - iconSize) / 2;
	collapsedWidth = size;
	expandedWidth =
	    size + ICON_TEXT_GAP + QFontMetrics(f).horizontalAdvance(text) + RIGHT_PADDING;

	setFixedHeight(size);
	setIconSize(QSize(iconSize, iconSize));

	if(textOnLeft)
		setLayoutDirection(Qt::RightToLeft);

	if(alwaysShowText)
	{
		setFixedWidth(expandedWidth);
		applyNormal();
		return;
	}

	QPushButton::setText({});
	setFixedWidth(collapsedWidth);

	animation = new QVariantAnimation(this);
	animation->setDuration(ANIMATION_MS);
	animation->setEasingCurve(QEasingCurve::InOutQuad);
	connect(
	    animation,
	    &QVariantAnimation::valueChanged,
	    this,
	    [this](const QVariant &value) { setFixedWidth(value.toInt()); }
	);
	connect(
	    animation,
	    &QVariantAnimation::finished,
	    this,
	    [this]()
	    {
		    if(width() == collapsedWidth)
			    QPushButton::setText({});
	    }
	);

	applyNormal();
}

QString
IconTextButton::buildStyleSheet(const QString &bgColor, const QString &textColor) const
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1;"
	           "    color: %2;"
	           "    border: none;"
	           "    border-radius: 6px;"
	           "    text-align: left;"
	           "    padding-left: %3px;"
	           "    padding-right: %4px;"
	           "}"
	)
	    .arg(bgColor, textColor)
	    .arg(iconPadding)
	    .arg(RIGHT_PADDING);
}

void IconTextButton::applyNormal()
{
	setIcon(normalIcon);
	setStyleSheet(buildStyleSheet(color2, color1));
}

void IconTextButton::applyHover()
{
	setIcon(hoverIcon);
	setStyleSheet(buildStyleSheet(color1, color2));
}

void IconTextButton::animateTo(int targetWidth)
{
	animation->stop();
	animation->setStartValue(width());
	animation->setEndValue(targetWidth);
	animation->start();
}

void IconTextButton::enterEvent(QEnterEvent *event)
{
	applyHover();
	if(!alwaysShowText)
	{
		QPushButton::setText(displayText);
		animateTo(expandedWidth);
	}
	QPushButton::enterEvent(event);
}

void IconTextButton::leaveEvent(QEvent *event)
{
	applyNormal();
	if(!alwaysShowText)
		animateTo(collapsedWidth);
	QPushButton::leaveEvent(event);
}

void IconTextButton::showEvent(QShowEvent *event)
{
	QPushButton::showEvent(event);
	if(alwaysShowText)
		return;
	QTimer::singleShot(
	    0,
	    this,
	    [this]()
	    {
		    if(underMouse())
		    {
			    applyHover();
			    setFixedWidth(expandedWidth);
		    }
	    }
	);
}
