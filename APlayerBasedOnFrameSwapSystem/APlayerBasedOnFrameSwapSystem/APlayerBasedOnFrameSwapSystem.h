#pragma once

#include <QtWidgets/QWidget>
#include "ui_APlayerBasedOnFrameSwapSystem.h"

class APlayerBasedOnFrameSwapSystem : public QWidget
{
	Q_OBJECT

public:
	APlayerBasedOnFrameSwapSystem(QWidget *parent = Q_NULLPTR);

private:
	Ui::APlayerBasedOnFrameSwapSystemClass ui;
};
