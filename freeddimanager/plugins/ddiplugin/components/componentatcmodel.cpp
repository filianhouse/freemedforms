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
/**
 * \class DDI::ComponentAtcModel
*/

#include "componentatcmodel.h"
#include <ddiplugin/ddicore.h>
#include <ddiplugin/constants.h>
#include <ddiplugin/atc/atctablemodel.h>
#include <ddiplugin/database/ddidatabase.h>

#include <coreplugin/icore.h>
#include <coreplugin/imainwindow.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/isettings.h>

#include <utils/log.h>

#include <QColor>
#include <QIcon>
#include <QSqlTableModel>

using namespace DDI;
using namespace Internal;

static inline Core::IMainWindow *mainwindow() {return Core::ICore::instance()->mainWindow();}
static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }
static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline DDI::DDICore *ddiCore() {return DDI::DDICore::instance();}
static inline DDI::Internal::DDIDatabase &ddiBase()  { return DDI::DDICore::instance()->database(); }

namespace DDI {
namespace Internal {
class ComponentAtcModelPrivate
{
public:
    ComponentAtcModelPrivate(ComponentAtcModel *parent) :
        _sql(0),
        _rows(0),
        q(parent)
    {
    }

    ~ComponentAtcModelPrivate()
    {}

    void createSqlModel()
    {
        _sql = new QSqlTableModel(q, ddiBase().database());
        _sql->setTable(ddiBase().table(Constants::Table_COMPONENTS));
        _sql->setEditStrategy(QSqlTableModel::OnManualSubmit);

        // QObject::connect(_sql, SIGNAL(primeInsert(int,QSqlRecord&)), q, SLOT(populateNewRowWithDefault(int, QSqlRecord&)));
        QObject::connect(_sql, SIGNAL(layoutAboutToBeChanged()), q, SIGNAL(layoutAboutToBeChanged()));
        QObject::connect(_sql, SIGNAL(layoutChanged()), q, SIGNAL(layoutChanged()));
        QObject::connect(_sql, SIGNAL(modelAboutToBeReset()), q, SIGNAL(modelAboutToBeReset()));
        QObject::connect(_sql, SIGNAL(modelReset()), q, SIGNAL(modelReset()));
    }

    int modelColumnToSqlColumn(const int modelColumn)
    {
        int sql = -1;
        switch (modelColumn) {
        case ComponentAtcModel::Id: sql = Constants::COMPO_ID; break;
        case ComponentAtcModel::Uid: sql = Constants::COMPO_UID; break;
        case ComponentAtcModel::DrugDatabaseComponentUid1: sql = Constants::COMPO_DRUGDB_UID1; break;
        case ComponentAtcModel::DrugDatabaseComponentUid2: sql = Constants::COMPO_DRUGDB_UID2; break;
        case ComponentAtcModel::IsValid: sql = Constants::COMPO_ISVALID; break;
        case ComponentAtcModel::IsReviewed: sql = Constants::COMPO_ISREVIEWED; break;
        case ComponentAtcModel::Name: sql = Constants::COMPO_LABEL; break;
        case ComponentAtcModel::AtcCodeList: sql = Constants::COMPO_ATCCODES; break;
        case ComponentAtcModel::SuggestedAtcCodeList: sql = Constants::COMPO_SUGGESTED; break;
        case ComponentAtcModel::Reviewer: sql = Constants::COMPO_REVIEWERS; break;
        case ComponentAtcModel::DateCreation: sql = Constants::COMPO_DATECREATE; break;
        case ComponentAtcModel::DateUpdate: sql = Constants::COMPO_DATEUPDATE; break;
        case ComponentAtcModel::Comments: sql = Constants::COMPO_COMMENT; break;
        };
        return sql;
    }

public:
    QSqlTableModel *_sql;
    QString _reviewer, _actualDbUid;
    int _rows;
    QString _drugsDbFilter;

private:
    ComponentAtcModel *q;
};

}  // namespace Internal
}  // namespace DDI

