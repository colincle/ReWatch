#include "ElidedLabel.hpp"

#include <QFontMetrics>
#include <QResizeEvent>
#include <QTextLayout>

ElidedLabel::ElidedLabel(const QString &text, int maxLines, QWidget *parent)
    : QLabel(parent), fullText(text), maxLines(maxLines)
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
	return QSize(fm.horizontalAdvance("…"), fm.height() * qMax(1, maxLines));
}

void ElidedLabel::updateElidedText()
{
	const QFontMetrics fm = fontMetrics();
	const int          effectiveLines =
	    maxLines > 0 ? maxLines : qMax(1, height() / fm.lineSpacing());

	if(effectiveLines == 1)
	{
		QLabel::setText(fm.elidedText(fullText, Qt::ElideRight, width()));
		return;
	}

	QLabel::setText(buildLines(fm, effectiveLines, width()).join('\n'));
}

QStringList
ElidedLabel::buildLines(const QFontMetrics &fm, int effectiveLines, int w) const
{
	QTextLayout layout(fullText, font());
	layout.beginLayout();

	struct Span
	{
		int start, length;
	};
	QVector<Span> spans;

	for(int i = 0; i < effectiveLines; ++i)
	{
		QTextLine line = layout.createLine();
		if(!line.isValid())
			break;
		line.setLineWidth(w);
		spans.push_back({line.textStart(), line.textLength()});
	}
	layout.endLayout();

	if(spans.isEmpty())
		return {};

	QStringList lines;
	for(int i = 0; i < spans.size() - 1; ++i)
		lines << fullText.mid(spans[i].start, spans[i].length).trimmed();

	const Span last = spans.last();
	const bool truncated = last.start + last.length < fullText.size();
	lines
	    << (truncated ? fm.elidedText(fullText.mid(last.start), Qt::ElideRight, w)
	                  : fullText.mid(last.start, last.length).trimmed());

	return lines;
}
