#include "Overlay.h"
#include <QMouseEvent>

Overlay::Overlay(PatchList * pPatchList)
	: QWidget(Q_NULLPTR, Qt::Dialog)
{
	ui.setupUi(this);

	setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
	setAttribute(Qt::WA_TranslucentBackground);

	QTimer * inputHandler = new QTimer();
	connect(inputHandler, &QTimer::timeout, this, [&]()->void { handleAsyncKeyInput(); });
	inputHandler->start();

	patchList = pPatchList;

	QThread * scanThreads[10];

	QThread * scanThread = QThread::create([&]() {
		int found = 0;
		while (found < patchList->internalPatchList.size()) {
			found = 0;
			for (auto p : patchList->internalPatchList) {
				if (!p->m_addr) p->scan();
				else found++;
			}
			if (!patchList->PlayerHaveRequirementsSub->m_addr) patchList->PlayerHaveRequirementsSub->scan();
			if (!patchList->InventoryGuiIsVisible->m_addr) patchList->InventoryGuiIsVisible->scan();
		}
	});
	scanThread->start();

	connect(ui.cbPlayerInGodMode, &QCheckBox::stateChanged, this, [&]()->void { patchList->PlayerInGodMode->toggle(); });
	connect(ui.cbPlayerHaveStamina, &QCheckBox::stateChanged, this, [&]()->void { patchList->PlayerHaveStamina->toggle();  });
	connect(ui.cbPlayerIsDebugFlying, &QCheckBox::stateChanged, this, [&]()->void { patchList->PlayerIsDebugFlying->toggle();  });
	connect(ui.cbPlayerHaveRequirements, &QCheckBox::stateChanged, this, [&]()->void {
		if (patchList->PlayerHaveRequirementsSub->isPatched || patchList->PlayerHaveRequirementsMain->isPatched) {
			patchList->PlayerHaveRequirementsSub->unpatch();
			patchList->PlayerHaveRequirementsMain->unpatch();
		}
		else {
			if (patchList->PlayerHaveRequirementsMain->m_addr) {
				patchList->PlayerHaveRequirementsMain->patch();

				if (patchList->PlayerHaveRequirementsSub->m_addr) {
					patchList->PlayerHaveRequirementsSub->patch();
				}
			}
		}
		updateCheckboxes(); 
	});
	connect(ui.cbShipIsWindControllActive, &QCheckBox::stateChanged, this, [&]()->void { patchList->ShipIsWindControllActive->toggle(); updateCheckboxes(); });
	connect(ui.cbAttackGetLevelDamageFactor, &QCheckBox::stateChanged, this, [&]()->void { patchList->AttackGetLevelDamageFactor->toggle(); updateCheckboxes(); });
	connect(ui.cbInventoryIsTeleportable, &QCheckBox::stateChanged, this, [&]()->void { patchList->InventoryIsTeleportable->toggle(); updateCheckboxes(); });
	connect(ui.cbMonsterAIIsSleeping, &QCheckBox::stateChanged, this, [&]()->void { patchList->MonsterAIIsSleeping->toggle(); updateCheckboxes(); });
	connect(ui.cbInventoryMoveInventoryToGrave, &QCheckBox::stateChanged, this, [&]()->void { patchList->InventoryMoveInventoryToGrave->toggle(); updateCheckboxes(); });
	connect(ui.cbInventoryAddItem, &QCheckBox::stateChanged, this, [&]()->void { patchList->InventoryAddItem->toggle(); updateCheckboxes(); });

	checkBoxList.push_back(ui.cbPlayerInGodMode);
	checkBoxList.push_back(ui.cbPlayerHaveStamina);
	checkBoxList.push_back(ui.cbPlayerIsDebugFlying);
	checkBoxList.push_back(ui.cbPlayerHaveRequirements);
	checkBoxList.push_back(ui.cbShipIsWindControllActive);
	checkBoxList.push_back(ui.cbAttackGetLevelDamageFactor);
	checkBoxList.push_back(ui.cbInventoryIsTeleportable);
	checkBoxList.push_back(ui.cbMonsterAIIsSleeping);
	checkBoxList.push_back(ui.cbInventoryMoveInventoryToGrave);
	checkBoxList.push_back(ui.cbInventoryAddItem);
		
	connect(ui.exitButton, &QPushButton::pressed, this, [&]()->void { toggle(); });

	// 1. Set the cursor map
	QPixmap cursor_pix = QPixmap("./assets/cursor.png");
	QCursor c = QCursor(cursor_pix, 0, 0);
	setCursor(c);
}