ComponentAtcModel::ComponentAtcModel(QObject *parent) :
    QAbstractTableModel(parent),
    d(new Internal::ComponentAtcModelPrivate(this))
{
    setObjectName("ComponentAtcModel");
    d->createSqlModel();
}

ComponentAtcModel::~ComponentAtcModel()
{
    if (d)
        delete d;
    d = 0;
}

bool ComponentAtcModel::initialize()
{
    // Fetch all row from the sql model
    d->_sql->select();
    while (d->_sql->canFetchMore(index(d->_sql->rowCount(), 0)))
        d->_sql->fetchMore(index(d->_sql->rowCount(), 0));
    return true;
}

bool ComponentAtcModel::onDdiDatabaseChanged()
{
    delete d->_sql;
    d->_sql = 0;
    d->createSqlModel();
    return initialize();
}

/** Return all available Drug database Uid from the database */
QStringList ComponentAtcModel::availableDrugsDatabases() const
{
    return ddiBase().availableComponentDrugsDatabaseUids();
}

/** Filter data according to the drug database uid \e dbUid1 and \e dbUid2 */
bool ComponentAtcModel::selectDatabase(const QString &dbUid1, const QString &dbUid2)
{
//    if (dbUid == d->_drugsDbFilter)
//        return true;
//    qWarning() << "ComponentAtcModel::selectDatabase" << dbUid;
    beginResetModel();
    QHash<int, QString> where;
    where.insert(Constants::COMPO_DRUGDB_UID1, QString("='%1'").arg(dbUid1));
    if (!dbUid2.isEmpty())
        where.insert(Constants::COMPO_DRUGDB_UID2, QString("='%1'").arg(dbUid2));
    QString filter = ddiBase().getWhereClause(Constants::Table_COMPONENTS, where);
    d->_sql->setFilter(filter);
    initialize();
    endResetModel();
    return true;
}

/** Define the reviewer \e name to use */
void ComponentAtcModel::setActualReviewer(const QString &name)
{
    d->_reviewer = name;
}

int ComponentAtcModel::rowCount(const QModelIndex &parent) const
{
    return d->_sql->rowCount(parent);
}

int ComponentAtcModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

QVariant ComponentAtcModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QModelIndex sqlIndex = d->_sql->index(index.row(), d->modelColumnToSqlColumn(index.column()));
        return d->_sql->data(sqlIndex, role);
    } else if (role == Qt::CheckStateRole) {
        int sql = -1;
        switch (index.column()) {
        case IsValid: sql = Constants::COMPO_ISVALID; break;
        case IsReviewed: sql = Constants::COMPO_ISREVIEWED; break;
        default: sql = -1; break;
        }
        if (sql != -1) {
            QModelIndex sqlIndex = d->_sql->index(index.row(), sql);
            // using displayrole
            return d->_sql->data(sqlIndex).toBool()?Qt::Checked:Qt::Unchecked;
        }
    } else if (role == Qt::ToolTipRole) {
        QStringList atc = d->_sql->index(index.row(), Constants::COMPO_ATCCODES).data().toString().split(";", QString::SkipEmptyParts);
        QStringList atcFull;
        if (!atc.isEmpty()) {
            foreach(const QString &c, atc)
                atcFull << QString("%1 - %2").arg(c).arg(ddiBase().atcLabelForCode(c));
        }
        atc = d->_sql->index(index.row(), Constants::COMPO_SUGGESTED).data().toString().split(";", QString::SkipEmptyParts);
        QStringList suggestedFull;
        if (!atc.isEmpty()) {
            foreach(const QString &c, atc)
                suggestedFull << QString ("%1 - %2").arg(c).arg(ddiBase().atcLabelForCode(c));
        }

        QStringList lines;
        lines << QString("<b>%1</b><br>    %2")
                 .arg(d->_sql->index(index.row(), Constants::COMPO_LABEL).data().toString())
                 .arg(d->_sql->index(index.row(), Constants::COMPO_ISREVIEWED).data().toBool()?"Reviewed":"Unreviewed")
                 .replace(" ", "&nbsp;");
        if (!atcFull.isEmpty()) {
            lines << QString("<u>%1</u>").arg(tr("Linked ATC codes:"));
            lines << QString("&nbsp;&nbsp;%1").arg(atcFull.join("<br>&nbsp;&nbsp;"));
        }
        if (!suggestedFull.isEmpty()) {
            lines << QString("<u>%1</u>").arg(tr("Suggested ATC codes:"));
            lines << QString("&nbsp;&nbsp;%1").arg(suggestedFull.join("<br>&nbsp;&nbsp;"));
        }
        return lines.join("<br>");
    }
    return QVariant();
}

