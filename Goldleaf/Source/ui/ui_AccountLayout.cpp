#include <ui/ui_AccountLayout.hpp>
#include <ui/ui_MainApplication.hpp>

extern set::Settings gsets;

namespace ui
{
    extern MainApplication *mainapp;

    AccountLayout::AccountLayout() : pu::ui::Layout()
    {
        this->optsMenu = new pu::ui::elm::Menu(0, 160, 1280, gsets.CustomScheme.Base, gsets.MenuItemSize, (560 / gsets.MenuItemSize));
        this->optsMenu->SetOnFocusColor(gsets.CustomScheme.BaseFocus);
        pu::ui::elm::MenuItem *itm = new pu::ui::elm::MenuItem(set::GetDictionaryEntry(208));
        itm->SetColor(gsets.CustomScheme.Text);
        itm->AddOnClick(std::bind(&AccountLayout::optsRename_Click, this));
        this->optsMenu->AddItem(itm);
        pu::ui::elm::MenuItem *itm2 = new pu::ui::elm::MenuItem(set::GetDictionaryEntry(209));
        itm2->SetColor(gsets.CustomScheme.Text);
        itm2->AddOnClick(std::bind(&AccountLayout::optsIcon_Click, this));
        this->optsMenu->AddItem(itm2);
        pu::ui::elm::MenuItem *itm3 = new pu::ui::elm::MenuItem(set::GetDictionaryEntry(210));
        itm3->SetColor(gsets.CustomScheme.Text);
        itm3->AddOnClick(std::bind(&AccountLayout::optsDelete_Click, this));
        this->optsMenu->AddItem(itm3);
        this->Add(this->optsMenu);
    }

    AccountLayout::~AccountLayout()
    {
        delete this->optsMenu;
    }

    void AccountLayout::Load(u128 UserId)
    {
        this->uid = UserId;

        AccountProfile prof;
        auto rc = accountGetProfile(&prof, UserId);
        if(rc != 0)
        {
            HandleResult(rc, set::GetDictionaryEntry(211));
            mainapp->UnloadMenuData();
            mainapp->LoadLayout(mainapp->GetMainMenuLayout());
            return;
        }

        AccountProfileBase pbase;
        AccountUserData udata;

        rc = accountProfileGet(&prof, &udata, &pbase);
        if(rc != 0)
        {
            HandleResult(rc, set::GetDictionaryEntry(211));
            mainapp->UnloadMenuData();
            mainapp->LoadLayout(mainapp->GetMainMenuLayout());
            return;
        }

        size_t iconsz = 0;
        rc = accountProfileGetImageSize(&prof, &iconsz);
        if(rc != 0)
        {
            HandleResult(rc, set::GetDictionaryEntry(211));
            mainapp->UnloadMenuData();
            mainapp->LoadLayout(mainapp->GetMainMenuLayout());
            return;
        }

        u8 *icon = new u8[iconsz]();
        size_t tmpsz = 0;
        rc = accountProfileLoadImage(&prof, icon, iconsz, &tmpsz);
        if(rc != 0)
        {
            HandleResult(rc, set::GetDictionaryEntry(211));
            mainapp->UnloadMenuData();
            mainapp->LoadLayout(mainapp->GetMainMenuLayout());
            return;
        }

        accountProfileClose(&prof);
        mainapp->LoadMenuHead(set::GetDictionaryEntry(212) + " " + pu::String(pbase.username));

        pu::String iconpth = "sdmc:/" + GoldleafDir + "/userdata/" + hos::FormatHex128(UserId) + ".jpg";
        fs::DeleteFile(iconpth);
        FILE *f = fopen(iconpth.AsUTF8().c_str(), "wb");
        if(f)
        {
            fwrite(icon, 1, iconsz, f);
            fclose(f);
        }
        delete[] icon;
    }

    void AccountLayout::optsRename_Click()
    {
        pu::String name = AskForText(set::GetDictionaryEntry(213), "", 10);
        if(!name.empty())
        {
            auto rc = acc::EditUser([&](AccountProfileBase *pbase, AccountUserData *udata)
            {
                strcpy(pbase->username, name.AsUTF8().c_str());
            });
            if(rc == 0)
            {
                mainapp->LoadMenuHead(set::GetDictionaryEntry(212) + " " + name);
                mainapp->ShowNotification(set::GetDictionaryEntry(214) + " \'" + name + "\'.");
            }
            else HandleResult(rc, set::GetDictionaryEntry(215));
        }
    }

    void AccountLayout::optsIcon_Click()
    {
        std::string iconpth = "/" + GoldleafDir + "/userdata/" + hos::FormatHex128(this->uid) + ".jpg";
        mainapp->CreateShowDialog(set::GetDictionaryEntry(216), set::GetDictionaryEntry(217) + "\n\'SdCard:" + iconpth + "\'", { set::GetDictionaryEntry(234) }, false, "sdmc:" + iconpth);
    }

    void AccountLayout::optsDelete_Click()
    {
        int sopt = mainapp->CreateShowDialog(set::GetDictionaryEntry(216), set::GetDictionaryEntry(218), { set::GetDictionaryEntry(111), set::GetDictionaryEntry(18) }, true);
        if(sopt == 0)
        {
            s32 ucount = 0;
            accountGetUserCount(&ucount);
            if(ucount < 2)
            {
                mainapp->CreateShowDialog(set::GetDictionaryEntry(216), set::GetDictionaryEntry(276), { set::GetDictionaryEntry(234) }, true);
                return;
            }
            auto rc = acc::DeleteUser(this->uid);
            if(rc == 0)
            {
                mainapp->ShowNotification(set::GetDictionaryEntry(219));
                mainapp->UnloadMenuData();
                mainapp->LoadLayout(mainapp->GetMainMenuLayout());
            }
            else HandleResult(rc, set::GetDictionaryEntry(220));
        }
    }
}