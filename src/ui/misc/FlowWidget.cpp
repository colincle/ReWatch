// reflow() deletes and recreates the layout on each call; there's no incremental update
// because Qt layouts don't support row-wrapping natively.
#include "FlowWidget.hpp"

#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>

FlowWidget::FlowWidget(int spacing, QWidget *parent) : QWidget(parent), m_spacing(spacing)
{
	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	m_timer->setInterval(150);
	connect(m_timer, &QTimer::timeout, this, [this]() { reflow(width()); });
}

void FlowWidget::addWidget(QWidget *w)
{
	w->setParent(this);
	m_widgets.append(w);
}

QSize FlowWidget::minimumSizeHint() const
{
	return QSize(0, 0);
}

void FlowWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	m_timer->start();
}

void FlowWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	QTimer::singleShot(0, this, [this]() { reflow(width()); });
}

void FlowWidget::reflow(int available)
{
	if(m_widgets.isEmpty() || available <= 0)
		return;

	delete layout();

	auto *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(m_spacing);

	QHBoxLayout *row = nullptr;
	int          rowWidth = 0;

	for(auto *w : m_widgets)
	{
		w->ensurePolished();
		const int bw = w->sizeHint().width();
		if(!row || (rowWidth > 0 && rowWidth + bw > available))
		{
			if(row)
				row->addStretch();
			row = new QHBoxLayout;
			row->setContentsMargins(0, 0, 0, 0);
			row->setSpacing(m_spacing);
			vbox->addLayout(row);
			rowWidth = 0;
		}
		row->addWidget(w);
		rowWidth += bw + m_spacing;
	}
	if(row)
		row->addStretch();

	layout()->activate();
	updateGeometry();
}
