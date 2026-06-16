#pragma once

#include <QLabel>

class ElidedLabel : public QLabel
{
	Q_OBJECT

public:
	explicit ElidedLabel(const QString &text, QWidget *parent = nullptr);

	void refreshElision();

	QSize minimumSizeHint() const override;

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	QString fullText;

	void updateElidedText();
};
