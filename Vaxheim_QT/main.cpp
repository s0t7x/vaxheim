#include <QtCore/QCoreApplication>
#include "PatchList.h"
#include "Overlay.h"
#include <qobject.h>

void terminationHandler() {
	std::cout << "terminating..." << std::endl; 
	gPatchList->unpatchAll();
	Sleep(2000);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);

	FreeConsole();

	enableDebugPriviliges();
	hookExitHandler(terminationHandler);

	uintptr_t valheimPID = NULL;
	valheimPID = getPIDByProcessName("valheim.exe");

	if (!valheimPID) {
		startViaSteam();
		Sleep(5000);
	}

	while (!valheimPID) {
		valheimPID = getPIDByProcessName("valheim.exe");
	}

	HANDLE hValheim = NULL;
	while (!hValheim) {
		hValheim = OpenProcess(PROCESS_ALL_ACCESS, NULL, valheimPID);
	}

	gPatchList = new PatchList(hValheim);
	gPatchList->InventoryGuiIsVisible->scan();

	Overlay * overlay = new Overlay(gPatchList);

	QObject::connect(&a, &QApplication::aboutToQuit, []() { terminationHandler(); });

    a.exec();

	return 1;
}