bool ComponentAtcModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    int sql = d->modelColumnToSqlColumn(index.column());
    if (role == Qt::EditRole) {
        bool ok = false;
        QModelIndex sqlIndex = d->_sql->index(index.row(), sql);

        switch (index.column()) {
        case IsValid:
        case IsReviewed:
            ok = d->_sql->setData(sqlIndex, value.toBool()?1:0, role);
            break;
        default: ok = d->_sql->setData(sqlIndex, value, role); break;
        }

        // set the date update
        if (ok) {
            Q_EMIT dataChanged(index, index);
            sqlIndex = d->_sql->index(index.row(), Constants::COMPO_DATEUPDATE);
            ok = d->_sql->setData(sqlIndex, QDateTime::currentDateTime(), role);
            if (ok) {
                QModelIndex idx = this->index(index.row(), DateUpdate);
                Q_EMIT dataChanged(idx, idx);
            }
        }

        return ok;
    }
    return false;
}

Qt::ItemFlags ComponentAtcModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
//    if (index.column() == MoleculeName || index.column() == Date || index.column() == FancyButton)
//        return f;

//    if (index.column() == Review)
//        f |= Qt::ItemIsUserCheckable;

//    f |= Qt::ItemIsEditable;

    return f;
}

//QVariant ComponentAtcModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
//        switch (section) {
//        case Name:
//            return tr("Component name");
//        case AtcCodeList:
//            return tr("ATC code");
//        case IsReviewed:
//            return tr("Review state");
//        case Reviewer:
//            return tr("Reviewer");
//        case Comments:
//            return tr("Comments");
//        case DateCreation:
//            return tr("Date of creation");
//        case DateUpdate:
//            return tr("Date of update");
//        default:
//            return QVariant();
//        }
//    }

//    return QVariant();
//}

bool ComponentAtcModel::addUnreviewedMolecules(const QString &dbUid, const QStringList &molecules)
{
//    QDomElement el = d->m_RootNode.firstChildElement("Database");
//    while (!el.isNull()) {
//        if (el.attribute("uid").compare(dbUid, Qt::CaseInsensitive) == 0) {
//            break;
//        }
//        el = el.nextSiblingElement("Database");
//    }

//    if (el.isNull())
//        return false;

//    selectDatabase(dbUid);

//    QStringList known;
//    for(int i=0; i < rowCount(); ++i) {
//        known << index(i, MoleculeName).data().toString();
//    }
//    known.removeDuplicates();
//    known.removeAll("");

//    foreach(const QString &mol, molecules) {
//        if (mol.isEmpty())
//            continue;
//        if (known.contains(mol, Qt::CaseInsensitive))
//            continue;
//        QDomElement newmol = d->domDocument.createElement("Molecule");
//        newmol.setAttribute("name", mol);
//        newmol.setAttribute("AtcCode", QString());
//        newmol.setAttribute("review", "false");
//        newmol.setAttribute("reviewer", QString());
//        newmol.setAttribute("references", QString());
//        newmol.setAttribute("comment", QString());
//        newmol.setAttribute("dateofreview", QString());
//        el.appendChild(newmol);
//        known << mol;
//    }

//    reset();
    return true;
}

