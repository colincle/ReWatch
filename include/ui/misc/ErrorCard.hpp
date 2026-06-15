#pragma once

#include "IconButton.hpp"

#include <QLabel>
#include <QWidget>

class ErrorCard : public QWidget
{
	Q_OBJECT

public:
	explicit ErrorCard(QWidget *parent, const QString &message);

	void show();
};
