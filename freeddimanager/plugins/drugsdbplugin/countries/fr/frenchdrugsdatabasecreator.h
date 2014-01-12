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
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef DDIMANAGER_DRUGSDB_INTERNAL_FRENCHDRUGSDATABASECREATOR_H
#define DDIMANAGER_DRUGSDB_INTERNAL_FRENCHDRUGSDATABASECREATOR_H

//#include <coreplugin/itoolpage.h>
//#include <coreplugin/ftb_constants.h>
#include <drugsdbplugin/idrugdatabase.h>

#include <QWidget>
#include <QMultiHash>

/**
 * \file frenchdrugsdatabasecreator.h
 * \author Eric Maeker
 * \version 0.10.0
 * \date 11 Jan 2014
*/

namespace DrugsDb {
namespace Internal {
//class FrDrugDatatabase;

//class FreeFrenchDrugsDatabasePage : public Core::IToolPage
//{
//    Q_OBJECT
//public:
//    explicit FreeFrenchDrugsDatabasePage(QObject *parent = 0);
//    ~FreeFrenchDrugsDatabasePage();

//    virtual QString id() const {return "FreeFrenchDrugsDatabasePage";}
//    virtual QString name() const;
//    virtual QString category() const;
//    virtual QIcon icon() const {return QIcon();}

//    // widget will be deleted after the show
//    virtual QWidget *createPage(QWidget *parent = 0);

//private:
//    FrDrugDatatabase *_step;
//};

//class NonFreeFrenchDrugsDatabasePage : public Core::IToolPage
//{
//    Q_OBJECT
//public:
//    explicit NonFreeFrenchDrugsDatabasePage(QObject *parent = 0);
//    ~NonFreeFrenchDrugsDatabasePage();

//    virtual QString id() const {return "NonFreeFrenchDrugsDatabasePage";}
//    virtual QString name() const;
//    virtual QString category() const;
//    virtual QIcon icon() const {return QIcon();}

//    // widget will be deleted after the show
//    virtual QWidget *createPage(QWidget *parent = 0);

//private:
//    FrDrugDatatabase *_step;
//};

class FrDrugDatatabase : public DrugsDb::Internal::IDrugDatabase
{
    Q_OBJECT

public:
    FrDrugDatatabase(QObject *parent = 0);
    ~FrDrugDatatabase();

    QString id() const {return "FrDrugDatatabase";}
    void setLicenseType(LicenseType type);

    bool process();
    QString processMessage() const;

    bool prepareData();
    bool populateDatabase();
    bool linkMolecules();

    QStringList errors() const {return m_Errors;}

private:
    QStringList m_Errors;
    bool m_WithProgress;
};

}  //  namespace Internal
}  //  namespace DrugsDb

#endif // DDIMANAGER_DRUGSDB_INTERNAL_FRENCHDRUGSDATABASECREATOR_H