bool ComponentAtcModel::addAutoFoundMolecules(const QMultiHash<QString, QString> &mol_atc, bool removeFromModel)
{
//    int nb = 0;
//    foreach(const QString &mol, mol_atc.keys()) {
//        QDomNode n = d->m_RootItem->node().firstChild();

//         while (!n.isNull()) {
//             QDomNamedNodeMap attributeMap = n.attributes();
//             if (attributeMap.namedItem("name").nodeValue() == mol) {
//                 if (removeFromModel) {
//                     const QStringList &list = mol_atc.values(mol);
//                     n.toElement().setAttribute("autofound", list.join(","));
//                     QDomNode rem = d->m_RootItem->node().removeChild(n);
//                 } else {
//                     const QStringList &list = mol_atc.values(mol);
//                     n.toElement().setAttribute("autofound", list.join(","));
//                 }
//                 ++nb;
//                 break;
//             }
//             n = n.nextSibling();
//         }
//    }
    return true;
}


struct MolLink {
    int lk_nature;
    int mol;
    QString mol_form;
};

//bool ComponentAtcModel::moleculeLinker(Internal::MoleculeLinkData *data)
//{
//    Q_ASSERT(data);
//    if (!data)
//        return false;
//    // get all ATC ids
//    QSqlDatabase iam = data->database->database();
//    if (!iam.open()) {
//        LOG_ERROR("Can not connect to MASTER db");
//        return false;
//    }
//    data->moleculeIdToAtcId.clear();
//    data->unfoundMoleculeAssociations.clear();
//    QHash<QString, int> atc_id;
//    QMultiHash<QString, int> atcName_id;
//    QString req;
//    QSqlQuery query(iam);
//    using namespace DrugsDB::Constants;

//    // Get all ATC Code and Label
//    LOG("Getting ATC Informations from the interactions database");
//    Utils::FieldList get;
//    get << Utils::Field(Table_ATC, ATC_ID);
//    get << Utils::Field(Table_ATC, ATC_CODE);
//    get << Utils::Field(Table_LABELS, LABELS_LABEL);
//    Utils::JoinList joins;
//    joins << Utils::Join(Table_ATC_LABELS, ATC_LABELS_ATCID, Table_ATC, ATC_ID)
//          << Utils::Join(Table_LABELSLINK, LABELSLINK_MASTERLID, Table_ATC_LABELS, ATC_LABELS_MASTERLID)
//          << Utils::Join(Table_LABELS, LABELS_LID, Table_LABELSLINK, LABELSLINK_LID);
//    Utils::FieldList cond;
//    cond << Utils::Field(Table_LABELS, LABELS_LANG, QString("='%1'").arg(data->lang));

//    if (query.exec(data->database->select(get,joins,cond))) {
//        while (query.next()) {
//            atc_id.insert(query.value(1).toString(), query.value(0).toInt());
//            atcName_id.insertMulti(query.value(2).toString().toUpper(), query.value(0).toInt());
//        }
//    } else {
//        LOG_QUERY_ERROR(query);
//    }
//    query.finish();
//    qWarning() << "ATC" << atc_id.count();

//    // Get source ID (SID)
//    int sid = data->sourceId;
//    if (sid==-1) {
//        LOG_ERROR("NO SID: " + data->drugDbUid);
//        return false;
//    }

//    // Get all MOLS.MID and Label
//    LOG("Getting Drugs Composition from " + data->drugDbUid);
//    QMultiHash<QString, int> mols;
//    QHash<int, QString> w;
//    w.insert(MOLS_SID, QString("=%1").arg(sid));
//    req = data->database->selectDistinct(Table_MOLS, QList<int>()
//                                   << MOLS_MID
//                                   << MOLS_NAME, w);
//    if (query.exec(req)) {
//        while (query.next()) {
//            mols.insertMulti(query.value(1).toString(), query.value(0).toInt());
//        }
//    } else {
//        LOG_QUERY_ERROR(query);
//    }
//    query.finish();
//    qWarning() << "Number of distinct molecules" << mols.uniqueKeys().count();
//    const QStringList &knownMoleculeNames = mols.uniqueKeys();

