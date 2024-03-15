#include "Utilities.h"
#define checkSetting(teamval, enemyval) (((pLocalPlayer->teamId == pPlayer->teamId) && teamval) || ((pLocalPlayer->teamId != pPlayer->teamId) && enemyval))

#define pasteColor(x) x.r, x.g, x.b
#define pasteColorA(x) x.r, x.g, x.b, x.a

namespace Settings {
	D3DXCOLOR teamColor = D3DXCOLOR(0, 255, 0, 255);
	D3DXCOLOR occludedEnemyColor = D3DXCOLOR(255, 0, 0, 255);
	D3DXCOLOR visibleEnemyColor = D3DXCOLOR(255, 255, 0, 255);

	bool enemyBoxes = true;
	bool teamBoxes = true;

	bool enemyNames = true;
	bool teamNames = true;

	bool enemyDistance = true;
	bool teamDistance = true;

	bool enemySnaplines = true;
	bool teamSnaplines = true;

	bool enemyHealth = true;
	bool teamHealth = true;

	int lineWidth = 2;
}


void drawLoop(Dxo* dxo, int width, int height) {
	s_Width = width;
	s_Height = height;

	GameRenderer* pGameRenderer = GameRenderer::GetInstance();
	if (!IsValidPtr(pGameRenderer) || !IsValidPtr(pGameRenderer->renderView)) return;
	s_ViewProj = pGameRenderer->renderView->viewProj;

	ClientPlayer* pLocalPlayer = (ClientPlayer*)PlayerManager::GetLocalPlayer();

	if (!IsValidPtr(pLocalPlayer)) return;
	if (!IsValidPtr(pLocalPlayer->name)) return;

	ClientSoldierEntity* pLocalSoldier = pLocalPlayer->clientSoldierEntity.GetData();

	if (!IsValidPtr(pLocalSoldier)) return;

	for (size_t i = 0; i < 64; i++) {

		ClientPlayer* pPlayer = (ClientPlayer*)PlayerManager::GetPlayerById(i);

		if (!IsValidPtr(pPlayer)) continue;
		

		if (pPlayer == pLocalPlayer) continue;

		ClientSoldierEntity* pSoldier = pPlayer->clientSoldierEntity.GetData();

		if (!IsValidPtr(pSoldier)) continue;

		if (!IsValidPtr(pSoldier->healthComponent)) continue;

		if (pSoldier->healthComponent->currentHealth <= 0) continue;

		D3DXCOLOR chosenColor;

		if (pPlayer->teamId == pLocalPlayer->teamId) {

			chosenColor = Settings::teamColor;
		}
		else {

			if (pSoldier->occluded) {

				chosenColor = Settings::occludedEnemyColor;
			}
			else {

				chosenColor = Settings::visibleEnemyColor;
			}
		}


		D3DXVECTOR3 footLocation = pSoldier->location;
		D3DXVECTOR3 headLocation = footLocation;

		if (pSoldier->poseType == 0) headLocation.y += 1.6;
		if (pSoldier->poseType == 1) headLocation.y += 1;
		if (pSoldier->poseType == 2) headLocation.y += .5;
		


		if (WorldToScreen(footLocation) && WorldToScreen(headLocation)) {
			float w2sHeight = Distance3D(footLocation, headLocation);
			float w2sWidth = w2sHeight;
			if (pSoldier->poseType == 0) w2sWidth /= 2;
			if (pSoldier->poseType == 1) w2sWidth /= 1.5;

			float distanceToPlayer = Distance3D(pLocalSoldier->location, pSoldier->location);


			if checkSetting(Settings::teamBoxes, Settings::enemyBoxes) {
				dxo->DrawBox(headLocation.x - w2sWidth / 2, headLocation.y, w2sWidth, w2sHeight, Settings::lineWidth, pasteColorA(chosenColor));
			}

			if (checkSetting(Settings::teamSnaplines, Settings::enemySnaplines)) {
				dxo->DrawLine(s_Width / 2, s_Height, footLocation.x, footLocation.y, Settings::lineWidth, pasteColor(chosenColor));
			}

			if (checkSetting(Settings::teamNames, Settings::enemyNames)) {
				dxo->DrawString(pPlayer->name, headLocation.x, headLocation.y + 20, pasteColor(chosenColor));
			}


			if (checkSetting(Settings::teamDistance, Settings::enemyDistance)) {
				dxo->DrawString(std::to_string((int)distanceToPlayer).c_str(), footLocation.x, footLocation.y + 20, pasteColor(chosenColor));
			}

			if (checkSetting(Settings::teamHealth, Settings::enemyHealth)) {
				float health = (pSoldier->healthComponent->currentHealth / pSoldier->healthComponent->maxHealth) * 255;
			
				D3DXCOLOR boxColor(255 - health, health, 0, 255);
				float border = 2;
				float box_width = 10;
				float boxOffset = w2sWidth * .75f;
				dxo->DrawFilledBox(headLocation.x + boxOffset, headLocation.y, 10, w2sHeight, 0, 0, 0, 255);
				dxo->DrawFilledBox(headLocation.x + boxOffset + border, headLocation.y + border, box_width - (2 * border), (w2sHeight - (2 * border))*(health / 255), pasteColorA(boxColor));
			}


		} 
	}
} 

DWORD WINAPI hackThread(LPVOID lpParam) {
	Dxo::SetupRenderer(drawLoop);
	for (;;) { Sleep(500); }
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {

		CreateThread(NULL, NULL, hackThread, hModule, NULL, NULL);
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		PlayerManager::Detach();
	}
	return TRUE;
}