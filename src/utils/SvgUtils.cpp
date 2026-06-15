#include "SvgUtils.hpp"

#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

QIcon loadColoredSvg(const QString &path, const QString &color, int size)
{
	QFile file(path);

	if(!file.open(QIODevice::ReadOnly))
		return {};

	QSvgRenderer renderer(file.readAll());

	QPixmap pixmap(size, size);

	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	renderer.render(&painter);

	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

	painter.fillRect(pixmap.rect(), QColor(color));

	painter.end();

	return QIcon(pixmap);
}