//    // manage corrected molecules
//    foreach(const QString &mol, data->correctedByName.keys()) {
//        if (!knownMoleculeNames.contains(mol))
//            continue;
//        foreach(int id, atcName_id.values(data->correctedByName.value(mol)))
//            data->moleculeIdToAtcId.insertMulti(mols.value(mol), id);
//    }
//    foreach(const QString &mol, data->correctedByAtcCode.uniqueKeys()) {  // Key=mol, Val=ATC
//        if (!knownMoleculeNames.contains(mol))
//            continue;
//        // For all ATC codes corresponding to the molecule name
//        foreach(const QString &atc, data->correctedByAtcCode.values(mol)) {
//            // Get the ATC label and retreive all ATC_ids that have the same label --> NO
//            //                QString atcLabel = atcName_id.key(atc_id.value(atc));
//            //                foreach(int id, atcName_id.values(atcLabel))
//            //                    data->moleculeIdToAtcId.insertMulti(mols.value(mol), id);
//            // Associate molecule to the ATC_Id corresponding to the code
//            foreach(const int molId, mols.values(mol)) {
//                data->moleculeIdToAtcId.insertMulti(molId, atc_id.value(atc));
//            }
//            qWarning() << "Corrected by ATC" << mol << atc << atcName_id.key(atc_id.value(atc));
//        }
//    }
//    LOG("Hand made association: " + QString::number(data->moleculeIdToAtcId.count()));

//    // find links
//    data->unfoundMoleculeAssociations.clear();
//    foreach(const QString &mol, knownMoleculeNames) {
//        if (mol.isEmpty())
//            continue;
//        foreach(const int codeMol, mols.values(mol)) {
//            if (data->moleculeIdToAtcId.keys().contains(codeMol)) {
//                continue;
//            }
//            // Does molecule name exact-matches an ATC label
//            QString molName = mol.toUpper();
//            QList<int> atcIds = atcName_id.values(molName);
//            if (atcIds.count()==0) {
//                // No --> Try to find the ATC label by transforming the molecule name
//                // remove accents
//                molName = molName.replace(QString::fromUtf8("É"), "E");
//                molName = molName.replace(QString::fromUtf8("È"), "E");
//                molName = molName.replace(QString::fromUtf8("À"), "A");
//                molName = molName.replace(QString::fromUtf8("Ï"), "I");
//                atcIds = atcName_id.values(molName);
//                if (atcIds.count()>0) {
//                    qWarning() << "Without accent >>>>>>>>" << molName;
//                }
//                // Not found try some transformations
//                // remove '(*)'
//                if ((atcIds.count()==0) && (molName.contains("("))) {
//                    molName = molName.left(molName.indexOf("(")).simplified();
//                    atcIds = atcName_id.values(molName);
//                    if (atcIds.count()>0) {
//                        qWarning() << "Without (*) >>>>>>>>" << molName;
//                    }
//                }
//                if (atcIds.count()==0) {
//                    // remove last word : CITRATE, DIHYDRATE, SODIUM, HYDROCHLORIDE, POLISTIREX, BROMIDE, MONOHYDRATE, CHLORIDE, CARBAMATE
//                    //  INDANYL SODIUM, ULTRAMICROCRYSTALLINE, TROMETHAMINE
//                    molName = molName.left(molName.lastIndexOf(" ")).simplified();
//                    atcIds = atcName_id.values(molName);
//                    if (atcIds.count()>0) {
//                        qWarning() << "Without last word >>>>>>>>" << molName;
//                    }
//                }
//                if (atcIds.count()==0) {
//                    // remove first words : CHLORHYDRATE DE, ACETATE DE
//                    QStringList toRemove;
//                    toRemove << "CHLORHYDRATE DE" << "CHLORHYDRATE D'" << "ACETATE DE" << "ACETATE D'";
//                    foreach(const QString &prefix, toRemove) {
//                        if (molName.startsWith(prefix)) {
//                            QString tmp = molName;
//                            tmp.remove(prefix);
//                            tmp = tmp.simplified();
//                            atcIds = atcName_id.values(tmp);
//                            if (atcIds.count()) {
//                                molName = tmp;
//                                qWarning() << "Without prefix"<< prefix << ">>>>>>>>" << tmp << atcIds;
//                                break;
//                            }
//                        }
//                    }
//                }
//                if (atcIds.count()==0) {
//                    // Manage (DINITRATE D')
//                    if (molName.contains("(DINITRATE D')")) {
//                        QString tmp = molName;
//                        tmp = tmp.remove("(DINITRATE D')");
//                        tmp += "DINITRATE";
//                        atcIds = atcName_id.values(tmp);
//                        if (atcIds.count()) {
//                            molName = tmp;
//                            qWarning() << "With DINITRATE"<< molName << ">>>>>>>>" << tmp << atcIds;
//                            break;
//                        }
//                    }
//                }
//            }
//            bool found = false;
//            foreach(int id, atcIds) {
//                found = true;
//                data->moleculeIdToAtcId.insertMulti(codeMol, id);
//                qWarning() << "Linked" << mol << atcName_id.key(id);
//            }
//            if (!found) {
//                if (!data->unfoundMoleculeAssociations.contains(mol))
//                    data->unfoundMoleculeAssociations.append(mol);
//            }
//        }
//    }

