#include "singlegearview.h"

#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>

#include "filecache.h"
#include "magic_enum.hpp"

SingleGearView::SingleGearView(GameData* data, FileCache& cache) : data(data) {
    gearView = new GearView(data, cache);
    gearView->setHair(-1);
    gearView->setEar(-1);
    gearView->setFace(-1);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(gearView);
    setLayout(layout);

    auto controlLayout = new QHBoxLayout();
    layout->addLayout(controlLayout);

    raceCombo = new QComboBox();
    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setRace((Race)raceCombo->itemData(index).toInt());
    });
    controlLayout->addWidget(raceCombo);

    subraceCombo = new QComboBox();
    connect(subraceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setSubrace((Subrace)raceCombo->itemData(index).toInt());
    });
    controlLayout->addWidget(subraceCombo);

    genderCombo = new QComboBox();
    connect(genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setGender((Gender)genderCombo->itemData(index).toInt());
    });
    controlLayout->addWidget(genderCombo);

    lodCombo = new QComboBox();
    connect(lodCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setLevelOfDetail(index);
    });
    controlLayout->addWidget(lodCombo);

    addToFMVButton = new QPushButton("Add to FMV");
    connect(addToFMVButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            Q_EMIT addToFullModelViewer(*currentGear);
        }
    });
    controlLayout->addWidget(addToFMVButton);

    exportButton = new QPushButton("Export...");
    connect(exportButton, &QPushButton::clicked, this, [this](bool) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Model"), "model.fbx", tr("FBX Files (*.fbx)"));

        gearView->exportModel(fileName);
    });
    controlLayout->addWidget(exportButton);

    connect(this, &SingleGearView::gearChanged, this, &SingleGearView::reloadGear);
    connect(this, &SingleGearView::raceChanged, this, [=] {
        gearView->setRace(currentRace);
    });
    connect(this, &SingleGearView::subraceChanged, this, [=] {
        gearView->setSubrace(currentSubrace);
    });
    connect(this, &SingleGearView::genderChanged, this, [=] {
        gearView->setGender(currentGender);
    });
    connect(this, &SingleGearView::levelOfDetailChanged, this, [=] {
        gearView->setLevelOfDetail(currentLod);
    });

    reloadGear();
}

void SingleGearView::clear() {
    currentGear.reset();

    Q_EMIT gearChanged();
}

void SingleGearView::setGear(const GearInfo& info) {
    currentGear = info;

    Q_EMIT gearChanged();
}

void SingleGearView::setRace(Race race) {
    if (currentRace == race) {
        return;
    }

    currentRace = race;
    Q_EMIT raceChanged();
}

void SingleGearView::setSubrace(Subrace subrace) {
    if (currentSubrace == subrace) {
        return;
    }

    currentSubrace = subrace;
    Q_EMIT subraceChanged();
}

void SingleGearView::setGender(Gender gender) {
    if (currentGender == gender) {
        return;
    }

    currentGender = gender;
    Q_EMIT genderChanged();
}

void SingleGearView::setLevelOfDetail(int lod) {
    if (currentLod == lod) {
        return;
    }

    currentLod = lod;
    Q_EMIT levelOfDetailChanged();
}

void SingleGearView::reloadGear() {
    gearView->clear();

    raceCombo->setEnabled(currentGear.has_value());
    subraceCombo->setEnabled(currentGear.has_value());
    genderCombo->setEnabled(currentGear.has_value());
    lodCombo->setEnabled(currentGear.has_value());
    addToFMVButton->setEnabled(currentGear.has_value());
    exportButton->setEnabled(currentGear.has_value());

    if (currentGear.has_value()) {
        gearView->addGear(*currentGear);

        loadingComboData = true;

        raceCombo->clear();
        subraceCombo->clear();
        for (auto [race, subrace] : gearView->supportedRaces()) {
            raceCombo->addItem(magic_enum::enum_name(race).data(), (int)race);
            subraceCombo->addItem(magic_enum::enum_name(subrace).data(), (int)subrace);
        }

        genderCombo->clear();
        for (auto gender : gearView->supportedGenders()) {
            genderCombo->addItem(magic_enum::enum_name(gender).data(), (int)gender);
        }

        lodCombo->clear();
        for (int i = 0; i < gearView->lodCount(); i++)
            lodCombo->addItem(QString::number(i));

        loadingComboData = false;
    }
}

#include "moc_singlegearview.cpp"