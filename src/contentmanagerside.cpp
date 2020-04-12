#include "contentmanagerside.h"
#include "ui_contentmanagerside.h"
#include "kiwixapp.h"

#include <QLocale>

#include "klistwidgetitem.h"
#include "static_content.h"

ContentManagerSide::ContentManagerSide(QWidget *parent) :
    QWidget(parent),
    mp_ui(new Ui::contentmanagerside)
{
    mp_ui->setupUi(this);
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { this->mp_contentManager->setLocal(!checked); });
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { this->mp_contentManager->setLocal(checked); });
    connect(mp_ui->allFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->allFileButton->setStyleSheet(
                    checked ? "*{font-weight: bold}" : "");});
    connect(mp_ui->localFileButton, &QRadioButton::toggled,
            this, [=](bool checked) { mp_ui->localFileButton->setStyleSheet(
                    checked ?"*{font-weight: bold}" : "");});
    mp_ui->localFileButton->setStyleSheet("*{font-weight: bold}");

    mp_ui->allFileButton->setText(gt("all-files"));
    mp_ui->localFileButton ->setText(gt("local-files"));
    mp_ui->languageButton->setText(gt("browse-by-language"));
    mp_ui->categoryButton->setText(gt("browse-by-category"));
    mp_ui->contentTypeButton->setText(gt("content-type"));

    mp_languageButton = mp_ui->languageButton;
    mp_languageSelector = mp_ui->languageSelector;
    connect(mp_languageButton, &QCheckBox::toggled, this, [=](bool checked) { mp_languageSelector->setHidden(!checked); });
    mp_languageSelector->setHidden(true);

    mp_categoryButton = mp_ui->categoryButton;
    mp_categorySelector = mp_ui->categorySelector;
    connect(mp_categoryButton, &QCheckBox::toggled, this, [=](bool checked) { mp_categorySelector->setHidden(!checked); });
    mp_categorySelector->setHidden(true);

    mp_contentTypeButton = mp_ui->contentTypeButton;
    mp_contentTypeSelector = mp_ui->contentTypeSelector;

    connect(mp_contentTypeButton, &QCheckBox::toggled, this, [=](bool checked) { mp_contentTypeSelector->setHidden(!checked); });
    mp_contentTypeSelector->setHidden(true);
    mp_contentTypeSelector->setSelectionMode(QAbstractItemView::MultiSelection);
    mp_contentTypeSelector->setTextElideMode(Qt::ElideNone);

    m_contentTypeStatesTurnover["no-filter"] = "yes";
    m_contentTypeStatesTurnover["yes"] = "no";
    m_contentTypeStatesTurnover["no"] = "no-filter";

    for(auto lang: S_LANGUAGES)
    {
        auto currentLang = QLocale().language();
        if (lang == QLocale::AnyLanguage) {
            auto item = new KListWidgetItem("All");
            item->setData(Qt::UserRole, lang);
            mp_languageSelector->addItem(item);
            continue;
        }
        auto locale = QLocale(lang);
        if (locale.language() != lang) {
            // Qt may not find the locale for the lang :/
            // In this case, Qt return the current locale
            // So we must be sure that the locale found correspond to the lang we want to add,
            // else we may add several time the current language.
            continue;
        }
        auto item = new KListWidgetItem(QLocale::languageToString(locale.language()));
        item->setData(Qt::UserRole, lang);
        mp_languageSelector->addItem(item);
        if (lang == currentLang) {
            item->setSelected(true);
        }
    }

    for (auto category: S_CATEGORIES)
    {
        auto item = new KListWidgetItem(category.second);
        item->setData(Qt::UserRole, category.first);
        mp_categorySelector->addItem(item);
        if (category.first ==  "all")
        {
            item->setSelected(true);
        }
    }

    for (auto contentType: S_CONTENTTYPE)
    {
        auto item = new QListWidgetItem(contentType.second);
        item->setData(Qt::UserRole, contentType.first);
        mp_contentTypeSelector->addItem(item);
        item->setFlags(Qt::NoItemFlags);
        if (!(contentType.first == "all")) {
            item->setText(item->text() + " : " + gt("no-filter"));
            item->setData(Qt::UserRole, contentType.first + ":no-filter");
        }
    }
}

ContentManagerSide::~ContentManagerSide()
{
    delete mp_ui;
}

void ContentManagerSide::onContentTypeFilterChanged(QListWidgetItem *item)
{
    auto currentFilterName = item->data(Qt::UserRole).toString().split(":")[0];
    if (currentFilterName == "all") {
        for (int i = 1; i < mp_contentTypeSelector->count(); i++) {
            auto filter = mp_contentTypeSelector->item(i);
            auto filterName = filter->data(Qt::UserRole).toString().split(":")[0];
            filter->setText(gt(filterName) + " : " + gt("no-filter"));
            filter->setData(Qt::UserRole, filterName + ":no-filter");
        }
        mp_contentManager->setCurrentContentTypeFilter("all");
        return;
    }
    auto currentFilterState = item->data(Qt::UserRole).toString().split(":")[1];
    item->setText(gt(currentFilterName) + " : " + gt(m_contentTypeStatesTurnover[currentFilterState]));
    item->setData(Qt::UserRole, currentFilterName + ":" + m_contentTypeStatesTurnover[currentFilterState]);
    QString contentTypeFilters;
    for (int i = 1; i < mp_contentTypeSelector->count(); i++) {
        auto filter = mp_contentTypeSelector->item(i);
        contentTypeFilters.append(filter->data(Qt::UserRole).toString() + ";");
    }
    mp_contentManager->setCurrentContentTypeFilter(contentTypeFilters);
}


void ContentManagerSide::setContentManager(ContentManager *contentManager)
{
    mp_contentManager = contentManager;
    connect(mp_languageSelector, &QListWidget::itemSelectionChanged,
            this, [=]() {
                auto item = mp_languageSelector->selectedItems().at(0);
                if (!item) return;
                auto langId = item->data(Qt::UserRole).toInt();
                auto lang = QLocale::Language(langId);
                if (lang == QLocale::AnyLanguage) {
                    mp_contentManager->setCurrentLanguage("*");
                    return;
                }
                auto locale = QLocale(lang);
                mp_contentManager->setCurrentLanguage(locale.name().split("_").at(0));
    });
    connect(mp_categorySelector, &QListWidget::itemSelectionChanged,
            this, [=]() {
                auto item = mp_categorySelector->selectedItems().at(0);
                if (!item) return;
                auto category = item->data(Qt::UserRole).toString();
                mp_contentManager->setCurrentCategoryFilter(category);
    });
    connect(mp_contentTypeSelector, &QListWidget::itemClicked,
            this, &ContentManagerSide::onContentTypeFilterChanged);
}