//    // Inform model of founded links
//    QMultiHash<QString, QString> mol_atc_forModel;
//    QHashIterator<int,int> it(data->moleculeIdToAtcId);
//    while (it.hasNext()) {
//        it.next();
//        mol_atc_forModel.insertMulti(mols.key(it.key()), atc_id.key(it.value()));
//    }
//    addAutoFoundMolecules(mol_atc_forModel);
//    mol_atc_forModel.clear();

//    // Try to find molecules in the ComponentAtcModel
//    QHash<QString, QString> model_mol_atc;
//    int modelFound = 0;
//    int reviewedWithoutAtcLink = 0;
//    selectDatabase(data->drugDbUid);
//    while (canFetchMore(QModelIndex()))
//        fetchMore(QModelIndex());

//    for(int i=0; i< rowCount(); ++i) {
//        if (index(i, ComponentAtcModel::Review).data().toString() == "true") {
//            model_mol_atc.insert(index(i, ComponentAtcModel::MoleculeName).data().toString(),
//                                 index(i, ComponentAtcModel::ATC_Code).data().toString());
//        }
//    }
//    qWarning() << "ComponentAtcModel::AvailableLinks" << model_mol_atc.count();

//    foreach(const QString &mol, data->unfoundMoleculeAssociations) {
//        if (model_mol_atc.keys().contains(mol)) {
//            int codeMol = mols.value(mol);
//            if (!data->moleculeIdToAtcId.keys().contains(codeMol)) {
//                if (model_mol_atc.value(mol).trimmed().isEmpty()) {
//                    ++reviewedWithoutAtcLink;
//                    continue;
//                }
//                QStringList atcCodes = model_mol_atc.value(mol).split(",");
//                foreach(const QString &atcCode, atcCodes) {
//                    QString atcName = atcName_id.key(atc_id.value(atcCode));
//                    QList<int> atcIds = atcName_id.values(atcName);
//                    foreach(int id, atcIds) {
//                        data->moleculeIdToAtcId.insertMulti(codeMol, id);
//                        qWarning() << "ModelLinker Found" << codeMol << mol << id << atcName_id.key(id);
//                        data->unfoundMoleculeAssociations.removeAll(mol);
//                    }
//                    if (atcIds.count())
//                        ++modelFound;
//                }
//            }
//        }
//    }