Overlay::~Overlay()
{
}

void Overlay::updateCheckboxes()
{
	for (int i = 0; i < checkBoxList.size(); i++) {
		checkBoxList[i]->setChecked(patchList->internalPatchList[i]->isPatched);
	}
	//ui.cbPlayerInGodMode->setChecked(patchList->PlayerInGodMode->isPatched);
	//ui.cbPlayerHaveStamina->setChecked(patchList->PlayerHaveStamina->isPatched);
	//ui.cbPlayerIsDebugFlying->setChecked(patchList->PlayerIsDebugFlying->isPatched);
	//ui.cbPlayerHaveRequirements->setChecked(patchList->PlayerHaveRequirementsPiece->isPatched);
	//ui.cbShipIsWindControllActive->setChecked(patchList->ShipIsWindControllActive->isPatched);
	//ui.cbAttackGetLevelDamageFactor->setChecked(patchList->AttackGetLevelDamageFactor->isPatched);
	//ui.cbInventoryIsTeleportable->setChecked(patchList->InventoryIsTeleportable->isPatched);
	//ui.cbMonsterAIIsSleeping->setChecked(patchList->MonsterAIIsSleeping->isPatched);
	//ui.cbInventoryMoveInventoryToGrave->setChecked(patchList->InventoryMoveInventoryToGrave->isPatched);
	//ui.cbInventoryAddItem->setChecked(patchList->InventoryAddItem->isPatched);
}

void Overlay::validatePatches()
{
	for (int i = 0; i < checkBoxList.size(); i++) {
		checkBoxList[i]->setEnabled(patchList->internalPatchList[i]->m_addr);
		checkBoxList[i]->setStyleSheet(patchList->internalPatchList[i]->m_addr ? QString("QCheckBox {color: #ffffff}") : QString("QCheckBox {color: #888888}"));
	}
}

void Overlay::toggle() {
	if (isHidden()) {
		updateCheckboxes();
		patchList->InventoryGuiIsVisible->patch();
		show();
	}
	else {
		hide();
		patchList->InventoryGuiIsVisible->unpatch();
	}
}

void Overlay::handleAsyncKeyInput()
{
	if (GetAsyncKeyState(VK_DELETE) & 0x1) {
		toggle();
	}
	//else 
	//if(GetAsyncKeyState(VK_END) & 0x1) {
	//	hide();
	//	exit(1);
	//}
	/*else if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		if (GetAsyncKeyState(VK_NUMPAD1) & 0x1) {
			patchList->PlayerInGodMode->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD2) & 0x1) {
			patchList->PlayerHaveStamina->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD3) & 0x1) {
			patchList->PlayerIsDebugFlying->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD4) & 0x1) {
			patchList->PlayerHaveRequirementsRecipe->toggle();
			if (patchList->PlayerHaveRequirementsRecipe->isPatched) {
				patchList->PlayerHaveRequirementsPiece->scanAndPatch();
			}
			else {
				patchList->PlayerHaveRequirementsPiece->unpatch();
			}
		}
		else if (GetAsyncKeyState(VK_NUMPAD5) & 0x1) {
			patchList->ShipIsWindControllActive->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD6) & 0x1) {
			patchList->AttackGetLevelDamageFactor->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD7) & 0x1) {
			patchList->InventoryIsTeleportable->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD8) & 0x1) {
			patchList->MonsterAIIsSleeping->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD9) & 0x1) {
			patchList->InventoryMoveInventoryToGrave->toggle();
		}
		else if (GetAsyncKeyState(VK_NUMPAD0) & 0x1) {
			patchList->InventoryAddItem->toggle();
		}

	}*/
	validatePatches();
	updateCheckboxes();


	DWORD pCode = 0;
	GetExitCodeProcess(patchList->InventoryGuiIsVisible->processHandle, &pCode);
	if (pCode != STILL_ACTIVE) exit(1);
}

void Overlay::mousePressEvent(QMouseEvent *event)
{
	startPos = event->pos();
	QWidget::mousePressEvent(event);
}

void Overlay::mouseMoveEvent(QMouseEvent *event)
{
	QPoint delta = event->pos() - startPos;
	QWidget * w = window();
	if (w)
		w->move(w->pos() + delta);
	QWidget::mouseMoveEvent(event);
}