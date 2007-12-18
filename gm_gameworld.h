//////////////////////////////////////////////////////////////////////
// Yet Another Tibia Client
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#ifndef __GM_GAMEWORLD_H
#define __GM_GAMEWORLD_H

#include <GLICT/container.h>
#include <GLICT/panel.h>
#include <GLICT/textbox.h>
#include <GLICT/button.h>
#include "gamemode.h"
#include "sprite.h"

class Console;

class pnlInventory_t {
public:
	pnlInventory_t() {
		static int posInvSlots[] = {
            45, 3,  // helmet
            8, 17,  // necklace
            83, 17, // backpack
            45, 40, // armor
            83, 54, // right weapon
            8, 54,  // left weapon
            45, 77, // legs
            45, 115,// feet
            8, 91,  // ring
            83, 91};// hand


		panel.SetHeight(150);
		panel.SetWidth(150);
		for (int i = 0; i < 10; i++) {
			panel.AddObject(pnlItem+i);
			pnlItem[i].SetPos(posInvSlots[i*2], posInvSlots[i*2+1]);
			pnlItem[i].SetHeight(32);
			pnlItem[i].SetWidth(32);
			pnlItem[i].SetBGColor(.1,.1,.1,1);
			pnlItem[i].SetCaption("");
//			pnlItem[i].SetBGActiveness(true);
		}
	}

	glictContainer panel; // it's a container, so it's transparent
	glictPanel pnlItem[10];

};

class GM_Gameworld : public GameMode {
public:
	GM_Gameworld();
	~GM_Gameworld();

	void updateScene();
	void keyPress (char key);
	void specKeyPress (int key);
	void doResize(float w, float h);
	void mouseEvent(SDL_Event& event);

	void onWalk();
	void onTextMessage(MessageType_t type, const std::string& message);
	void onCreatureSpeak(SpeakClasses_t type, int n, const std::string& name, int level, const Position& pos, const std::string& message);
	void onCreatureSpeak(SpeakClasses_t type, int n, const std::string& name, int level, int channel, const std::string& message);
	void onCreatureSpeak(SpeakClasses_t type, int n, const std::string& name, int level, const std::string& message);
private:
	Sprite* ui;
	glictContainer desktop;
	pnlInventory_t pnlInventory;
	glictPanel pnlTraffic;
	glictTextbox txtConsoleEntry;

	std::vector<Console> m_consoles;

	bool m_preWalk;
	int m_walkState;

	class ProtocolGame* m_protocol;
};

#endif

