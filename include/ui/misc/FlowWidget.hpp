// Container that arranges child widgets in a wrapping horizontal flow, used for the
// streaming platform buttons on the title detail page.
#pragma once

#include <QList>
#include <QSize>
#include <QWidget>

class QTimer;

class FlowWidget : public QWidget
{
  public:
	explicit FlowWidget(int spacing, QWidget *parent = nullptr);

	void  addWidget(QWidget *w);
	QSize minimumSizeHint() const override;

  protected:
	void resizeEvent(QResizeEvent *event) override;
	void showEvent(QShowEvent *event) override;

  private:
	int              m_spacing;
	QTimer          *m_timer;
	QList<QWidget *> m_widgets;

	void reflow(int available);
};
