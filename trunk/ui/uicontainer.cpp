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

#include "uicontainer.h"
#include "../gm_gameworld.h"
#include "../net/protocolgame.h"

extern int g_lastmousebutton;

winContainer_t::winContainer_t(Container* _container, uint32_t cid) {
    container = _container;
    containerId = cid;

    window.SetCaption(container->getName());
    // wide enough to display 4 items per row, tall enough to display all rows or three rows, whichever is smalle
    window.SetWidth(150 + 10); // 10 == for scrollbar
    window.SetHeight(MIN(4+36*3, 4 + (36*ceil(container->getCapacity()/4.))));

    // close button on titlebar
    // support added to glict svn 76+
    // this button needs to be relocated manually
    #if (GLICT_APIREV >= 76)
	window.AddTitlebarObject(&closebtn);
	closebtn.SetCaption("x");
	closebtn.SetWidth(12);
	closebtn.SetHeight(12);
	closebtn.SetPos(150 + 10 - 12, 2);
	closebtn.SetCustomData(this);
	closebtn.SetOnClick(OnClose);

	window.AddTitlebarObject(&btnCollapse);
	btnCollapse.SetCaption("-");
	btnCollapse.SetWidth(12);
	btnCollapse.SetHeight(12);
	btnCollapse.SetPos(150 + 10 - 24, 2);
	btnCollapse.SetCustomData(this);
	//btnCollapse.SetOnClick(OnCollapse);

	itemIcon = Item::CreateItem(container->getItemId(), 1);

	window.AddTitlebarObject(&pnlIcon);
	pnlIcon.SetWidth(12);
	pnlIcon.SetHeight(12);
	pnlIcon.SetPos(2, 2);
	pnlIcon.SetCustomData(itemIcon);
	pnlIcon.SetOnPaint(containerIconOnPaint);
	pnlIcon.SetBGActiveness(false);
    #else
    #warning For titlebar objects (such as close buttons) to work properly, you need GLICT APIREV 76+
    #endif

    // we're adding a winpanel object.
    // sadly, glictWindow does not have virtualsize handling implemented.
    // glictPanel does have virtual size; we'll add subpanels into glictPanel
    // which'll be of the same size as the window itself, and the only object
    // on the window.
    window.AddObject(&winpanel);
    winpanel.SetBGActiveness(false);
    winpanel.SetPos(0,0);
    winpanel.SetWidth(150 + 10); // same size as for window
    winpanel.SetHeight(MIN(4+36*3, 4 + (36*ceil(container->getCapacity()/4.))));
    // virtual size is always to display all rows
    winpanel.SetVirtualSize(160, 4 + (36*ceil(container->getCapacity()/4.)));

    for(uint32_t i = 0; i != container->getCapacity(); ++i)
    {
        glictPanel* panel = new glictPanel;
        panel->SetPos(5 + ((i % 4) * 36), 4 + (std::floor(i / 4) * 36));
        panel->SetWidth(32);
        panel->SetHeight(32);
        panel->SetBGColor(.1,.1,.1,1);
        panel->SetCaption("");
        panel->SetCustomData(this);
        panel->SetOnPaint(winContainer_t::containerItemOnPaint);
        panel->SetOnClick(winContainer_t::containerItemOnClick);
        panel->SetOnMouseDown(winContainer_t::containerItemOnMouseDown);
        panel->SetOnMouseUp(winContainer_t::containerItemOnMouseUp);

        panel->SetSkin(&g_skin.inv);

        winpanel.AddObject(panel);
        pnlItems.push_back(panel);
    }
}

winContainer_t::~winContainer_t()
{
	for(PanelList::iterator it = pnlItems.begin(); it != pnlItems.end(); ++it)
	{
		delete (*it);
	}

	pnlItems.clear();
	#if (GLICT_APIREV >= 76)
	delete itemIcon;
	#endif
}

void winContainer_t::OnClose(glictPos* pos, glictContainer *caller) {
    GM_Gameworld *gw = ((GM_Gameworld*)g_game);
	winContainer_t* window = (winContainer_t*)caller->GetCustomData();

    gw->m_protocol->sendCloseContainer(window->containerId);
}

void winContainer_t::containerItemOnPaint(glictRect *real, glictRect *clipped, glictContainer *caller)
{
	winContainer_t* window = (winContainer_t*)caller->GetCustomData();
	uint32_t slot_id = window->getSlotId(caller);
	Item* item = window->container->getItem(slot_id);

	if(item != NULL)
	{
		item->Blit((int)real->left, (int)real->top, 1.f);
	}
}

void winContainer_t::containerItemOnClick(glictPos *relmousepos, glictContainer* callerclass)
{
    if (g_lastmousebutton != SDL_BUTTON_LEFT)
        return;
	winContainer_t* window = (winContainer_t*)callerclass->GetCustomData();
	uint32_t slot_id = window->getSlotId(callerclass);
	Item* item = window->container->getItem(slot_id);
    Position p(0xFFFF, window->containerId | 0x40, slot_id);
    GM_Gameworld* gameclass = (GM_Gameworld*)g_game;

    if (gameclass->isExtendedUsing()) {
        gameclass->performExtendedUse(p, item, 0);
    } else
	if(SDL_GetModState() & KMOD_CTRL && item) {

	    if (!item->isExtendedUseable()) {
            gameclass->m_protocol->sendUseItem (p, item->getID(), slot_id);
        } else {
            gameclass->beginExtendedUse(item, slot_id, p);
        }
	}
	else if(SDL_GetModState() & KMOD_SHIFT && item) {
		GM_Gameworld* gameclass = (GM_Gameworld*)g_game;
		gameclass->m_protocol->sendLookItem(p, item->getID(), slot_id);
	}
}


void winContainer_t::containerItemOnMouseUp(glictPos *relmousepos,
	glictContainer* callerclass)
{
    if (g_lastmousebutton != SDL_BUTTON_LEFT)
        return;
    GM_Gameworld *gw = ((GM_Gameworld*)g_game);

	winContainer_t* window = (winContainer_t*)callerclass->GetCustomData();
    uint32_t slot_id = window->getSlotId(callerclass);
//	Item* item = window->container->getItem(slot_id);
	uint32_t id = window->container->getId();

	if (gw->isDragging())
	{
		Position dest(0xFFFF, id | 0x40, slot_id);
	    gw->dragComplete(dest);
        gw->dismissDrag();

	}
}

void winContainer_t::containerItemOnMouseDown(glictPos *relmousepos,
	glictContainer* callerclass)
{
    if (g_lastmousebutton != SDL_BUTTON_LEFT)
        return;
    GM_Gameworld *gw = ((GM_Gameworld*)g_game);

    winContainer_t* window = (winContainer_t*)callerclass->GetCustomData();
    uint32_t slot_id = window->getSlotId(callerclass);
	uint32_t id = window->container->getId();

    gw->setDragCtr(id, slot_id);

}

void winContainer_t::containerIconOnPaint(glictRect *real, glictRect *clipped, glictContainer *caller)
{
	Item* item = (Item*)caller->GetCustomData();
	item->Blit((int)real->left, (int)real->top, .33f);
}