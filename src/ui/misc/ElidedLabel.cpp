#include "ElidedLabel.hpp"

#include <QFontMetrics>
#include <QResizeEvent>

ElidedLabel::ElidedLabel(const QString &text, int maxLines, QWidget *parent)
	: QLabel(parent)
	, fullText(text)
	, maxLines(maxLines)
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
	const int effectiveLines = maxLines > 0 ? maxLines : qMax(1, height() / fm.lineSpacing());

	if(effectiveLines == 1)
	{
		QLabel::setText(fm.elidedText(fullText, Qt::ElideRight, width()));
		return;
	}

	const int w = width();
	const QStringList words = fullText.split(' ', Qt::SkipEmptyParts);
	QStringList lines(effectiveLines);
	int lineIndex = 0;

	for(int i = 0; i < words.size(); ++i)
	{
		const QString candidate = lines[lineIndex].isEmpty()
		                          ? words[i]
		                          : lines[lineIndex] + " " + words[i];

		if(lines[lineIndex].isEmpty() || fm.horizontalAdvance(candidate) <= w)
		{
			lines[lineIndex] = candidate;
			continue;
		}

		if(lineIndex == effectiveLines - 1)
		{
			const QString remaining = lines[lineIndex] + " " + QStringList(words.mid(i)).join(' ');
			lines[lineIndex] = fm.elidedText(remaining, Qt::ElideRight, w);
			break;
		}

		++lineIndex;
		lines[lineIndex] = words[i];
	}

	while(!lines.isEmpty() && lines.last().isEmpty())
	{
		lines.removeLast();
	}

	QLabel::setText(lines.join('\n'));
}
