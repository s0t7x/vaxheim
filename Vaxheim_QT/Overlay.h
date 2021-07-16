#pragma once

#include <QWidget>
#include <qtimer.h>
#include <qthread.h>
#include "ui_Overlay.h"
#include "PatchList.h"

class Overlay : public QWidget
{
	Q_OBJECT

public:
	PatchList * patchList;
	std::vector<QCheckBox*> checkBoxList;

	Overlay(PatchList * pPatchList);
	~Overlay();

	void handleAsyncKeyInput();
	void updateCheckboxes();
	void validatePatches();


protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

private:
	QPoint startPos;
	Ui::Overlay ui;
};