//    // TODO: Try to find new associations via the COMPOSITION.LK_NATURE field
//    int natureLinkerNb = 0;
////    if (drugsDbUid == "FR_AFSSAPS") {
////        // TODO: code here */
////        QMap<int, QMultiHash<int, int> > cis_codeMol_lk;
////        QMap<int, QVector<MolLink> > cis_compo;
////        {
////            //            QString req = "SELECT `DID`, `MID`, `LK_NATURE` FROM `COMPOSITION` ORDER BY `DID`";
////            QString req = database->select(Table_COMPO, QList<int>()
////                                                             << COMPO_DID
////                                                             << COMPO_MID
////                                                             << COMPO_LK_NATURE
////                                                             );
////            req += QString(" ORDER BY `%1`").arg(database->fieldName(Table_COMPO, COMPO_DID));
////            if (query.exec(req)) {
////                while (query.next()) {
////                    QVector<MolLink> &receiver = cis_compo[query.value(0).toInt()];
////                    MolLink lk;
////                    lk.mol = query.value(1).toInt();
////                    lk.lk_nature = query.value(2).toInt();
////                    //                    lk.mol_form = query.value(3).toString();
////                    receiver.append(lk);
////                }
////            } else {
////                LOG_QUERY_ERROR(query);
////            }
////            QMutableMapIterator<int, QVector<MolLink> > i(cis_compo);
////            while (i.hasNext()) {
////                i.next();
////                const QVector<MolLink> &links = i.value();
////                QMultiHash<int, int> lk_mol;
////                QMultiHash<int, QString> lk_form;
////                foreach(const MolLink &lk, links) {
////                    lk_mol.insertMulti(lk.lk_nature, lk.mol);
////                    lk_form.insertMulti(lk.lk_nature, lk.mol_form);
////                }
////                foreach(int key, lk_mol.uniqueKeys()) {
////                    QStringList forms = lk_form.values(key);
////                    const QList<int> &mols = lk_mol.values(key);
////                    forms.removeDuplicates();
////                    if (forms.count()==1 && mols.count()==2) {
////                        // link molecules
////                        QMultiHash<int, int> &code_lk = cis_codeMol_lk[i.key()];
////                        code_lk.insertMulti(key, mols.at(0));
////                        code_lk.insertMulti(key, mols.at(1));
////                    }
////                }
////            }
////        }
////        // Computation
////        int nb;
////        do
////        {
////            nb=0;
////            QMutableMapIterator<int, QMultiHash<int, int> > i (cis_codeMol_lk);
////            while (i.hasNext()) {
////                i.next();
////                const QMultiHash<int, int> lk_codemol = i.value();
////                // for all molecule_codes
////                foreach(int lk, lk_codemol.uniqueKeys()) {
////                    if (lk_codemol.count(lk) == 2) {
////                        // Ok for computation
////                        int one, two;
////                        one = lk_codemol.values(lk).at(0);
////                        two = lk_codemol.values(lk).at(1);

////                        // if both molecule_codes are known or unknown --> continue
////                        if ((!data->moleculeIdToAtcId.keys().contains(one)) &&
////                                (!data->moleculeIdToAtcId.keys().contains(two)))
////                            continue;
////                        if ((data->moleculeIdToAtcId.keys().contains(one)) &&
////                                (data->moleculeIdToAtcId.keys().contains(two)))
////                            continue;

