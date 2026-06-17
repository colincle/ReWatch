#pragma once

#include <QLabel>

class ElidedLabel : public QLabel
{
	Q_OBJECT

public:
	explicit ElidedLabel(const QString &text, int maxLines = 1, QWidget *parent = nullptr);

	void refreshElision();

	QSize minimumSizeHint() const override;

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	QString fullText;
	int maxLines;

	void updateElidedText();
};
