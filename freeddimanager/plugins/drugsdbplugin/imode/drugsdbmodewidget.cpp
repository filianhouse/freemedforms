/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *  Main Developer: Eric MAEKER, <eric.maeker@gmail.com>                   *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "drugsdbmodewidget.h"

//#include <coreplugin/icore.h>
//#include <coreplugin/itheme.h>
//#include <coreplugin/constants_icons.h>
//#include <coreplugin/isettings.h>

#include <utils/log.h>
#include <utils/global.h>
//#include <utils/httpdownloader.h>
//#include <translationutils/constants.h>
//#include <translationutils/trans_drugs.h>
//#include <translationutils/trans_menu.h>
//#include <translationutils/trans_msgerror.h>

//#include <QDataWidgetMapper>
//#include <QToolBar>
//#include <QDir>
//#include <QStringListModel>
//#include <QToolButton>
//#include <QSortFilterProxyModel>

#include "ui_drugsdbmodewidget.h"

using namespace DrugsDb;
using namespace Internal;
//using namespace Trans::ConstantTranslations;

//static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }
//static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }


namespace DrugsDb {
namespace Internal {
class DrugsDbModeWidgetPrivate
{
public:
    DrugsDbModeWidgetPrivate(DrugsDbModeWidget *parent):
        ui(0),
        q(parent)
    {
        Q_UNUSED(q);
    }

    ~DrugsDbModeWidgetPrivate()
    {}

    void setupUi()
    {
        ui = new Ui::DrugsDbModeWidget;
        ui->setupUi(q);
        ui->tabWidget->setCurrentIndex(0);
    }

    void createActionsAndToolBar()
    {
    }

    void connectActions()
    {
    }

public:
    Ui::DrugsDbModeWidget *ui;
    bool _testsRunning;

private:
    DrugsDbModeWidget *q;
};
}  // namespace Internal
}  // namespace DDI

DrugsDbModeWidget::DrugsDbModeWidget(QWidget *parent) :
    QWidget(parent),
    d(new DrugsDbModeWidgetPrivate(this))
{
    d->setupUi();
    layout()->setMargin(0);
    layout()->setSpacing(0);

    retranslateUi();
}

DrugsDbModeWidget::~DrugsDbModeWidget()
{
    if (d) {
        delete d->ui;
        delete d;
        d=0;
    }
}

void DrugsDbModeWidget::retranslateUi()
{
    d->ui->retranslateUi(this);
}

void DrugsDbModeWidget::changeEvent(QEvent *e)
{
    if (e->type()==QEvent::LanguageChange) {
        retranslateUi();
    }
}

#ifdef WITH_TESTS
#include <coreplugin/imainwindow.h>
#include <utils/randomizer.h>
#include <QTest>

//static inline Core::IMainWindow *mainWindow()  { return Core::ICore::instance()->mainWindow(); }

void DrugsDbModeWidget::test_runAllTests()
{
    d->_testsRunning = true;
}

void DrugsDbModeWidget::test_views()
{
//    // Proxy model over the central DDITableModel
//    QCOMPARE(d->_ddiProxyModel->sourceModel(), ddiCore()->drugDrugInteractionTableModel());
//    // Views connected to the proxy model
//    QCOMPARE(d->ui->tableView->model(), d->_ddiProxyModel);
//    // In this editor, mapper runs over the core model not the proxy model
//     QCOMPARE(d->_mapper->model(), ddiCore()->drugDrugInteractionTableModel());
}

#endif // WITH_TESTS