////                        // Associate unknown molecule_code with the known ATC
////                        if (data->moleculeIdToAtcId.keys().contains(one)) {
////                            // The second molecule is unknown
////                            const QList<int> &atcIds = data->moleculeIdToAtcId.values(one);
////                            foreach(int actId, atcIds) {
////                                data->moleculeIdToAtcId.insertMulti(two, actId);
////                                qWarning() << "LK_NATURE: Linked" << i.key() << mols.key(one) << mols.key(two) << lk << atcName_id.key(actId);
////                                data->unfoundMoleculeAssociations.removeAll(mols.key(two));
////                                data->moleculeIdToAtcId_forModel.insertMulti(mols.key(one), atcName_id.key(actId));
////                            }
////                        } else if (data->moleculeIdToAtcId.keys().contains(two)) {
////                            // The first is unknown
////                            const QList<int> &atcIds = data->moleculeIdToAtcId.values(two);
////                            foreach(int actId, atcIds) {
////                                data->moleculeIdToAtcId.insertMulti(one, actId);
////                                qWarning() << "LK_NATURE: Linked" << i.key() << mols.key(one) << mols.key(two) << lk << atcName_id.key(actId);
////                                data->unfoundMoleculeAssociations.removeAll(mols.key(one));
////                                data->moleculeIdToAtcId_forModel.insertMulti(mols.key(one), atcName_id.key(actId));
////                            }
////                        }
////                        ++nb;
////                        ++natureLinkerNb;
////                    }
////                }
////            }
////            LOG(QString("Link composition by nature: %1 total associations founded.").arg(nb));
////        }
////        while (nb > 0);
////    }

//    // Save completion percent in drugs database INFORMATION table
//    data->completionPercentage = ((double) (1.0 - ((double)(data->unfoundMoleculeAssociations.count() - reviewedWithoutAtcLink) / (double)knownMoleculeNames.count())) * 100.00);
//    LOG(QString("Molecule links completion: %1").arg(data->completionPercentage));
//    //    DrugsDB::Tools::executeSqlQuery(QString("UPDATE SOURCES SET MOL_LINK_COMPLETION=%1 WHERE SID=%2")
//    //                                 .arg(completion).arg(sid),
//    //                                 Core::Constants::MASTER_DATABASE_NAME, __FILE__, __LINE__);
//    QHash<int, QString> where;
//    where.insert(SOURCES_SID, QString("='%1'").arg(sid));
//    query.prepare(data->database->prepareUpdateQuery(Table_SOURCES, SOURCES_COMPLETION, where));
//    query.bindValue(0, data->completionPercentage);
//    if (!query.exec()) {
//        LOG_QUERY_ERROR_FOR("Tools", query);
//        return false;
//    }
//    query.finish();

//    // Inform model of founded links
//    addAutoFoundMolecules(mol_atc_forModel, true);
//    mol_atc_forModel.clear();

//    qWarning()
//            << "\nNUMBER OF MOLECULES" << knownMoleculeNames.count()
//            << "\nCORRECTED BY NAME" << data->correctedByName.keys().count()
//            << "\nCORRECTED BY ATC" << data->correctedByAtcCode.uniqueKeys().count()
//            << "\nFOUNDED" << data->moleculeIdToAtcId.uniqueKeys().count()
//            << QString("\nLINKERMODEL (WithATC:%1;WithoutATC:%2) %3").arg(modelFound).arg(reviewedWithoutAtcLink).arg(modelFound + reviewedWithoutAtcLink)
//            << "\nLINKERNATURE" << natureLinkerNb
//            << "\nLEFT" << (data->unfoundMoleculeAssociations.count() - reviewedWithoutAtcLink)
//            << "\nCONFIDENCE INDICE" << data->completionPercentage
//            << "\n\n";

//    return true;
//}

int ComponentAtcModel::removeUnreviewedMolecules()
{
//    int nb = 0;
//    int examined = 0;
//    QDomElement n = d->m_RootItem->node().firstChildElement();

//     while (!n.isNull()) {
//         if (n.attribute("review").compare("true", Qt::CaseInsensitive) != 0) {
//             QDomElement save = n.nextSiblingElement();
//             QDomNode rem = d->m_RootItem->node().removeChild(n);
//             if (!rem.isNull())
//                 ++nb;
//             n = save;
//             ++examined;
//             continue;
//         }
//         ++examined;
//         n = n.nextSiblingElement();
//     }
//     reset();
//     return nb;
    return 0;
}

