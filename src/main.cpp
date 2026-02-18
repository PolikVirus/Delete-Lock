// Delete lock by Polik

//Im always scared that I will click the delete level button on accident.
//This mod adds a "failsafe" that will ask you to enter the level name to proceed to the deletion.

//v1.0.1

#include <Geode/Geode.hpp>
#include <Geode/modify/FLAlertLayer.hpp>

#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

static bool sSkipNextDeleteAlert = false;

class DeleteNameConfirmPopup : public Popup {
    FLAlertLayer*   m_alert = nullptr;
    LevelInfoLayer* m_info  = nullptr;
    TextInput*      m_input = nullptr;

    bool init(FLAlertLayer* alert, LevelInfoLayer* info) {
        if (!Popup::init(280.f, 190.f))
            return false;

        m_alert = alert;
        m_info  = info;

        auto size  = m_size;
        auto level = m_info ? m_info->m_level : nullptr;
        std::string name = level ? level->m_levelName : "Unknown";

        auto title = CCLabelBMFont::create("Enter Level Name", "goldFont.fnt");
        if (!title)
            title = CCLabelBMFont::create("Enter Level Name", "bigFont.fnt");
        title->setScale(0.7f);
        title->setAlignment(kCCTextAlignmentCenter);
        title->setPosition(size.width / 2.f, size.height - 28.f);
        m_mainLayer->addChild(title);


        std::string text =
            "To delete this level, type\n"
            "its name exactly as shown:\n\"" + name + "\"";

        auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        label->setScale(0.35f);
        label->setAlignment(kCCTextAlignmentCenter);
        label->setPosition(size.width / 2.f, size.height - 60.f);
        m_mainLayer->addChild(label);

        float inputWidth = size.width - 40.f;
        m_input = TextInput::create(inputWidth, "Level name", "bigFont.fnt");
        if (!m_input) {
            sSkipNextDeleteAlert = true;
            return false;
        }
        m_input->setMaxCharCount(64);
        m_input->setTextAlign(TextInputAlign::Center);
        m_input->setPosition(size.width / 2.f, size.height / 2.f);
        m_mainLayer->addChild(m_input);

        auto deleteSpr = ButtonSprite::create("DELETE");
        auto deleteBtn = CCMenuItemSpriteExtra::create(
            deleteSpr, this, menu_selector(DeleteNameConfirmPopup::onOK)
        );

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        menu->addChild(deleteBtn);

        deleteBtn->setPosition(size.width / 2.f, 35.f);

        m_mainLayer->addChild(menu);
        return true;
    }

    void onOK(CCObject*) {
        if (!m_alert) {
            removeFromParent();
            return;
        }
        if (!m_info || !m_info->m_level || !m_input) {
            m_alert->release();
            m_alert = nullptr;
            removeFromParent();
            return;
        }

        std::string typed   = m_input->getString();
        std::string correct = m_info->m_level->m_levelName;

        if (typed == correct) {
            sSkipNextDeleteAlert = true;

            auto alert = m_alert;
            m_alert = nullptr;

            removeFromParent();

            alert->show();
            alert->release();
        } else {
            FLAlertLayer::create(
                "Wrong name",
                "The level name you typed doesn't match.",
                "OK"
            )->show();
        }
    }

    void onClose(CCObject* sender) override {
        if (m_alert) {
            m_alert->release();
            m_alert = nullptr;
        }
        Popup::onClose(sender);
    }

public:
    static DeleteNameConfirmPopup* create(FLAlertLayer* alert, LevelInfoLayer* info) {
        auto p = new DeleteNameConfirmPopup();
        if (p && p->init(alert, info)) {
            p->autorelease();
            return p;
        }
        delete p;
        return nullptr;
    }
};

class $modify(DeleteLockAlert, FLAlertLayer) {
public:
    void show() {
        if (sSkipNextDeleteAlert) {
            sSkipNextDeleteAlert = false;
            FLAlertLayer::show();
            return;
        }

        auto info = typeinfo_cast<LevelInfoLayer*>(m_alertProtocol);
        if (!info || !m_button2) {
            FLAlertLayer::show();
            return;
        }

        auto const& cap = m_button2->m_caption;
        if (cap == "DELETE" || cap == "Delete") {
            this->retain();
            if (auto p = DeleteNameConfirmPopup::create(this, info)) {
                p->show();
            } else {
                this->release();
                FLAlertLayer::show();
            }
            return;
        }

        FLAlertLayer::show();
    }
};
