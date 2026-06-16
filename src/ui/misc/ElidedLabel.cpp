#include "ElidedLabel.hpp"

#include <QFontMetrics>
#include <QResizeEvent>

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent)
	: QLabel(parent)
	, fullText(text)
{
	setWordWrap(false);
	updateElidedText();
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
	QLabel::resizeEvent(event);
	updateElidedText();
}

void ElidedLabel::refreshElision()
{
	updateElidedText();
}

QSize ElidedLabel::minimumSizeHint() const
{
	const QFontMetrics fm = fontMetrics();
	return QSize(fm.horizontalAdvance("…"), fm.height());
}

void ElidedLabel::updateElidedText()
{
	QLabel::setText(fontMetrics().elidedText(fullText, Qt::ElideRight, width()));
}
