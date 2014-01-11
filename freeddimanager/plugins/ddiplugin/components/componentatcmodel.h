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
#ifndef DDIMANAGER_DDIPLUGIN_INTERNAL_COMPONENT_TO_ATC_MODEL_H
#define DDIMANAGER_DDIPLUGIN_INTERNAL_COMPONENT_TO_ATC_MODEL_H

#include <QAbstractTableModel>
#include <QMultiHash>
#include <QHash>
#include <QString>

/**
 * \file componentatcmodel.h
 * \author Eric Maeker
 * \version 0.10.0
 * \date 02 Dec 2013
*/

namespace DDI {
class DDICore;
namespace Internal {
class ComponentAtcModelPrivate;
}  // namespace Internal

class ComponentAtcModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class DDI::DDICore;

protected:
    explicit ComponentAtcModel(QObject *parent = 0);
    bool initialize();
    bool onDdiDatabaseChanged();

public:
    enum DataRepresentation {
        FancyButton = 0,
        Id,
        Uid,
        DrugDatabaseComponentUid1,
        DrugDatabaseComponentUid2,
        IsValid,
        IsReviewed,
        Name,
        AtcCodeList,
        SuggestedAtcCodeList,
        DateCreation,
        DateUpdate,
        Reviewer,
        Comments,
        ColumnCount
    };
    ~ComponentAtcModel();

    QStringList availableDrugsDatabases() const;
    bool selectDatabase(const QString &dbUid, const QString &dbUid2 = QString::null);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool addUnreviewedMolecules(const QString &dbUid, const QStringList &molecules);
    bool addAutoFoundMolecules(const QMultiHash<QString, QString> &mol_atc, bool removeFromModel = false);

    // bool moleculeLinker(Internal::MoleculeLinkData *data);

public Q_SLOTS:
    void setActualReviewer(const QString &name);
    int removeUnreviewedMolecules();

private:
    Internal::ComponentAtcModelPrivate *d;
};

}  //  End namespace DrugsDbCreator

#endif // DDIMANAGER_DDIPLUGIN_INTERNAL_COMPONENT_TO_ATC_MODEL_H
